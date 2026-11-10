#include "Renderer/D3D11Renderer.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Geometry.hpp"
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>

namespace NeonCity::Renderer
{
    D3D11Renderer::~D3D11Renderer()
    {
        Shutdown();
    }

    bool D3D11Renderer::Initialize(HWND hwnd, uint32_t width, uint32_t height)
    {
        m_hwnd = hwnd;
        m_width = width;
        m_height = height;

        if (!CreateDeviceAndSwapChain(hwnd, width, height)) return false;
        if (!CreateRenderTargets(width, height)) return false;
        if (!CreateRasterizerStates()) return false;
        if (!CreateConstantBuffers()) return false;
        if (!LoadShaders()) return false;

        return true;
    }

    bool D3D11Renderer::CreateDeviceAndSwapChain(HWND hwnd, uint32_t width, uint32_t height)
    {
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2; // Double buffered
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        UINT createDeviceFlags = 0;
#if defined(_DEBUG)
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1
        };
        D3D_FEATURE_LEVEL featureLevel;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            _countof(featureLevels),
            D3D11_SDK_VERSION,
            &sd,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context
        );

        return SUCCEEDED(hr);
    }

    bool D3D11Renderer::CreateRenderTargets(uint32_t width, uint32_t height)
    {
        // 1. Create Render Target View from BackBuffer
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr)) return false;

        hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
        SafeRelease(backBuffer);
        if (FAILED(hr)) return false;

        // 2. Create Depth Stencil Buffer & View
        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        hr = m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer);
        if (FAILED(hr)) return false;

        hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView);
        if (FAILED(hr)) return false;

        // 3. Create Depth Stencil State
        D3D11_DEPTH_STENCIL_DESC dsDesc{};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

        hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
        if (FAILED(hr)) return false;

        // 4. Viewport
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;
        m_viewport.Width = static_cast<float>(width);
        m_viewport.Height = static_cast<float>(height);
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        return true;
    }

    bool D3D11Renderer::CreateRasterizerStates()
    {
        D3D11_RASTERIZER_DESC solidDesc{};
        solidDesc.FillMode = D3D11_FILL_SOLID;
        solidDesc.CullMode = D3D11_CULL_BACK;
        solidDesc.FrontCounterClockwise = FALSE;
        solidDesc.DepthClipEnable = TRUE;

        HRESULT hr = m_device->CreateRasterizerState(&solidDesc, &m_rasterizerSolid);
        if (FAILED(hr)) return false;

        D3D11_RASTERIZER_DESC wireDesc = solidDesc;
        wireDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireDesc.CullMode = D3D11_CULL_NONE;

        hr = m_device->CreateRasterizerState(&wireDesc, &m_rasterizerWireframe);
        return SUCCEEDED(hr);
    }

    bool D3D11Renderer::CreateConstantBuffers()
    {
        D3D11_BUFFER_DESC cbd{};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        // Per Frame Constant Buffer
        cbd.ByteWidth = sizeof(CBPerFrame);
        HRESULT hr = m_device->CreateBuffer(&cbd, nullptr, &m_cbPerFrame);
        if (FAILED(hr)) return false;

        // Per Object Constant Buffer
        cbd.ByteWidth = sizeof(CBPerObject);
        hr = m_device->CreateBuffer(&cbd, nullptr, &m_cbPerObject);
        return SUCCEEDED(hr);
    }

    bool D3D11Renderer::LoadShaders()
    {
        // Try locating shader in shaders/ directory or adjacent to binary
        const wchar_t* shaderPaths[] = {
            L"shaders/Basic3D.hlsl",
            L"../shaders/Basic3D.hlsl",
            L"Basic3D.hlsl"
        };

        const wchar_t* foundPath = nullptr;
        for (auto path : shaderPaths)
        {
            DWORD attr = GetFileAttributesW(path);
            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
            {
                foundPath = path;
                break;
            }
        }

        if (!foundPath)
        {
            foundPath = L"shaders/Basic3D.hlsl";
        }

        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        // 1. Compile Vertex Shader
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(
            foundPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VSMain", "vs_5_0", flags, 0, &vsBlob, &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                SafeRelease(errorBlob);
            }
            return false;
        }

        hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
        if (FAILED(hr))
        {
            SafeRelease(vsBlob);
            return false;
        }

        // 2. Create Input Layout matching Vertex struct
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = m_device->CreateInputLayout(layout, _countof(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
        SafeRelease(vsBlob);
        if (FAILED(hr)) return false;

        // 3. Compile Pixel Shader
        ID3DBlob* psBlob = nullptr;
        hr = D3DCompileFromFile(
            foundPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain", "ps_5_0", flags, 0, &psBlob, &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                SafeRelease(errorBlob);
            }
            return false;
        }

        hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
        SafeRelease(psBlob);

        return SUCCEEDED(hr);
    }

    void D3D11Renderer::OnResize(uint32_t width, uint32_t height)
    {
        if (!m_swapChain || width == 0 || height == 0) return;

        m_width = width;
        m_height = height;

        // Release existing views
        SafeRelease(m_renderTargetView);
        SafeRelease(m_depthStencilView);
        SafeRelease(m_depthStencilBuffer);

        // Resize swapchain buffers
        m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        CreateRenderTargets(width, height);
    }

    void D3D11Renderer::BeginFrame(const Camera& camera, float totalTime)
    {
        // Signature Project Neon City midnight navy clear color
        const float clearColor[4] = { 0.04f, 0.06f, 0.11f, 1.0f };
        m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
        m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
        m_context->OMSetDepthStencilState(m_depthStencilState, 1);
        m_context->RSSetViewports(1, &m_viewport);
        m_context->RSSetState(m_isWireframe ? m_rasterizerWireframe : m_rasterizerSolid);

        // Update Per-Frame Constant Buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_context->Map(m_cbPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            auto* cb = reinterpret_cast<CBPerFrame*>(mapped.pData);

            // Transpose for column-major HLSL register format
            DirectX::XMStoreFloat4x4(&cb->view, DirectX::XMMatrixTranspose(camera.GetView()));
            DirectX::XMStoreFloat4x4(&cb->proj, DirectX::XMMatrixTranspose(camera.GetProj()));
            cb->camPos = camera.GetPosition3f();
            cb->time = totalTime;

            // Directional sun light matching afternoon California / Neon City
            cb->lightDir = DirectX::XMFLOAT3(-0.4f, -0.8f, 0.45f);
            cb->lightColor = DirectX::XMFLOAT4(1.0f, 0.94f, 0.85f, 1.8f);

            // Atmospheric horizon fog
            cb->fogColor = DirectX::XMFLOAT4(0.04f, 0.06f, 0.11f, 1.0f);
            cb->fogStart = 15.0f;
            cb->fogRange = 350.0f;

            m_context->Unmap(m_cbPerFrame, 0);
        }

        m_context->VSSetConstantBuffers(0, 1, &m_cbPerFrame);
        m_context->PSSetConstantBuffers(0, 1, &m_cbPerFrame);

        m_context->IASetInputLayout(m_inputLayout);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->VSSetShader(m_vertexShader, nullptr, 0);
        m_context->PSSetShader(m_pixelShader, nullptr, 0);
    }

    void D3D11Renderer::RenderIndexed(
        ID3D11Buffer* vertexBuffer,
        ID3D11Buffer* indexBuffer,
        uint32_t indexCount,
        const DirectX::XMMATRIX& worldMatrix,
        const DirectX::XMFLOAT4& baseColor,
        float roughness,
        float metallic,
        float emissive
    )
    {
        // Update Per-Object Constant Buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_context->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            auto* cb = reinterpret_cast<CBPerObject*>(mapped.pData);
            DirectX::XMStoreFloat4x4(&cb->world, DirectX::XMMatrixTranspose(worldMatrix));
            cb->baseColor = baseColor;
            cb->roughness = roughness;
            cb->metallic = metallic;
            cb->emissiveIntensity = emissive;
            m_context->Unmap(m_cbPerObject, 0);
        }

        m_context->VSSetConstantBuffers(1, 1, &m_cbPerObject);
        m_context->PSSetConstantBuffers(1, 1, &m_cbPerObject);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        m_context->DrawIndexed(indexCount, 0, 0);
    }

    void D3D11Renderer::EndFrame()
    {
        m_swapChain->Present(m_enableVsync ? 1 : 0, 0);
    }

    bool D3D11Renderer::CreateVertexBuffer(const void* data, uint32_t byteSize, ID3D11Buffer** outBuffer)
    {
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = byteSize;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = data;

        return SUCCEEDED(m_device->CreateBuffer(&bd, &initData, outBuffer));
    }

    bool D3D11Renderer::CreateIndexBuffer(const void* data, uint32_t byteSize, ID3D11Buffer** outBuffer)
    {
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = byteSize;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = data;

        return SUCCEEDED(m_device->CreateBuffer(&bd, &initData, outBuffer));
    }

    void D3D11Renderer::SetWireframe(bool wireframe)
    {
        m_isWireframe = wireframe;
    }

    void D3D11Renderer::Shutdown()
    {
        SafeRelease(m_cbPerObject);
        SafeRelease(m_cbPerFrame);
        SafeRelease(m_inputLayout);
        SafeRelease(m_pixelShader);
        SafeRelease(m_vertexShader);
        SafeRelease(m_rasterizerWireframe);
        SafeRelease(m_rasterizerSolid);
        SafeRelease(m_depthStencilState);
        SafeRelease(m_depthStencilView);
        SafeRelease(m_depthStencilBuffer);
        SafeRelease(m_renderTargetView);
        SafeRelease(m_swapChain);

        if (m_context)
        {
            m_context->ClearState();
            SafeRelease(m_context);
        }

        SafeRelease(m_device);
    }
}
