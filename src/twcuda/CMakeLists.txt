if(MSVC)
    set(KERNEL_LIB_PATH ${PROJECT_SOURCE_DIR}/ext/build/$<CONFIG>/twkern.lib)
    set(CUDA_RT_LIB_PATH $ENV{CUDA_PATH}/lib/x64/cudart.lib)
else()
    set(KERNEL_LIB_PATH ${PROJECT_SOURCE_DIR}/ext/build/libtwkern.a)
    set(CUDA_RT_LIB_PATH $ENV{CUDA_PATH}/lib64/libcudart_static.a)
endif()

# build the external kernel library
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/ext/build)
execute_process(COMMAND "cmake" ".."
   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/ext/build")
add_custom_target(kernel ALL
   COMMAND cmake "--build" "." "--config" "$<CONFIG>"
   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/ext/build"
   COMMENT "running external kernel build")

# add the twcuda library
add_library(twcuda STATIC twcuda.c)
target_include_directories(
    twcuda PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/ext/inc
    $ENV{CUDA_PATH}/include
)
target_link_libraries(
    twcuda
    ${KERNEL_LIB_PATH}
    ${CUDA_RT_LIB_PATH}
)
add_dependencies(twcuda kernel)