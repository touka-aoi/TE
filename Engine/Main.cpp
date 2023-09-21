#include <Windows.h>	

#include "WIndow.h"
#include "Platform.h"

#include "Engine.h"

#include <iostream>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR pScmdl, int iCmdShow)
{
	FStartupParameters StartupParameters{};
	StartupParameters.hExeInstance = hInst; // アプリケーションハンドル 
	StartupParameters.iCmdShow = iCmdShow; // ウィンドウ表示状態
	strcpy_s(StartupParameters.EngineSettings.WndMain.Title, sizeof(StartupParameters.EngineSettings.WndMain.Title), "Main WIndow");
	strcpy_s(StartupParameters.EngineSettings.WndDebug.Title, sizeof(StartupParameters.EngineSettings.WndDebug.Title), "Debug WIndow");


    // ログの初期化
    StartupParameters.LogInitParams.bLogConsole = true;
    Log::Initialize(StartupParameters.LogInitParams);

	// セットアップのパラメータの設定 //

	{
		// エンジンの起動
		Engine Engine{};
		Engine.Initialize(StartupParameters);

		MSG msg;
		bool bQuit = false;
		// while (!bQuit && !Engine.ShouldExit())
		while (!bQuit)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					bQuit = true;
					break;
				}
			}
			// エンジンの更新
			Engine.MainThread_Tick();
		}

		// エンジンの終了
		Engine.Destroy();
	}

	return 0;
}
