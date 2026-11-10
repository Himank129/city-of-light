#include "Platform/InputManager.hpp"

namespace NeonCity::Platform
{
    void InputManager::BeginFrame()
    {
        // Reset transient one-frame states
        m_keysJustPressed.fill(false);
        m_keysJustReleased.fill(false);
        m_mouseButtonsJustPressed.fill(false);

        m_deltaMouseX = 0;
        m_deltaMouseY = 0;
        m_mouseWheelDelta = 0.0f;
    }

    void InputManager::OnKeyDown(uint32_t virtualKey)
    {
        if (virtualKey < 256)
        {
            if (!m_keysDown[virtualKey])
            {
                m_keysJustPressed[virtualKey] = true;
            }
            m_keysDown[virtualKey] = true;
        }
    }

    void InputManager::OnKeyUp(uint32_t virtualKey)
    {
        if (virtualKey < 256)
        {
            if (m_keysDown[virtualKey])
            {
                m_keysJustReleased[virtualKey] = true;
            }
            m_keysDown[virtualKey] = false;
        }
    }

    void InputManager::OnMouseMove(int32_t x, int32_t y)
    {
        m_mouseX = x;
        m_mouseY = y;
    }

    void InputManager::OnRawMouseDelta(int32_t deltaX, int32_t deltaY)
    {
        m_deltaMouseX += deltaX;
        m_deltaMouseY += deltaY;
    }

    void InputManager::OnMouseButtonDown(int32_t button)
    {
        if (button >= 0 && button < 3)
        {
            if (!m_mouseButtonsDown[button])
            {
                m_mouseButtonsJustPressed[button] = true;
            }
            m_mouseButtonsDown[button] = true;
        }
    }

    void InputManager::OnMouseButtonUp(int32_t button)
    {
        if (button >= 0 && button < 3)
        {
            m_mouseButtonsDown[button] = false;
        }
    }

    void InputManager::OnMouseWheel(float delta)
    {
        m_mouseWheelDelta += delta;
    }

    bool InputManager::IsKeyDown(uint32_t virtualKey) const
    {
        return (virtualKey < 256) ? m_keysDown[virtualKey] : false;
    }

    bool InputManager::IsKeyJustPressed(uint32_t virtualKey) const
    {
        return (virtualKey < 256) ? m_keysJustPressed[virtualKey] : false;
    }

    bool InputManager::IsKeyJustReleased(uint32_t virtualKey) const
    {
        return (virtualKey < 256) ? m_keysJustReleased[virtualKey] : false;
    }

    bool InputManager::IsMouseButtonDown(int32_t button) const
    {
        return (button >= 0 && button < 3) ? m_mouseButtonsDown[button] : false;
    }

    bool InputManager::IsMouseButtonJustPressed(int32_t button) const
    {
        return (button >= 0 && button < 3) ? m_mouseButtonsJustPressed[button] : false;
    }

    void InputManager::ClearState()
    {
        m_keysDown.fill(false);
        m_keysJustPressed.fill(false);
        m_keysJustReleased.fill(false);
        m_mouseButtonsDown.fill(false);
        m_mouseButtonsJustPressed.fill(false);
        m_deltaMouseX = 0;
        m_deltaMouseY = 0;
        m_mouseWheelDelta = 0.0f;
    }
}
