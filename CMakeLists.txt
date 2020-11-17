cmake_minimum_required(VERSION 3.18)

# set the project name
project(TorchWindow VERSION 0.1.1
               DESCRIPTION "Pytorch Extension Library for Rendering CUDA Tensors"
               LANGUAGES C)

# specify the C standard
#set(CMAKE_C_STANDARD 89)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS on)

add_executable(torchwindow src/torchwindow.c)
target_include_directories(torchwindow PRIVATE inc)

add_library(twinterface SHARED src/twinterface.c)

add_subdirectory("src/twvulkan")
add_subdirectory("src/twcuda")
add_subdirectory("src/twconnect")

target_link_libraries(torchwindow twinterface)

target_include_directories(
   twinterface PUBLIC
   ${PROJECT_SOURCE_DIR}/inc
   $ENV{CUDA_PATH}/include
   $ENV{VULKAN_SDK}/Include
)
target_link_libraries(
   twinterface
   twcuda
   twconnect
)
target_link_libraries(twcuda twconnect)
target_link_libraries(twconnect twvulkan)
target_link_libraries(twvulkan twconnect)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS off)

# copy compiled library files to torchwindow directory

add_custom_command(TARGET twinterface POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
   "$<TARGET_FILE:twinterface>"
   "${PROJECT_SOURCE_DIR}/torchwindow/$<TARGET_FILE_NAME:twinterface>"
COMMENT "Copying twinterface.dll to torchwindow directory")

add_custom_command(TARGET torchwindow POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
   "$<TARGET_FILE:torchwindow>"
   "${PROJECT_SOURCE_DIR}/torchwindow/$<TARGET_FILE_NAME:torchwindow>" 
COMMENT "Copying torchwindow.exe to torchwindow directory")

add_custom_command(TARGET torchwindow POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
        "${PROJECT_SOURCE_DIR}/src/torchwindow.py"
        "${PROJECT_SOURCE_DIR}/torchwindow/torchwindow.py"
COMMENT "Copying torchwindow.py to torchwindow directory")

add_custom_command(TARGET torchwindow POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
        "${PROJECT_SOURCE_DIR}/src/__init__.py"
        "${PROJECT_SOURCE_DIR}/torchwindow/__init__.py"
COMMENT "Copying __init__.py to torchwindow directory")