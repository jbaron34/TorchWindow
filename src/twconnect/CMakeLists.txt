if(MSVC)
    set(CUDA_RT_LIB_PATH $ENV{CUDA_PATH}/lib/x64/cudart.lib)
    set(VULKAN_LIB_NAME vulkan-1.lib)
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CUDA_RT_LIB_PATH $ENV{CUDA_PATH}/lib64/libcudart.so)
    set(VULKAN_LIB_NAME libvulkan.so)
endif()

# add the twconnect library
add_library(twconnect STATIC twconnect.c)
target_include_directories(
    twconnect PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    $ENV{CUDA_PATH}/include
    $ENV{VULKAN_SDK}/Include
)
target_link_libraries(
    twconnect
    ${CUDA_RT_LIB_PATH}
    $ENV{VULKAN_SDK}/lib/${VULKAN_LIB_NAME}
)