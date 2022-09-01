#pragma once
#include <curand.h>
#include <time.h>
#include <typeinfo>
#include "./detail/CUDA_Impl_Kernel.cuh"

namespace CPRA{

template<typename T>
bool CudaImpl<T>::Initialize(T* flat_data_ptr, size_t num)
{
    // Check params
    if(num < 1) return false;

    // Random Uniform
    curandGenerator_t handle;
    CPRA_CURAND_CALL(curandCreateGenerator(&handle, CURAND_RNG_PSEUDO_MT19937));
    CPRA_CURAND_CALL(curandSetPseudoRandomGeneratorSeed(handle, time(NULL)));
    if constexpr(std::is_same_v<T, float>)
        CPRA_CURAND_CALL(curandGenerateUniform(handle, flat_data_ptr, num));
    else
        CPRA_CURAND_CALL(curandGenerateUniformDouble (handle, flat_data_ptr, num));
    CPRA_CURAND_CALL(curandDestroyGenerator(handle));
    return true;
}

template<typename T>
bool CudaImpl<T>::Forward2D(std::complex<T>* flat_input)
{
    if constexpr (std::is_same_v<T, float>)
    {
        cufftExecC2C(Dfti2DHandle_, (cuComplex*)flat_input, (cuComplex*)flat_input, CUFFT_FORWARD);
    }
    else
    {
        cufftExecZ2Z(Dfti2DHandle_, (cuDoubleComplex*)flat_input, (cuDoubleComplex*)flat_input, CUFFT_FORWARD);
    }
    return true;
}

template<typename T>
bool CudaImpl<T>::Backward2D(std::complex<T>* flat_input)
{
    if constexpr (std::is_same_v<T, float>)
    {
        cufftExecC2C(Dfti2DHandle_, (cuComplex*)flat_input, (cuComplex*)flat_input, CUFFT_INVERSE);
    }
    else
    {
        cufftExecZ2Z(Dfti2DHandle_, (cuDoubleComplex*)flat_input, (cuDoubleComplex*)flat_input, CUFFT_INVERSE);
    }
    return true;
}

template<typename T>
bool CudaImpl<T>::Forward3D(std::complex<T>* flat_input)
{
    if constexpr (std::is_same_v<T, float>)
    {
        cufftExecC2C(Dfti3DHandle_, (cuComplex*)flat_input, (cuComplex*)flat_input, CUFFT_FORWARD);
    }
    else
    {
        cufftExecZ2Z(Dfti3DHandle_, (cuDoubleComplex*)flat_input, (cuDoubleComplex*)flat_input, CUFFT_FORWARD);
    }
    return true;
}

template<typename T>
bool CudaImpl<T>::Backward3D(std::complex<T>* flat_input)
{
    if constexpr (std::is_same_v<T, float>)
    {
        cufftExecC2C(Dfti3DHandle_, (cuComplex*)flat_input, (cuComplex*)flat_input, CUFFT_INVERSE);
    }
    else
    {
        cufftExecZ2Z(Dfti3DHandle_, (cuDoubleComplex*)flat_input, (cuDoubleComplex*)flat_input, CUFFT_INVERSE);
    }
    return true;
}

template<typename T>
bool CudaImpl<T>::SpaceConstraint(std::complex<T>* flat_src_data, T* flat_constr_data, size_t num, size_t batch_size)
{
    int block_size = 256;
    int per_thread_data = 8;
    int per_block_data = block_size * per_thread_data;
    int grid_size = (num + per_block_data - 1) / per_block_data;
    Kernel::ker_SpaceConstraint<T><<<grid_size, block_size, 0, stream_>>>
    ((thrust::complex<T>*)flat_src_data, flat_constr_data, num, batch_size);
}



}