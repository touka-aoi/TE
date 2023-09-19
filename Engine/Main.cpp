#include <Windows.h>	

#include "WIndow.h"
#include "Platform.h"

#include "Engine.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR pScmdl, int iCmdShow)
{
	FStartupParameters StartupParameters{};
	StartupParameters.hExeInstance = hInst; // �A�v���P�[�V�����n���h�� 
	StartupParameters.iCmdShow = iCmdShow; // �E�B���h�E�\�����
	strcpy_s(StartupParameters.EngineSettings.WndMain.Title, sizeof(StartupParameters.EngineSettings.WndMain.Title), "Main WIndow");
	strcpy_s(StartupParameters.EngineSettings.WndDebug.Title, sizeof(StartupParameters.EngineSettings.WndDebug.Title), "Debug WIndow");

	// �Z�b�g�A�b�v�̃p�����[�^�̐ݒ� // 

	{
		// �G���W���̋N��
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
			// �G���W���̍X�V
			Engine.MainThread_Tick();
		}

		// �G���W���̏I��
		Engine.Destroy();
	}

	return 0;
}