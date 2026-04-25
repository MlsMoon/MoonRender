//***************************************************************************************
// CpuTimer.h by Frank Luna (C) 2011 All Rights Reserved.
// Modify name from GameTimer.cpp
// CPU计时器
//***************************************************************************************

#pragma once

#ifndef CPU_TIMER_H
#define CPU_TIMER_H

#include <chrono>

class CpuTimer
{
public:
    CpuTimer();

    float TotalTime() const;
    float DeltaTime() const;

    void Reset();
    void Start();
    void Stop();
    void Tick();
    bool IsStopped() const;

private:
    double m_DeltaTime = -1.0;

    std::chrono::high_resolution_clock::time_point m_BaseTime;
    std::chrono::high_resolution_clock::duration m_PausedTime;
    std::chrono::high_resolution_clock::time_point m_StopTime;
    std::chrono::high_resolution_clock::time_point m_PrevTime;
    std::chrono::high_resolution_clock::time_point m_CurrTime;

    bool m_Stopped = false;
};

#endif // CPU_TIMER_H
