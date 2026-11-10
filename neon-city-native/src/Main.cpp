// ==============================================================================
// PROJECT NEON CITY - Native Windows C++20 / DirectX 11 Engine Entry Point
// Stage 2 Foundation: Win32 Window, D3D11 PBR-Lite Pipeline, Raw Input, and Timer
// ==============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include "Core/Timer.hpp"
#include "Platform/Win32Window.hpp"
#include "Platform/InputManager.hpp"
#include "Renderer/D3D11Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Geometry.hpp"

using namespace NeonCity;

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPWSTR lpCmdLine,
    [[maybe_unused]] int nShowCmd)
{
    // 1. Initialize High-Precision Timer
    Core::Timer timer;
    timer.Reset();

    // 2. Initialize Win32 Window
    Platform::WindowDesc winDesc;
    winDesc.title = L"PROJECT NEON CITY - DirectX 11 Native Engine (Stage 2)";
    winDesc.width = 1280;
    winDesc.height = 720;
    winDesc.isFullscreen = false;

    Platform::Win32Window window;
    if (!window.Initialize(winDesc))
    {
        MessageBoxW(nullptr, L"Failed to initialize Win32 Window.", L"Engine Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    // 3. Initialize DirectX 11 Renderer
    Renderer::D3D11Renderer renderer;
    if (!renderer.Initialize(window.GetHWND(), window.GetWidth(), window.GetHeight()))
    {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Renderer.\nPlease ensure your GPU supports DirectX 11 Feature Level 11_0.", L"DirectX Error", MB_ICONERROR | MB_OK);
        return -2;
    }

    // 4. Initialize Camera
    Renderer::Camera camera;
    camera.SetPosition(0.0f, 3.5f, -18.0f);
    camera.SetLens(DirectX::XMConvertToRadians(65.0f), window.GetAspectRatio(), 0.1f, 1000.0f);

    // Wire up window resize to renderer and camera projection
    window.SetResizeCallback([&](uint32_t w, uint32_t h) {
        renderer.OnResize(w, h);
        camera.SetLens(DirectX::XMConvertToRadians(65.0f), static_cast<float>(w) / static_cast<float>(h), 0.1f, 1000.0f);
    });

    // 5. Generate 3D Scene Geometry
    // Floor Grid Mesh (300m x 300m)
    std::vector<Renderer::Vertex> gridVertices;
    std::vector<uint32_t> gridIndices;
    Renderer::GeometryGenerator::CreateGrid(300.0f, 300.0f, 100, 100, gridVertices, gridIndices);

    ID3D11Buffer* gridVB = nullptr;
    ID3D11Buffer* gridIB = nullptr;
    renderer.CreateVertexBuffer(gridVertices.data(), static_cast<uint32_t>(gridVertices.size() * sizeof(Renderer::Vertex)), &gridVB);
    renderer.CreateIndexBuffer(gridIndices.data(), static_cast<uint32_t>(gridIndices.size() * sizeof(uint32_t)), &gridIB);

    // Cyber Skyscraper / Monolith Box
    std::vector<Renderer::Vertex> boxVertices;
    std::vector<uint32_t> boxIndices;
    Renderer::GeometryGenerator::CreateBox(
        6.0f, 32.0f, 6.0f,
        DirectX::XMFLOAT4(0.06f, 0.08f, 0.14f, 1.0f),
        boxVertices, boxIndices
    );

    ID3D11Buffer* boxVB = nullptr;
    ID3D11Buffer* boxIB = nullptr;
    renderer.CreateVertexBuffer(boxVertices.data(), static_cast<uint32_t>(boxVertices.size() * sizeof(Renderer::Vertex)), &boxVB);
    renderer.CreateIndexBuffer(boxIndices.data(), static_cast<uint32_t>(boxIndices.size() * sizeof(uint32_t)), &boxIB);

    // Neon Accent Core Box (Central rotating glowing crystal)
    std::vector<Renderer::Vertex> neonCoreVertices;
    std::vector<uint32_t> neonCoreIndices;
    Renderer::GeometryGenerator::CreateBox(
        2.5f, 2.5f, 2.5f,
        DirectX::XMFLOAT4(0.0f, 0.94f, 1.0f, 1.0f), // Signature Neon Cyan
        neonCoreVertices, neonCoreIndices
    );

    ID3D11Buffer* neonCoreVB = nullptr;
    ID3D11Buffer* neonCoreIB = nullptr;
    renderer.CreateVertexBuffer(neonCoreVertices.data(), static_cast<uint32_t>(neonCoreVertices.size() * sizeof(Renderer::Vertex)), &neonCoreVB);
    renderer.CreateIndexBuffer(neonCoreIndices.data(), static_cast<uint32_t>(neonCoreIndices.size() * sizeof(uint32_t)), &neonCoreIB);

    // 6. Capture mouse for tactical first-person camera control
    window.SetCursorCaptured(true);

    float rotationAngle = 0.0f;
    float titleTimer = 0.0f;

    // 7. Main Native Engine Game Loop
    while (window.ProcessMessages())
    {
        timer.Tick();
        float dt = timer.GetDeltaTime();

        auto& input = Platform::InputManager::Get();

        // Keyboard Controls
        float moveSpeed = 12.0f; // m/s
        if (input.IsKeyDown(VK_SHIFT))
        {
            moveSpeed = 30.0f; // Tactical sprint
        }

        if (input.IsKeyDown('W')) camera.Walk(moveSpeed * dt);
        if (input.IsKeyDown('S')) camera.Walk(-moveSpeed * dt);
        if (input.IsKeyDown('A')) camera.Strafe(-moveSpeed * dt);
        if (input.IsKeyDown('D')) camera.Strafe(moveSpeed * dt);
        if (input.IsKeyDown(VK_SPACE)) camera.Elevate(moveSpeed * dt);
        if (input.IsKeyDown('C') || input.IsKeyDown(VK_CONTROL)) camera.Elevate(-moveSpeed * dt);

        // Toggle Wireframe with F1
        if (input.IsKeyJustPressed(VK_F1))
        {
            renderer.SetWireframe(!renderer.IsWireframe());
        }

        // Toggle VSync with V
        if (input.IsKeyJustPressed('V'))
        {
            renderer.SetVsync(!renderer.GetVsync());
        }

        // Mouse look (RawInput deltas)
        if (window.IsCursorCaptured())
        {
            const float mouseSensitivity = 0.0018f;
            int dx = input.GetDeltaMouseX();
            int dy = input.GetDeltaMouseY();

            if (dx != 0) camera.RotateY(static_cast<float>(dx) * mouseSensitivity);
            if (dy != 0) camera.Pitch(static_cast<float>(dy) * mouseSensitivity);
        }

        camera.UpdateViewMatrix();

        // Rotate central cyber monolith
        rotationAngle += 0.8f * dt;

        // Render Frame
        renderer.BeginFrame(camera, timer.GetTotalTime());

        // 1. Draw Asphalt Road Grid
        DirectX::XMMATRIX gridWorld = DirectX::XMMatrixIdentity();
        renderer.RenderIndexed(gridVB, gridIB, static_cast<uint32_t>(gridIndices.size()), gridWorld,
                               DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.85f, 0.1f, 0.0f);

        // 2. Draw Central Skyscraper Blocks (Financial District cluster)
        const DirectX::XMFLOAT3 buildingPositions[] = {
            { -30.0f, 16.0f,  40.0f },
            {  30.0f, 16.0f,  40.0f },
            { -30.0f, 16.0f, -40.0f },
            {  30.0f, 16.0f, -40.0f },
        };

        for (const auto& pos : buildingPositions)
        {
            DirectX::XMMATRIX bldWorld = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
            renderer.RenderIndexed(boxVB, boxIB, static_cast<uint32_t>(boxIndices.size()), bldWorld,
                                   DirectX::XMFLOAT4(0.15f, 0.18f, 0.24f, 1.0f), 0.3f, 0.8f, 0.05f);
        }

        // 3. Draw Central Rotating Neon Obelisk
        DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(rotationAngle);
        DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(rotationAngle * 0.5f);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0.0f, 4.0f, 0.0f);
        DirectX::XMMATRIX neonWorld = rotX * rotY * trans;

        renderer.RenderIndexed(neonCoreVB, neonCoreIB, static_cast<uint32_t>(neonCoreIndices.size()), neonWorld,
                               DirectX::XMFLOAT4(0.0f, 0.94f, 1.0f, 1.0f), 0.1f, 0.9f, 2.4f);

        renderer.EndFrame();

        // Input frame reset
        input.BeginFrame();

        // Update Window Title with Live Telemetry
        titleTimer += dt;
        if (titleTimer >= 0.25f)
        {
            titleTimer = 0.0f;
            DirectX::XMFLOAT3 pos = camera.GetPosition3f();
            std::wstringstream ss;
            ss << L"PROJECT NEON CITY [Native DirectX 11] | FPS: " << timer.GetFPS()
               << L" | Pos: (" << std::fixed << std::setprecision(1) << pos.x << L", " << pos.y << L", " << pos.z << L")"
               << (renderer.IsWireframe() ? L" | [WIREFRAME]" : L" | [PBR SOLID]")
               << (renderer.GetVsync() ? L" | [VSYNC ON]" : L" | [VSYNC OFF]")
               << L" | [WASD] Move [Shift] Sprint [Mouse] Look [F1] Wireframe [Esc] Unlock Mouse";

            SetWindowTextW(window.GetHWND(), ss.str().c_str());
        }
    }

    // 8. Cleanup Resources
    if (neonCoreIB) neonCoreIB->Release();
    if (neonCoreVB) neonCoreVB->Release();
    if (boxIB) boxIB->Release();
    if (boxVB) boxVB->Release();
    if (gridIB) gridIB->Release();
    if (gridVB) gridVB->Release();

    renderer.Shutdown();
    window.Shutdown();

    return 0;
}
