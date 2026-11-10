cmake_minimum_required(VERSION 3.20)
project(NeonCity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enforce UTF-8 and clean warning flags
if (MSVC)
    add_compile_options(/W4 /utf-8 /permissive- /Zc:__cplusplus)
    # Enable SIMD enhancements for AMD Ryzen / AVX2
    add_compile_options(/arch:AVX2)
elseif (MINGW)
    add_compile_options(-Wall -Wextra -std=c++20 -municode -mavx2)
endif()

# Source files
set(SOURCES
    src/Main.cpp
    src/Core/Timer.cpp
    src/Platform/Win32Window.cpp
    src/Platform/InputManager.cpp
    src/Renderer/D3D11Renderer.cpp
    src/Renderer/Camera.cpp
)

set(HEADERS
    src/Core/Timer.hpp
    src/Platform/Win32Window.hpp
    src/Platform/InputManager.hpp
    src/Renderer/D3D11Renderer.hpp
    src/Renderer/Camera.hpp
    src/Renderer/Geometry.hpp
)

# Shaders
set(SHADERS
    shaders/Basic3D.hlsl
)

# Executable target (Windows GUI application)
add_executable(NeonCity WIN32
    ${SOURCES}
    ${HEADERS}
    ${SHADERS}
)

target_include_directories(NeonCity PRIVATE
    src
)

# Target DirectX 11 and Win32 system libraries
if (MSVC)
    target_link_libraries(NeonCity PRIVATE
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        dxguid.lib
        user32.lib
        gdi32.lib
    )
elseif (MINGW)
    target_link_libraries(NeonCity PRIVATE
        -ld3d11
        -ldxgi
        -ld3dcompiler
        -ldxguid
        -luser32
        -lgdi32
    )
endif()

# Copy shader files next to executable on build
add_custom_command(TARGET NeonCity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:NeonCity>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/Basic3D.hlsl
        $<TARGET_FILE_DIR:NeonCity>/shaders/Basic3D.hlsl
    COMMENT "Copying HLSL shaders to output directory..."
)
