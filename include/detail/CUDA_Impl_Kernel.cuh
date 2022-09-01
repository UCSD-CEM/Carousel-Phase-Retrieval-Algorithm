#pragma once
#include <cuda_runtime.h>
#include <cuda.h>
#include <complex>
namespace CPRA
{
namespace Kernel
{
// Kernels can be optimized
// These are demos
template<typename T>
__global__ void ker_SpaceConstraint(thrust::complex<T>* flat_src_data, T* flat_constr_data, size_t num, size_t batch_size)
{
    int thread_id =  blockIdx.x * blockDim.x + threadIdx.x;
    for(size_t i = thread_id; i < num; i+= gridDim.x * blockDim.x)
    {
        flat_src_data[i].imag(0);
        if(flat_constr_data[i / batch_size] == 0)
            flat_src_data[i].real(0);
    }
}

template<typename T>
__global__ void ker_RealDataConstraint(thrust::complex<T>* flat_src_data, T* flat_constr_data, size_t num, size_t batch_size)
{
    int thread_id =  blockIdx.x * blockDim.x + threadIdx.x;
    for(size_t i = thread_id; i < num; i+= gridDim.x * blockDim.x)
    {
        flat_src_data[i] = thrust::polar<T, T>(flat_constr_data[i / batch_size], thrust::arg(flat_src_data[i]));
    }
}

template<typename T>
__global__ void ker_ComplexDataConstraint(thrust::complex<T>* flat_src_data, thrust::complex<T>* flat_constr_data, size_t num, size_t batch_size)
{
    int thread_id =  blockIdx.x * blockDim.x + threadIdx.x;
    for(size_t i = thread_id; i < num; i+= gridDim.x * blockDim.x)
    {
        flat_src_data[i] = (thrust::norm(flat_constr_data[i / batch_size]) == 0) ? flat_src_data[i] : flat_constr_data[i / batch_size];
    }
}

template<typename T>
__global__ void ker_MergeAddData(thrust::complex<T>* flat_src, thrust::complex<T>* flat_dst, T alpha, T beta, size_t num)
{
    int thread_id =  blockIdx.x * blockDim.x + threadIdx.x;
    for(size_t i = thread_id; i < num; i+= gridDim.x * blockDim.x)
    {
        flat_dst[i] = flat_src[i] * alpha + flat_dst[i] * beta;
    }
}

template<typename T>
__global__ void ker_Normalization(thrust::complex<T>* flat_src, T norm, size_t num)
{
    int thread_id =  blockIdx.x * blockDim.x + threadIdx.x;
    for(size_t i = thread_id; i < num; i+= gridDim.x * blockDim.x)
    {
        flat_src[i] /= norm;
    }
}


}
}