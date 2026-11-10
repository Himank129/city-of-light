@echo off
setlocal enabledelayedexpansion

echo ==============================================================
echo PROJECT NEON CITY - Native DirectX 11 / C++20 Build Script
echo Target: AMD Ryzen 7 8845HS / Radeon 780M / Windows x64
echo ==============================================================

if not exist build mkdir build
cd build

echo.
echo [1/3] Configuring CMake with Visual Studio 2022 x64...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed. Please ensure Visual Studio 2022 with C++ Desktop Development is installed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/3] Building NeonCity.exe (Release)...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [3/3] Build successful!
echo Executable generated at: build\Release\NeonCity.exe
echo.
echo Launching PROJECT NEON CITY...
start "" "Release\NeonCity.exe"

cd ..
