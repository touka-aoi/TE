#pragma once 

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

		SimulationThread_Exit();
	}
}

void Engine::SimulationThread_Initialize()
{
	mNumSimulationTicks = 0;

	RenderThread_Initialize();

	UpdateThread_Initialize();

	Log::Info("SimulationThread Initialized.");
}

void Engine::SimulationThread_Exit()
{
	UpdateThread_Exit();
	RenderThread_Exit();
	Log::Info("SimulationThread Exit.");
}

void Engine::SimulationThread_Tick(const float dt)
{
	// World Update
	UpdateThread_Tick(dt);
	
	// UI
	if (!(mbLoadingLevel || mbLoadingEnvironmentMap))
	{
		UpdateUIState(mpWinMain->GetHWND(), dt);
	}

	// render 
	RenderThread_Tick();

	// Logging
	mNumSimulationTicks++;
}