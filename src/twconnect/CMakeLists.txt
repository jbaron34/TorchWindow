# add the twconnect library
add_library(twconnect STATIC twconnect.c)
target_include_directories(
    twconnect PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    $ENV{CUDA_PATH}/include
    $ENV{VULKAN_SDK}/Include
)