#include "../public/CpuTimer.h"

CpuTimer::CpuTimer()
{
    Reset();
}

float CpuTimer::TotalTime() const
{
    using namespace std::chrono;

    if (m_Stopped)
    {
        return duration<float>((m_StopTime - m_BaseTime) - m_PausedTime).count();
    }
    else
    {
        return duration<float>((m_CurrTime - m_BaseTime) - m_PausedTime).count();
    }
}

float CpuTimer::DeltaTime() const
{
    return static_cast<float>(m_DeltaTime);
}

void CpuTimer::Reset()
{
    auto currTime = std::chrono::high_resolution_clock::now();

    m_BaseTime = currTime;
    m_PrevTime = currTime;
    m_StopTime = std::chrono::high_resolution_clock::time_point();
    m_PausedTime = std::chrono::high_resolution_clock::duration::zero();
    m_Stopped = false;
}

void CpuTimer::Start()
{
    auto startTime = std::chrono::high_resolution_clock::now();

    if (m_Stopped)
    {
        m_PausedTime += (startTime - m_StopTime);

        m_PrevTime = startTime;
        m_StopTime = std::chrono::high_resolution_clock::time_point();
        m_Stopped = false;
    }
}

void CpuTimer::Stop()
{
    if (!m_Stopped)
    {
        auto currTime = std::chrono::high_resolution_clock::now();

        m_StopTime = currTime;
        m_Stopped = true;
    }
}

void CpuTimer::Tick()
{
    if (m_Stopped)
    {
        m_DeltaTime = 0.0;
        return;
    }

    auto currTime = std::chrono::high_resolution_clock::now();
    m_CurrTime = currTime;

    m_DeltaTime = std::chrono::duration<double>(m_CurrTime - m_PrevTime).count();

    m_PrevTime = m_CurrTime;

    if (m_DeltaTime < 0.0)
    {
        m_DeltaTime = 0.0;
    }
}

bool CpuTimer::IsStopped() const
{
    return m_Stopped;
}
