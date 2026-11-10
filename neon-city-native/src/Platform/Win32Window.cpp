#include "Platform/Win32Window.hpp"
#include "Platform/InputManager.hpp"
#include <hidusage.h>

namespace NeonCity::Platform
{
    Win32Window::~Win32Window()
    {
        Shutdown();
    }

    bool Win32Window::Initialize(const WindowDesc& desc)
    {
        m_hInstance = GetModuleHandleW(nullptr);
        m_width = desc.width;
        m_height = desc.height;

        // Try setting Per-Monitor V2 DPI Awareness for crisp modern high-DPI scaling
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // Register Window Class
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = StaticWndProc;
        wc.hInstance = m_hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = m_className.c_str();

        if (!RegisterClassExW(&wc))
        {
            return false;
        }

        // Calculate actual window rect based on desired client area
        RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
        DWORD style = WS_OVERLAPPEDWINDOW;
        AdjustWindowRectEx(&windowRect, style, FALSE, 0);

        int posX = (GetSystemMetrics(SM_CXSCREEN) - (windowRect.right - windowRect.left)) / 2;
        int posY = (GetSystemMetrics(SM_CYSCREEN) - (windowRect.bottom - windowRect.top)) / 2;

        m_hwnd = CreateWindowExW(
            0,
            m_className.c_str(),
            desc.title.c_str(),
            style,
            posX, posY,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            m_hInstance,
            this
        );

        if (!m_hwnd)
        {
            return false;
        }

        RegisterRawInput();

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);

        return true;
    }

    void Win32Window::RegisterRawInput()
    {
        // Register standard mouse for high precision RawInput (WM_INPUT)
        RAWINPUTDEVICE rid{};
        rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
        rid.usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
        rid.dwFlags = 0;
        rid.hwndTarget = m_hwnd;

        RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
    }

    void Win32Window::Shutdown()
    {
        SetCursorCaptured(false);

        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }

        if (m_hInstance)
        {
            UnregisterClassW(m_className.c_str(), m_hInstance);
            m_hInstance = nullptr;
        }
    }

    bool Win32Window::ProcessMessages()
    {
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        return true;
    }

    void Win32Window::SetCursorCaptured(bool captured)
    {
        if (m_isCursorCaptured == captured)
        {
            return;
        }

        m_isCursorCaptured = captured;

        if (m_isCursorCaptured)
        {
            ShowCursor(FALSE);
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            MapWindowPoints(m_hwnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
            ClipCursor(&rect);
        }
        else
        {
            ClipCursor(nullptr);
            ShowCursor(TRUE);
        }
    }

    LRESULT CALLBACK Win32Window::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32Window* pThis = nullptr;

        if (msg == WM_NCCREATE)
        {
            auto* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hwnd = hwnd;
        }
        else
        {
            pThis = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT Win32Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto& input = InputManager::Get();

        switch (msg)
        {
        case WM_ACTIVATE:
            m_isActive = (LOWORD(wParam) != WA_INACTIVE);
            if (!m_isActive && m_isCursorCaptured)
            {
                SetCursorCaptured(false);
            }
            return 0;

        case WM_SIZE:
        {
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            m_isMinimized = (wParam == SIZE_MINIMIZED);

            if (m_width > 0 && m_height > 0 && !m_isMinimized && m_onResize)
            {
                m_onResize(m_width, m_height);
            }
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (!(lParam & (1 << 30))) // Filter out repeat keys
            {
                input.OnKeyDown(static_cast<uint32_t>(wParam));
            }
            if (wParam == VK_ESCAPE)
            {
                SetCursorCaptured(false);
            }
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            input.OnKeyUp(static_cast<uint32_t>(wParam));
            return 0;

        case WM_MOUSEMOVE:
            input.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_LBUTTONDOWN:
            input.OnMouseButtonDown(0);
            if (!m_isCursorCaptured)
            {
                SetCursorCaptured(true);
            }
            return 0;

        case WM_LBUTTONUP:
            input.OnMouseButtonUp(0);
            return 0;

        case WM_RBUTTONDOWN:
            input.OnMouseButtonDown(1);
            return 0;

        case WM_RBUTTONUP:
            input.OnMouseButtonUp(1);
            return 0;

        case WM_MBUTTONDOWN:
            input.OnMouseButtonDown(2);
            return 0;

        case WM_MBUTTONUP:
            input.OnMouseButtonUp(2);
            return 0;

        case WM_MOUSEWHEEL:
            input.OnMouseWheel(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA);
            return 0;

        case WM_INPUT:
        {
            UINT dwSize = 0;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
            if (dwSize > 0)
            {
                std::vector<BYTE> rawBuffer(dwSize);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
                {
                    auto* raw = reinterpret_cast<RAWINPUT*>(rawBuffer.data());
                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        input.OnRawMouseDelta(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
                    }
                }
            }
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
