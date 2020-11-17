# add the twvulkan library

add_library(twvulkan STATIC twvulkan.c)
target_include_directories(
    twvulkan PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/inc/SDL
    $ENV{VULKAN_SDK}/Include
)
target_link_libraries(
    twvulkan
    ${PROJECT_SOURCE_DIR}/lib/SDL2.lib
    $ENV{VULKAN_SDK}/lib/vulkan-1.lib
)

add_custom_command(TARGET twvulkan POST_BUILD
    COMMAND cmd /c "${PROJECT_SOURCE_DIR}/src/twvulkan/shaders/compile.bat" 
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/twvulkan/shaders
    COMMENT "Compiling shaders")

add_custom_command(TARGET twvulkan POST_BUILD 
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
        "${PROJECT_SOURCE_DIR}/src/twvulkan/shaders/vert.spv"
        "${PROJECT_SOURCE_DIR}/torchwindow/vert.spv" 
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
        "${PROJECT_SOURCE_DIR}/src/twvulkan/shaders/frag.spv"
        "${PROJECT_SOURCE_DIR}/torchwindow/frag.spv" 
    COMMENT "Copying shaders to torchwindow directory")

add_custom_command(TARGET twvulkan POST_BUILD 
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
            "${PROJECT_SOURCE_DIR}/lib/SDL2.dll"
            "${PROJECT_SOURCE_DIR}/torchwindow/SDL2.dll"
    COMMENT "Copying SDL2.dll to torchwindow directory")