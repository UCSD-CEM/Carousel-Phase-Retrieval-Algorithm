#pragma once
#ifdef HAS_MKL
#include "CPRA_Impl.hpp"
#include "mkl_dfti.h"
#include "mkl_service.h"
#include <cstring>
namespace CPRA
{
template <typename T>    
class MklImpl final : public CpraImpl<T>
{
    public:
        MklImpl() = default;
        MklImpl(unsigned long long m, 
                unsigned long long n, 
                unsigned long long l, 
                unsigned long long batch_size) : m_(m), n_(n), l_(l), batch_size_(batch_size)
        {
            MKL_LONG dim2D[2] = {m_, n_};
            MKL_LONG dim3D[3] = {m_, n_, l_};
            MKL_LONG batch = batch_size_; // TODO: maybe we should seperate batch_size 2D and 3D
            if constexpr(std::is_same_v<T, float>)
            {	
                status = DftiCreateDescriptor(&Dfti2DHandle_, DFTI_SINGLE,
                    DFTI_COMPLEX, 2, dim2D);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti2DHandle_, DFTI_NUMBER_OF_TRANSFORMS, batch_size_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti2DHandle_, DFTI_INPUT_DISTANCE, m_ * n_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti2DHandle_, DFTI_BACKWARD_SCALE, (float)1.0 / (m_ * n_));
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }

                status = DftiCreateDescriptor(&Dfti3DHandle_, DFTI_SINGLE,
                    DFTI_COMPLEX, 3, dim2D);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_NUMBER_OF_TRANSFORMS, batch_size_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_INPUT_DISTANCE, m_ * n_ * l_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_BACKWARD_SCALE, (float)1.0 / (m_ * n_ * l_));
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
            }
            else
            {
                status = DftiCreateDescriptor(&Dfti2DHandle_, DFTI_DOUBLE,
                    DFTI_COMPLEX, 2, dim2D);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti2DHandle_, DFTI_NUMBER_OF_TRANSFORMS, batch_size_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti2DHandle_, DFTI_INPUT_DISTANCE, m_ * n_);
                status = DftiSetValue(Dfti2DHandle_, DFTI_BACKWARD_SCALE, (double)1.0 / (m_ * n_));
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }

                status = DftiCreateDescriptor(&Dfti3DHandle_, DFTI_DOUBLE,
                    DFTI_COMPLEX, 3, dim2D);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_NUMBER_OF_TRANSFORMS, batch_size_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_INPUT_DISTANCE, m_ * n_ * l_);
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
                status = DftiSetValue(Dfti3DHandle_, DFTI_BACKWARD_SCALE, (double)1.0 / (m_ * n_ * l_));
                if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
                {
                printf("Error: %s\n", DftiErrorMessage(status));
                }
            }

            status = DftiCommitDescriptor(Dfti2DHandle_);
            if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
            {
            printf("Error: %s\n", DftiErrorMessage(status));
            }
            status = DftiCommitDescriptor(Dfti3DHandle_);
            if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
            {
            printf("Error: %s\n", DftiErrorMessage(status));
            }
        }
        
        // Initialize
        bool Initialize(T* flat_data_ptr, unsigned long long num) override;

        bool Forward2D(std::complex<T>* flat_input) override;

        bool Backward2D(std::complex<T>* flat_input) override;

        bool Forward3D(std::complex<T>* flat_input) override;

        bool Backward3D(std::complex<T>* flat_input) override;

        bool SpaceConstraint(std::complex<T>* flat_src_data, T* flat_constr_data, unsigned long long num, unsigned long long batch_size) override;

        bool RealDataConstraint(std::complex<T>* flat_src_data, T* flat_constr_data, unsigned long long num, unsigned long long batch_size) override;

        bool ComplexDataConstraint(std::complex<T>* flat_src_data, std::complex<T>* flat_constr_data, unsigned long long num, unsigned long long batch_size) override;

        // Add src to dst
        // flat_dst = alpha * flat_src + flat_dst 
        bool MergeAddData(std::complex<T>* flat_src, std::complex<T>* flat_dst, T alpha, T beta, unsigned long long num) override;

        // flat_src = flat_src ./ norm
        bool Normalization(std::complex<T>* flat_src, T norm, unsigned long long num) override;

        bool Complex2DTo3DInterpolation(std::complex<T>* flat_2d_src, std::complex<T>* flat_3D_dst, T* angles, unsigned long long m, unsigned long long n, unsigned long long p, unsigned long long l) override {};

        bool Memcpy(void* dst, void* src, unsigned long long bytes) override
        {
            std::memcpy(dst, src, bytes);
        }
        bool Sync() override
        {
            return true;
        }
        ~MklImpl()
        {
            DftiFreeDescriptor(&Dfti2DHandle_);
            if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
            {
            printf("Error: %s\n", DftiErrorMessage(status));
            }
            DftiFreeDescriptor(&Dfti3DHandle_);
            if (status && !DftiErrorClass(status, DFTI_NO_ERROR))
            {
            printf("Error: %s\n", DftiErrorMessage(status));
            }
        }

    private:
        unsigned long long m_;
        unsigned long long n_;
        unsigned long long l_;
        unsigned long long batch_size_;
        bool inplace_;
        DFTI_DESCRIPTOR_HANDLE Dfti2DHandle_;
        DFTI_DESCRIPTOR_HANDLE Dfti3DHandle_;
        MKL_LONG status; //TODO: Add error check

        
};  // MklCpra

template<typename T>
std::unique_ptr<CpraImpl<T>> NewMKLImpl(unsigned long long m, unsigned long long n, unsigned long long l, unsigned long long batch_size)
{
    return std::make_unique<MklImpl<T>>(m, n, l, batch_size);
}

}// namespace CPRA

#include "./detail/MKL_Impl.inl"

#endif