#pragma once

#include <Windows.h>
#include "Settings.h"

#include "Types.h"

// コンフィグ設定

struct FStartupParameters
{
	HINSTANCE                 hExeInstance;
	int                       iCmdShow;
	// Log::LogInitializeParams  LogInitParams;

	FEngineSettings EngineSettings;
	uint8 bOverrideGFXSetting_RenderScale : 1;
	uint8 bOverrideGFXSetting_bVSync : 1;
	uint8 bOverrideGFXSetting_bUseTripleBuffering : 1;
	uint8 bOverrideGFXSetting_bAA : 1;
	uint8 bOverrideGFXSetting_bMaxFrameRate : 1;
	uint8 bOverrideGFXSetting_bHDR : 1;
	uint8 bOverrideGFXSetting_EnvironmentMapResolution : 1;
	uint8 bOverrideGFXSettings_Reflections : 1;

	uint8 bOverrideENGSetting_MainWindowHeight : 1;
	uint8 bOverrideENGSetting_MainWindowWidth : 1;
	uint8 bOverrideENGSetting_bDisplayMode : 1;
	uint8 bOverrideENGSetting_PreferredDisplay : 1;

	uint8 bOverrideENGSetting_bDebugWindowEnable : 1;
	uint8 bOverrideENGSetting_DebugWindowHeight : 1;
	uint8 bOverrideENGSetting_DebugWindowWidth : 1;
	uint8 bOverrideENGSetting_DebugWindowDisplayMode : 1;
	uint8 bOverrideENGSetting_DebugWindowPreferredDisplay : 1;

	uint8 bOverrideENGSetting_bAutomatedTest : 1;
	uint8 bOverrideENGSetting_bTestFrames : 1;
	uint8 bOverrideENGSetting_StartupScene : 1;
};

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 循環加算
template<typename T> static inline T CircularIncrement(T currVal, T maxVal) { return (currVal + 1) % maxVal; }
// 循環減算 (最小値の場合、最大値にループする)
template<typename T> static inline T CircularDecrement(T currVal, T maxVal, T minVal = 0) { return currVal == minVal ? maxVal - 1 : currVal - 1; }

