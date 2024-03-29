/*
 * Copyright (c) 2020-2022, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cuda_runtime_api.h>
#include <stdexcept>
#include <string>
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <cublas_v2.h>
namespace CPRA {
/**
 * @brief Exception thrown when a CUDA error is encountered.
 *
 */
struct cuda_error : public std::runtime_error {
  /**
   * @brief Constructs a `cuda_error` object with the given `message`.
   *
   * @param message The error char array used to construct `cuda_error`
   */
  cuda_error(const char* message) : std::runtime_error(message) {}
  /**
   * @brief Constructs a `cuda_error` object with the given `message` string.
   *
   * @param message The `std::string` used to construct `cuda_error`
   */
  cuda_error(std::string const& message) : cuda_error{message.c_str()} {}
};
}  // namespace CPRA

#define STRINGIFY_DETAIL(x) #x
#define CPRA_STRINGIFY(x)   STRINGIFY_DETAIL(x)

/**
 * @brief Error checking macro for CUDA runtime API functions.
 *
 * Invokes a CUDA runtime API function call. If the call does not return
 * `cudaSuccess`, invokes cudaGetLastError() to clear the error and throws an
 * exception detailing the CUDA error that occurred
 *
 * Defaults to throwing `CPRA::cuda_error`, but a custom exception may also be
 * specified.
 *
 * Example:
 * ```c++
 *
 * // Throws `rmm::cuda_error` if `cudaMalloc` fails
 * CPRA_CUDA_TRY(cudaMalloc(&p, 100));
 *
 * // Throws `std::runtime_error` if `cudaMalloc` fails
 * CPRA_CUDA_TRY(cudaMalloc(&p, 100), std::runtime_error);
 * ```
 *
 */
#define CPRA_CUDA_TRY(...)                                               \
  GET_CPRA_CUDA_TRY_MACRO(__VA_ARGS__, CPRA_CUDA_TRY_2, CPRA_CUDA_TRY_1) \
  (__VA_ARGS__)
#define GET_CPRA_CUDA_TRY_MACRO(_1, _2, NAME, ...) NAME
#define CPRA_CUDA_TRY_2(_call, _exception_type)                                                    \
  do {                                                                                             \
    cudaError_t const error = (_call);                                                             \
    if (cudaSuccess != error) {                                                                    \
      cudaGetLastError();                                                                          \
      throw _exception_type{std::string{"CUDA error at: "} + __FILE__ + CPRA_STRINGIFY(__LINE__) + \
                            ": " + cudaGetErrorName(error) + " " + cudaGetErrorString(error)};     \
    }                                                                                              \
  } while (0);
#define CPRA_CUDA_TRY_1(_call) CPRA_CUDA_TRY_2(_call, CPRA::cuda_error)

/**
 * @brief Error checking macro for CUDA runtime API that asserts the result is
 * equal to `cudaSuccess`.
 *
 */
#define CPRA_ASSERT_CUDA_SUCCESS(expr) \
  do {                                 \
    cudaError_t const status = (expr); \
    assert(cudaSuccess == status);     \
  } while (0)

/**
 * @brief Macro for checking runtime conditions that throws an exception when
 * a condition is violated.
 *
 * Example usage:
 *
 * @code
 * CPRA_RUNTIME_EXPECTS(key == value, "Key value mismatch");
 * @endcode
 *
 * @param[in] cond Expression that evaluates to true or false
 * @param[in] reason String literal description of the reason that cond is
 * expected to be true
 * @throw std::runtime_error if the condition evaluates to false.
 */
#define CPRA_RUNTIME_EXPECTS(cond, reason)                           \
  (!!(cond)) ? static_cast<void>(0)                                  \
             : throw std::runtime_error("CPRA failure at: " __FILE__ \
                                        ":" CPRA_STRINGIFY(__LINE__) ": " reason)


#define CPRA_CURAND_CALL(x) do { if((x)!=CURAND_STATUS_SUCCESS) { \
    printf("Error at %s:%d\n",__FILE__,__LINE__);\
    exit (EXIT_FAILURE);}} while(0)

    static const char* _cublasGetErrorEnum(cublasStatus_t error)
{
	switch (error)
	{
	case CUBLAS_STATUS_SUCCESS:
		return "CUBLAS_STATUS_SUCCESS";

	case CUBLAS_STATUS_NOT_INITIALIZED:
		return "CUBLAS_STATUS_NOT_INITIALIZED";

	case CUBLAS_STATUS_ALLOC_FAILED:
		return "CUBLAS_STATUS_ALLOC_FAILED";

	case CUBLAS_STATUS_INVALID_VALUE:
		return "CUBLAS_STATUS_INVALID_VALUE";

	case CUBLAS_STATUS_ARCH_MISMATCH:
		return "CUBLAS_STATUS_ARCH_MISMATCH";

	case CUBLAS_STATUS_MAPPING_ERROR:
		return "CUBLAS_STATUS_MAPPING_ERROR";

	case CUBLAS_STATUS_EXECUTION_FAILED:
		return "CUBLAS_STATUS_EXECUTION_FAILED";

	case CUBLAS_STATUS_INTERNAL_ERROR:
		return "CUBLAS_STATUS_INTERNAL_ERROR";

	case CUBLAS_STATUS_NOT_SUPPORTED:
		return "CUBLAS_STATUS_NOT_SUPPORTED";

	case CUBLAS_STATUS_LICENSE_ERROR:
		return "CUBLAS_STATUS_LICENSE_ERROR";

	}
	return "<unknown>";
}


#define CPRA_CUBLAS_CALL(call) {															\
     cublasStatus_t err;															    \
    if ((err = (call)) != CUBLAS_STATUS_SUCCESS) {									    \
        fprintf(stderr, "cuBLAS error %d:%s at %s:%d\n", err, _cublasGetErrorEnum(err), \
                __FILE__, __LINE__);													\
        exit(1);																		\
    }																					\
}