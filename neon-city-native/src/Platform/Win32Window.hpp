#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string>
#include <functional>

namespace NeonCity::Platform
{
    struct WindowDesc
    {
        std::wstring title{ L"Project Neon City - DirectX 11 Native Engine" };
        uint32_t width{ 1280 };
        uint32_t height{ 720 };
        bool isFullscreen{ false };
    };

    class Win32Window
    {
    public:
        using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

        Win32Window() = default;
        ~Win32Window();

        bool Initialize(const WindowDesc& desc);
        void Shutdown();

        bool ProcessMessages();

        [[nodiscard]] HWND GetHWND() const { return m_hwnd; }
        [[nodiscard]] uint32_t GetWidth() const { return m_width; }
        [[nodiscard]] uint32_t GetHeight() const { return m_height; }
        [[nodiscard]] float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
        [[nodiscard]] bool IsActive() const { return m_isActive; }
        [[nodiscard]] bool IsMinimized() const { return m_isMinimized; }

        void SetResizeCallback(ResizeCallback callback) { m_onResize = std::move(callback); }
        void SetCursorCaptured(bool captured);
        [[nodiscard]] bool IsCursorCaptured() const { return m_isCursorCaptured; }

    private:
        static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        void RegisterRawInput();

        HWND m_hwnd{ nullptr };
        HINSTANCE m_hInstance{ nullptr };
        std::wstring m_className{ L"NeonCityWindowClass" };

        uint32_t m_width{ 1280 };
        uint32_t m_height{ 720 };
        bool m_isActive{ true };
        bool m_isMinimized{ false };
        bool m_isCursorCaptured{ false };

        ResizeCallback m_onResize;
    };
}
