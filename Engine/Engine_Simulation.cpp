#include "Engine.h"

#include <d3d12.h>
#include <dxgi.h>

//-----------------------------------------------------------------------------
//
// MAIN
//
//-----------------------------------------------------------------------------
void Engine::SimulationThread_Main()
{
    Log::Info("SimulationThread Created.");

    SimulationThread_Initialize();

    float dt = 0.0f;
    bool bQuit = false;
    while (!mbStopAllThreads && !bQuit)
    {
        dt = mTimer.Tick();

        SimulationThread_Tick(dt);

        float FrameLimiterTimeSpent = FramePacing(dt);

        // Logging
        constexpr int LOGGING_PERIOD = 4; // seconds
        static float LAST_LOG_TIME = mTimer.TotalTime();
        const float TotalTime = mTimer.TotalTime();
        if (TotalTime - LAST_LOG_TIME > LOGGING_PERIOD)
        {
            Log::Info("SimulationThread_Tick() : dt=%.2f ms (Sleep=%.2f)", dt * 1000.0f, FrameLimiterTimeSpent);
            LAST_LOG_TIME = TotalTime;
        }

    }
    SimulationThread_Exit();
}

void Engine::SimulationThread_Initialize()
{
    mNumSimulationTicks = 0;

    // RenderThread_Initialize();

    UpdateThread_Inititalize();

    Log::Info("SimulationThread Initialized.");
}

void Engine::SimulationThread_Exit()
{
    UpdateThread_Exit();
    // RenderThread_Exit();
    Log::Info("SimulationThread Exit.");
}

void Engine::SimulationThread_Tick(const float dt)
{
    // World Update
    UpdateThread_Tick(dt);

    //// UI
    //if (!(mbLoadingLevel || mbLoadingEnvironmentMap))
    //{
    //	UpdateUIState(mpWinMain->GetHWND(), dt);
    //}

    //// render 
    //RenderThread_Tick();

    // Logging
    mNumSimulationTicks++;
}
