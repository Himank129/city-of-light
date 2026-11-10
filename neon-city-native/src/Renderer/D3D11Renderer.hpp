#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <cstdint>
#include <string>
#include <vector>

namespace NeonCity::Renderer
{
    class Camera;

    // Aligned to 16 bytes for HLSL constant buffer rules
    struct alignas(16) CBPerFrame
    {
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 proj;
        DirectX::XMFLOAT3   camPos;
        float               time;
        DirectX::XMFLOAT3   lightDir;
        float               padding0;
        DirectX::XMFLOAT4   lightColor;
        DirectX::XMFLOAT4   fogColor;
        float               fogStart;
        float               fogRange;
        DirectX::XMFLOAT2   padding1;
    };

    struct alignas(16) CBPerObject
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4   baseColor;
        float               roughness;
        float               metallic;
        float               emissiveIntensity;
        float               padding2;
    };

    class D3D11Renderer
    {
    public:
        D3D11Renderer() = default;
        ~D3D11Renderer();

        bool Initialize(HWND hwnd, uint32_t width, uint32_t height);
        void Shutdown();

        void OnResize(uint32_t width, uint32_t height);

        void BeginFrame(const Camera& camera, float totalTime);
        void EndFrame();

        void RenderIndexed(
            ID3D11Buffer* vertexBuffer,
            ID3D11Buffer* indexBuffer,
            uint32_t indexCount,
            const DirectX::XMMATRIX& worldMatrix,
            const DirectX::XMFLOAT4& baseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            float roughness = 0.5f,
            float metallic = 0.0f,
            float emissive = 0.0f
        );

        bool CreateVertexBuffer(const void* data, uint32_t byteSize, ID3D11Buffer** outBuffer);
        bool CreateIndexBuffer(const void* data, uint32_t byteSize, ID3D11Buffer** outBuffer);

        void SetWireframe(bool wireframe);
        [[nodiscard]] bool IsWireframe() const { return m_isWireframe; }

        void SetVsync(bool vsync) { m_enableVsync = vsync; }
        [[nodiscard]] bool GetVsync() const { return m_enableVsync; }

        [[nodiscard]] ID3D11Device* GetDevice() const { return m_device; }
        [[nodiscard]] ID3D11DeviceContext* GetContext() const { return m_context; }

    private:
        bool CreateDeviceAndSwapChain(HWND hwnd, uint32_t width, uint32_t height);
        bool CreateRenderTargets(uint32_t width, uint32_t height);
        bool CreateRasterizerStates();
        bool CreateConstantBuffers();
        bool LoadShaders();

        template <typename T>
        void SafeRelease(T*& ptr)
        {
            if (ptr)
            {
                ptr->Release();
                ptr = nullptr;
            }
        }

        HWND m_hwnd{ nullptr };
        uint32_t m_width{ 1280 };
        uint32_t m_height{ 720 };
        bool m_enableVsync{ true };
        bool m_isWireframe{ false };

        ID3D11Device* m_device{ nullptr };
        ID3D11DeviceContext* m_context{ nullptr };
        IDXGISwapChain* m_swapChain{ nullptr };

        ID3D11RenderTargetView* m_renderTargetView{ nullptr };
        ID3D11Texture2D* m_depthStencilBuffer{ nullptr };
        ID3D11DepthStencilView* m_depthStencilView{ nullptr };
        ID3D11DepthStencilState* m_depthStencilState{ nullptr };

        ID3D11RasterizerState* m_rasterizerSolid{ nullptr };
        ID3D11RasterizerState* m_rasterizerWireframe{ nullptr };

        ID3D11VertexShader* m_vertexShader{ nullptr };
        ID3D11PixelShader* m_pixelShader{ nullptr };
        ID3D11InputLayout* m_inputLayout{ nullptr };

        ID3D11Buffer* m_cbPerFrame{ nullptr };
        ID3D11Buffer* m_cbPerObject{ nullptr };

        D3D11_VIEWPORT m_viewport{};
    };
}
