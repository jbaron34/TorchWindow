cmake_minimum_required(VERSION 3.13)

# set the project name
project(RenderTorchKernels VERSION 0.1.1
                   DESCRIPTION "CUDA functions for RenderTorch"
                   LANGUAGES C CUDA)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(twkern STATIC src/kernel.cu)
target_include_directories(
    twkern PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    $ENV{CUDA_PATH}/include)

#add_custom_command(TARGET twkern POST_BUILD 
#    COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
#            "$<TARGET_FILE:twkern>"
#            "${PROJECT_SOURCE_DIR}/../lib/$<TARGET_FILE_NAME:twkern>"
#    COMMENT "Copying $<TARGET_FILE_NAME:twkern> to twkern directory")