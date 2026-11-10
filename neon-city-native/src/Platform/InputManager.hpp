#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <array>

namespace NeonCity::Platform
{
    class InputManager
    {
    public:
        static InputManager& Get()
        {
            static InputManager instance;
            return instance;
        }

        void BeginFrame();

        // Keyboard events
        void OnKeyDown(uint32_t virtualKey);
        void OnKeyUp(uint32_t virtualKey);

        // Mouse events
        void OnMouseMove(int32_t x, int32_t y);
        void OnRawMouseDelta(int32_t deltaX, int32_t deltaY);
        void OnMouseButtonDown(int32_t button);
        void OnMouseButtonUp(int32_t button);
        void OnMouseWheel(float delta);

        // State queries
        [[nodiscard]] bool IsKeyDown(uint32_t virtualKey) const;
        [[nodiscard]] bool IsKeyJustPressed(uint32_t virtualKey) const;
        [[nodiscard]] bool IsKeyJustReleased(uint32_t virtualKey) const;

        [[nodiscard]] bool IsMouseButtonDown(int32_t button) const;
        [[nodiscard]] bool IsMouseButtonJustPressed(int32_t button) const;

        [[nodiscard]] int32_t GetMouseX() const { return m_mouseX; }
        [[nodiscard]] int32_t GetMouseY() const { return m_mouseY; }
        [[nodiscard]] int32_t GetDeltaMouseX() const { return m_deltaMouseX; }
        [[nodiscard]] int32_t GetDeltaMouseY() const { return m_deltaMouseY; }
        [[nodiscard]] float GetMouseWheelDelta() const { return m_mouseWheelDelta; }

        void ClearState();

    private:
        InputManager() = default;

        std::array<bool, 256> m_keysDown{ false };
        std::array<bool, 256> m_keysJustPressed{ false };
        std::array<bool, 256> m_keysJustReleased{ false };

        std::array<bool, 3> m_mouseButtonsDown{ false };
        std::array<bool, 3> m_mouseButtonsJustPressed{ false };

        int32_t m_mouseX{ 0 };
        int32_t m_mouseY{ 0 };
        int32_t m_deltaMouseX{ 0 };
        int32_t m_deltaMouseY{ 0 };
        float m_mouseWheelDelta{ 0.0f };
    };
}
