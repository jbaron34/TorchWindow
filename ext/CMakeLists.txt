cmake_minimum_required(VERSION 3.18)

# set the project name
project(RenderTorchKernels VERSION 0.1.1
                   DESCRIPTION "CUDA functions for RenderTorch"
                   LANGUAGES C CUDA)

add_library(twkern STATIC src/kernel.cu)
target_include_directories(
    twkern PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    $ENV{CUDA_PATH}/include)