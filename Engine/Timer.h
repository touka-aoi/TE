#pragma once

#include <chrono>

using TimeStamp = std::chrono::time_point<std::chrono::system_clock>; // システム時間のある一点
using Duration = std::chrono::duration<float>;

class Timer
{
public:
    Timer();

    // Start()から現在時間
    float TotalTime() const;

    // Start()とStop()の間
    float DeltaTime() const;
    float GetPausedTime() const;
    float GetStopDuration() const; // (Now - stopTime)

    void Reset();
    void Start();
    void Stop();
    inline float StopGetDeltaTimeAndReset() { Stop(); float dt = DeltaTime(); Reset(); return dt; }

    float Tick();

private:
    TimeStamp baseTime;
    TimeStamp prevTime, currTime;
    TimeStamp startTime, stopTime;
    Duration pausedTime;
    Duration dt;
    bool bIsStopped;
};
