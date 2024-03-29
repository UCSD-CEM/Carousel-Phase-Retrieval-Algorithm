#include <gtest/gtest.h>
#include "../include/CPRA.hpp"

TEST(CUDATEST, Test_CUDA_Initialize)
{
    CPRA::Cpra<float, CPRA::IMPL_TYPE::CUDA> compute(1, 1, 1, 1);
    auto* test_ptr = (float*)compute.allocate(sizeof(float) * 1000);
    compute.impl_->Initialize(test_ptr, 1000);
    for(int i = 0; i < 1000; i++)
        EXPECT_TRUE((test_ptr[i] >= 0) && (test_ptr[i] <= 1));
    compute.deallocate(test_ptr);
}

TEST(CUDATEST, Test_Merge_Norm)
{
    CPRA::Cpra<float, CPRA::IMPL_TYPE::CUDA> obj(1, 1, 1, 1);
    std::complex<float>* input = (std::complex<float>*)obj.allocate(sizeof(std::complex<float>) * 10);
    for(auto i = 0; i < 10; i++)
    {
        input[i].real(10);
        input[i].imag(10);
    }
    obj.impl_->MergeAddData(input, input, 1, 1, 10);

    cudaDeviceSynchronize(); // We need to sync here to make sure kernel is finished
    for(auto i = 0; i < 10; i++)
    {
        EXPECT_EQ(input[i].real(), 20); 
        EXPECT_EQ(input[i].imag(), 20);   
    }
    
    obj.impl_->Normalization(input, 20, 10);
    cudaDeviceSynchronize(); // We need to sync here to make sure kernel is finished
    for(auto i = 0; i < 10; i++)
    {
        EXPECT_EQ(input[i].real(), 1); 
        EXPECT_EQ(input[i].imag(), 1);   
    }


    obj.deallocate(input);
}