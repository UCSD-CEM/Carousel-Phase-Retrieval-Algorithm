#include "../include/CPRA.hpp"
#include "../include/Command_Parser.hpp"
#include <iostream>
#include <sstream>
#include <chrono> 
#include <vector>
#include <string>
#include <utility>

template<typename T>
std::pair<std::complex<T>*, T*> ShrinkWrap_CPRA_MKL_Sample(int M, int N, int L, int P, int BATCHSIZE_CPRA, 
                                                                T* dataConstr, T* spaceConstr, 
                                                                int epi, int iter, int pre_iter = 1000, T Beta = 0.9)
{
    // 2D part test
    // Shrinkwrap
    CPRA::Cpra<T, CPRA::IMPL_TYPE::MKL> compute(M, N, L, BATCHSIZE_CPRA);
    std::complex<T>* random_guess = (std::complex<T>*)compute.allocate(sizeof(std::complex<T>) * M * N * P * BATCHSIZE_CPRA);
    std::complex<T>* t_random_guess_1 = (std::complex<T>*)compute.allocate(sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    std::complex<T>* t_random_guess_2 = (std::complex<T>*)compute.allocate(sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    compute.impl_->Initialize(reinterpret_cast<T*>(random_guess), M * N * P * BATCHSIZE_CPRA * 2);
    compute.impl_->Initialize(reinterpret_cast<T*>(t_random_guess_1), M * N * BATCHSIZE_CPRA * 2);
    compute.impl_->Initialize(reinterpret_cast<T*>(t_random_guess_2), M * N * BATCHSIZE_CPRA * 2);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    // For error control
    std::complex<T>* old_record = (std::complex<T>*)compute.allocate(sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    T* error = (T*)compute.allocate(sizeof(T) * P * BATCHSIZE_CPRA);

    // Step A, pre-reconstruct
    // Num of iteration here is fixed at 1000
    // Shrinkwrap algo
    compute.impl_->Memcpy((void*)(t_random_guess_1), (void*)(random_guess), sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    compute.impl_->Memcpy((void*)(t_random_guess_2), (void*)(random_guess), sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    for(auto i = 0; i < pre_iter; i++)
    {
        // Part 1
        compute.impl_->Forward2D(t_random_guess_1);
        compute.impl_->DataConstraint(t_random_guess_1, dataConstr, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        compute.impl_->Backward2D(t_random_guess_1);
        compute.impl_->MergeAddData(random_guess, t_random_guess_1, -1.0 / Beta, 1.0 + 1.0 / Beta, M * N * BATCHSIZE_CPRA);
        compute.impl_->SpaceConstraint(t_random_guess_1, spaceConstr, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        
        // Part 2
        compute.impl_->SpaceConstraint(t_random_guess_2, spaceConstr, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        compute.impl_->MergeAddData(random_guess, t_random_guess_2, 1.0 / Beta, 1.0 - 1.0 / Beta, M * N * BATCHSIZE_CPRA);
        compute.impl_->Forward2D(t_random_guess_2);
        compute.impl_->DataConstraint(t_random_guess_2, dataConstr, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        compute.impl_->Backward2D(t_random_guess_2);
        // Merge 
        compute.impl_->MergeAddData(t_random_guess_1, random_guess, Beta, 1.0, M * N * BATCHSIZE_CPRA);
        compute.impl_->MergeAddData(t_random_guess_2, random_guess, -1.0 * Beta, 1.0, M * N * BATCHSIZE_CPRA);
    }
    compute.impl_->Sync();

    // Step B, reconstruct 2D projected object
    for(auto e = 0; e < epi; e++)  // episode
    {
        for(auto p = 0; p < P; p++) // each projected object
        {
           // Merge data
           if(p == P - 1)
           {
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * (p - 1), random_guess + M * N * BATCHSIZE_CPRA * p, 0.5, 0.5, M * N * BATCHSIZE_CPRA);  
           }
           else if(p == 0)
           {
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * (p + 1), random_guess + M * N * BATCHSIZE_CPRA * p, 0.5, 0.5, M * N * BATCHSIZE_CPRA);
           }
           else
           {
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * (p - 1), random_guess + M * N * BATCHSIZE_CPRA * p, 0.5, 0, M * N * BATCHSIZE_CPRA); 
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * (p + 1), random_guess + M * N * BATCHSIZE_CPRA * p, 0.5, 1, M * N * BATCHSIZE_CPRA);
           }

            // Copy to temporary variable
            compute.impl_->Memcpy((void*)(t_random_guess_1), (void*)(random_guess + M * N * BATCHSIZE_CPRA * p), sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
            compute.impl_->Memcpy((void*)(t_random_guess_2), (void*)(random_guess + M * N * BATCHSIZE_CPRA * p), sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
            // Reconstruct
            // Shrinkwrap algo
            for(auto i = 0; i < iter; i++) // each iteration
            {
                // error part
                if(i == iter - 1 && e == epi - 1)
                {
                    compute.impl_->Memcpy((void*)old_record,
                                          (void*)(random_guess + M * N * BATCHSIZE_CPRA * p),
                                          sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
                }
                    
                
                // Part 1
                compute.impl_->Forward2D(t_random_guess_1);
                compute.impl_->DataConstraint(t_random_guess_1, dataConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
                compute.impl_->Backward2D(t_random_guess_1);
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * p, t_random_guess_1, -1.0 / Beta, 1.0 + 1.0 / Beta, M * N * BATCHSIZE_CPRA);
                compute.impl_->SpaceConstraint(t_random_guess_1, spaceConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
                
                // Part 2
                compute.impl_->SpaceConstraint(t_random_guess_2, spaceConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
                compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * p, t_random_guess_2, 1.0 / Beta, 1.0 - 1.0 / Beta, M * N * BATCHSIZE_CPRA);
                compute.impl_->Forward2D(t_random_guess_2);
                compute.impl_->DataConstraint(t_random_guess_2, dataConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
                compute.impl_->Backward2D(t_random_guess_2);

                // Merge 
                compute.impl_->MergeAddData(t_random_guess_1, random_guess + M * N * BATCHSIZE_CPRA * p, Beta, 1.0, M * N * BATCHSIZE_CPRA);
                compute.impl_->MergeAddData(t_random_guess_2, random_guess + M * N * BATCHSIZE_CPRA * p, -1.0 * Beta, 1.0, M * N * BATCHSIZE_CPRA);

                // error part
                if(i == iter - 1 && e == epi - 1)
                {
                    compute.impl_->Memcpy((void*)t_random_guess_1,
                                          (void*)(random_guess + M * N * BATCHSIZE_CPRA * p),
                                          sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
                    compute.impl_->Forward2D(old_record);
                    compute.impl_->Forward2D(t_random_guess_1);
                    compute.impl_->ConvergeError(old_record, t_random_guess_1, error + BATCHSIZE_CPRA * p, M, N, 1, BATCHSIZE_CPRA);
                }
            }
        }
    }

    for(auto p = 0; p < P; p++)
    {
        // Finishing reconstruction
        compute.impl_->Memcpy((void*)t_random_guess_1,
                                (void*)(random_guess + M * N * BATCHSIZE_CPRA * p),
                                sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
        compute.impl_->Forward2D(t_random_guess_1);
        compute.impl_->DataConstraint(t_random_guess_1, dataConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        compute.impl_->Backward2D(t_random_guess_1);
        compute.impl_->MergeAddData(random_guess + M * N * BATCHSIZE_CPRA * p, t_random_guess_1, -1.0 / Beta, 1.0 + 1.0 / Beta, M * N * BATCHSIZE_CPRA);
        compute.impl_->SpaceConstraint(t_random_guess_1, spaceConstr + M * N * p, M * N * BATCHSIZE_CPRA, BATCHSIZE_CPRA);
        compute.impl_->Memcpy((void*)(random_guess + M * N * BATCHSIZE_CPRA * p),
                                (void*)t_random_guess_1,
                                sizeof(std::complex<T>) * M * N * BATCHSIZE_CPRA);
    }


    compute.impl_->Sync();
    end = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";

    compute.deallocate(t_random_guess_1);
    compute.deallocate(t_random_guess_2);
    compute.deallocate(old_record);
    return std::make_pair(random_guess, error);
}

int main(int argc, const char* argv[])
{
    CPRA::Parser parser;
    parser.parse(argc, argv);
    // an obj for data preparation
    CPRA::Cpra<float, CPRA::IMPL_TYPE::MKL> obj(1, 1, 1, 1);
    float* dataConstr = (float*)obj.allocate(sizeof(float) * parser.getM() * parser.getN() * parser.getP());
    if(!obj.ReadMatrixFromFile(parser.getDataConstr(), dataConstr, parser.getM(), parser.getN(), parser.getP()))
    {
        throw std::runtime_error("Read file " + parser.getDataConstr() + " failed!");
    }
    
    float* spaceConstr = (float*)obj.allocate(sizeof(float) * parser.getM() * parser.getN() * parser.getP());
    if(!obj.ReadMatrixFromFile(parser.getSpaceConstr(), spaceConstr, parser.getM(), parser.getN(), parser.getP()))
    {
        throw std::runtime_error("Read file " + parser.getSpaceConstr() + " failed!");
    }
    uint64_t N = std::max(parser.getBatch(), parser.getTotal());
    int cnt = N;
    int fileCnt = 1; // count file name index
    float* per_real_rec_projected_object = (float*)obj.allocate(sizeof(float) * parser.getM() * parser.getN() * parser.getP());
    float* per_error = (float*)obj.allocate(sizeof(float) * parser.getP());
    while(true)
    {
        int n = std::min(cnt, parser.getBatch());
        std::cout << "Reconstrucint batch size " << n << ". Total batch left " << cnt << "." << std::endl;
        auto results = ShrinkWrap_CPRA_MKL_Sample<float>(parser.getM(),
                                                        parser.getN(),
                                                        parser.getL(),
                                                        parser.getP(),
                                                        n,
                                                        dataConstr,
                                                        spaceConstr,
                                                        parser.getEpi(),
                                                        parser.getIter(),
                                                        parser.getPreIter(),
                                                        parser.getBeta()
                                                        );
        auto rec_projected_object = results.first;
        auto error = results.second;
        uint64_t num = parser.getM() * parser.getN() * parser.getP() * n;
        float* real_rec_projected_object = (float*)obj.allocate(sizeof(float) * num);
        obj.ComplexToReal(rec_projected_object, real_rec_projected_object, num);
        uint64_t start = N - cnt;
        // Write data
        for(uint64_t i = 0; i < n; i++)
        {
            for(uint64_t p = 0; p < parser.getP(); p++)
            {
                per_error[p] = error[p * n + i];
                obj.impl_->Memcpy(per_real_rec_projected_object + parser.getM() * parser.getN() * p,
                                  real_rec_projected_object + (p * n + i) * parser.getM() * parser.getN(),
                                  sizeof(float) * parser.getM() * parser.getN());  
            }
            obj.WriteMatrixToFile(parser.getOutputReconstr()+"."+std::to_string(fileCnt),
                                  per_real_rec_projected_object,
                                  parser.getM() * parser.getN() * parser.getP(), 1, 1);
            obj.WriteMatrixToFile(parser.getOutputError()+"."+std::to_string(fileCnt),
                                  per_error,
                                  sizeof(float) * parser.getP(), 1, 1);
            fileCnt++;
        }

        obj.deallocate(rec_projected_object);
        obj.deallocate(real_rec_projected_object);
        obj.deallocate(error);

        cnt -= n;
        if(cnt <= 0) break;  
    }
    // free memory
    obj.deallocate(per_real_rec_projected_object);
    obj.deallocate(per_error);
    obj.deallocate(dataConstr);
    obj.deallocate(spaceConstr);
    return EXIT_SUCCESS;
}