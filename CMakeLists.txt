# CPRA
# Fangzhou Ai, UCSD @ Prof. Vitaliy Lomakin's group

cmake_minimum_required(VERSION 3.18)

project(CPRA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CUDA version
option(HAS_CUDA CACHE OFF)
# Intel MKL
option(HAS_MKL CACHE OFF)
# Build test
option(BUILD_TEST CACHE OFF)
# Set output path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# CUDA_enabled device
if(HAS_CUDA)
    find_package(CUDAToolkit REQUIRED)
    enable_language(CUDA)
    add_definitions(-DHAS_CUDA)
endif()

# Intel MKL
if(HAS_MKL)
    find_package(MKL CONFIG REQUIRED)
    add_definitions(-DHAS_MKL)
    # OpenMP
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
        add_definitions(-DHAS_OMP)
    endif()

endif()


    
    
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

if(BUILD_TEST)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()
