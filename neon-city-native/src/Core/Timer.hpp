#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace NeonCity::Core
{
    class Timer
    {
    public:
        Timer();

        void Reset();
        void Start();
        void Stop();
        void Tick();

        [[nodiscard]] float GetDeltaTime() const { return static_cast<float>(m_deltaTime); }
        [[nodiscard]] float GetTotalTime() const;
        [[nodiscard]] uint32_t GetFPS() const { return m_fps; }

    private:
        double m_secondsPerCount{ 0.0 };
        double m_deltaTime{ -1.0 };

        __int64 m_baseTime{ 0 };
        __int64 m_pausedTime{ 0 };
        __int64 m_stopTime{ 0 };
        __int64 m_prevTime{ 0 };
        __int64 m_currTime{ 0 };

        bool m_isStopped{ false };

        // FPS calculation
        uint32_t m_frameCount{ 0 };
        uint32_t m_fps{ 0 };
        float m_fpsTimeElapsed{ 0.0f };
    };
}
