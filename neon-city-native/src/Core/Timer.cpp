#include "Core/Timer.hpp"

namespace NeonCity::Core
{
    Timer::Timer()
    {
        __int64 countsPerSec;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
        m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);

        __int64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
        m_baseTime = currTime;
        m_prevTime = currTime;
    }

    void Timer::Reset()
    {
        __int64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
        m_baseTime = currTime;
        m_prevTime = currTime;
        m_stopTime = 0;
        m_pausedTime = 0;
        m_isStopped = false;
    }

    void Timer::Start()
    {
        __int64 startTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

        if (m_isStopped)
        {
            m_pausedTime += (startTime - m_stopTime);
            m_prevTime = startTime;
            m_stopTime = 0;
            m_isStopped = false;
        }
    }

    void Timer::Stop()
    {
        if (!m_isStopped)
        {
            __int64 currTime;
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
            m_stopTime = currTime;
            m_isStopped = true;
        }
    }

    void Timer::Tick()
    {
        if (m_isStopped)
        {
            m_deltaTime = 0.0;
            return;
        }

        __int64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
        m_currTime = currTime;

        m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;
        m_prevTime = m_currTime;

        // Prevent negative delta time from processor frequency throttling
        if (m_deltaTime < 0.0)
        {
            m_deltaTime = 0.0;
        }

        // FPS calculation update
        m_frameCount++;
        m_fpsTimeElapsed += static_cast<float>(m_deltaTime);
        if (m_fpsTimeElapsed >= 1.0f)
        {
            m_fps = m_frameCount;
            m_frameCount = 0;
            m_fpsTimeElapsed -= 1.0f;
        }
    }

    float Timer::GetTotalTime() const
    {
        if (m_isStopped)
        {
            return static_cast<float>(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
        }
        else
        {
            return static_cast<float>(((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
        }
    }
}
