# PROJECT NEON CITY — Native DirectX 11 / C++20 Engine

A native Windows PC open-world game built from the ground up without web frameworks, Electron, or high-level game engines.

## Stage 2 Foundation Features

- **Language & Standard**: C++20 with strict RAII, SIMD math, and zero garbage collection pauses.
- **Platform Layer**: Native Win32 (`WNDCLASSEXW`), Per-Monitor V2 high-DPI scaling, and Raw Input (`WM_INPUT`) for sub-pixel FPS mouse look.
- **Rendering API**: DirectX 11 (`ID3D11Device`, `ID3D11DeviceContext`, `IDXGISwapChain`) targeting Feature Level 11_0.
- **Shaders**: HLSL 5.0 Vertex and Pixel shaders (`shaders/Basic3D.hlsl`) with PBR-Lite lighting (Directional sun, Blinn-Phong specular, ambient hemisphere, neon emissives, and atmospheric distance fog).
- **Camera Rig**: Dual-mode First-Person and Third-Person camera with orthonormal basis updates and smooth mouse pitch/yaw.
- **Input System**: Keyboard states (down, justPressed, justReleased) and mouse wheel tracking.
- **Timing**: Sub-millisecond `QueryPerformanceCounter` game loop timer with live FPS and delta time computation.

## Directory Structure

```
neon-city-native/
├── CMakeLists.txt              # Standard C++20 & DirectX 11 CMake build definition
├── vcpkg.json                  # Native dependency manifest
├── build.bat                   # 1-click Windows build and launch script
├── shaders/
│   └── Basic3D.hlsl            # HLSL 5.0 PBR-Lite & Fog shader
└── src/
    ├── Main.cpp                # Native WinMain entry point, 3D test scene, game loop
    ├── Core/
    │   ├── Timer.hpp           # High-precision timer header
    │   └── Timer.cpp           # QPC delta time and FPS tracking
    ├── Platform/
    │   ├── Win32Window.hpp     # Win32 window class header
    │   ├── Win32Window.cpp     # Message pump, DPI awareness, resize handling
    │   ├── InputManager.hpp    # Keyboard & Raw mouse manager header
    │   └── InputManager.cpp    # State arrays and transient event clearing
    └── Renderer/
        ├── D3D11Renderer.hpp   # DirectX 11 device, swapchain, constant buffers
        ├── D3D11Renderer.cpp   # Pipeline initialization, clear, draw calls, present
        ├── Camera.hpp          # FPS/TPS camera header
        ├── Camera.cpp          # Matrix look-at & perspective projection math
        └── Geometry.hpp        # Vertex definition and procedural 3D meshes
```

## Quick Build Instructions

### Method 1: Visual Studio 2022
1. Launch Visual Studio 2022.
2. Click **Open a local folder** and select `neon-city-native`.
3. Choose `x64-Release` from the configuration toolbar.
4. Press **F5** or click **Build > Build All**.

### Method 2: Command Line (CMake)
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
./build/Release/NeonCity.exe
```

## Default Controls

- `WASD`: Move forward / backward / strafe
- `Shift`: Tactical sprint (2.5x speed)
- `Space`: Ascend / elevate
- `C` / `Ctrl`: Descend
- `Mouse`: High-precision 360° first-person look
- `F1`: Toggle Wireframe / PBR Solid rendering
- `V`: Toggle VSync (60 FPS cap / uncapped)
- `Esc`: Release mouse cursor from window
- `Left Click`: Re-lock mouse cursor to viewport
