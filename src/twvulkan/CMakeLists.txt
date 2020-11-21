# add the twvulkan library
if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS on)
    set(VULKAN_LIB_NAME vulkan-1.lib)
    set(SDL_LIB_NAME SDL2.lib)
    set(SDL_DYNAMIC_LIB_NAME SDL.dll)
else()
    set(VULKAN_LIB_NAME libvulkan.so)
    set(SDL_LIB_NAME libSDL2.so)
    set(SDL_DYNAMIC_LIB_NAME libSDL2.so)
endif()

add_library(twvulkan STATIC twvulkan.c)
target_include_directories(
    twvulkan PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/inc/SDL
    $ENV{VULKAN_SDK}/Include
)
target_link_libraries(
    twvulkan
    ${PROJECT_SOURCE_DIR}/lib/${SDL_LIB_NAME}
    $ENV{VULKAN_SDK}/lib/${VULKAN_LIB_NAME}
)

if (MSVC)
add_custom_command(TARGET twvulkan POST_BUILD
    COMMAND sh "${PROJECT_SOURCE_DIR}/src/twvulkan/shaders/compile.bat" 
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/twvulkan/shaders
    COMMENT "Compiling shaders")
else()
add_custom_command(TARGET twvulkan POST_BUILD
    COMMAND sh "${PROJECT_SOURCE_DIR}/src/twvulkan/shaders/compile.sh" 
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/twvulkan/shaders
    COMMENT "Compiling shaders")
endif()

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
            "${PROJECT_SOURCE_DIR}/lib/${SDL_DYNAMIC_LIB_NAME}"
            "${PROJECT_SOURCE_DIR}/torchwindow/${SDL_DYNAMIC_LIB_NAME}"
    COMMENT "Copying ${SDL_DYNAMIC_LIB_NAME} to torchwindow directory")