#include "Engine.h"


#ifdef _DEBUG
constexpr const char* BUILD_CONFIG = "-Debug";
#else
constexpr char* BUILD_CONFIG = "";
#endif
constexpr const char* VQENGINE_VERSION = "v0.1.0";

Engine::Engine()
{
}

void Engine::MainThread_Tick()
{
	MainThread_HandleEvents();
}

bool Engine::Initialize(const FStartupParameters& Params)
{
	InitializeEngineSettings(Params);

	// InitializeHDRProfiles();
	// InitializeEnvironmentMaps();

	InitializeWindows(Params);
	InitializeInput();
	
	return true;
}

void Engine::InitializeEngineSettings(const FStartupParameters& Params)
{
	const FEngineSettings& p = Params.EngineSettings;

	// デフォルト設定
	FEngineSettings& s = mSettings;
	s.gfx.bVsync = false;
	s.gfx.bUseTripleBuffering = true;
	s.gfx.RenderScale = 1.0f;
	s.gfx.MaxFrameRate = -1; // Auto

	s.WndMain.Width = 1920;
	s.WndMain.Height = 1080;
	s.WndMain.DisplayMode = EDisplayMode::WINDOWED;
	s.WndMain.PreferredDisplay = 0;
	s.WndMain.bEnableHDR = false; 
	sprintf_s(s.WndMain.Title, "Engine %s%s", VQENGINE_VERSION, BUILD_CONFIG);

	s.WndDebug.Width = 600;
	s.WndDebug.Height = 600;
	s.WndDebug.DisplayMode = EDisplayMode::WINDOWED;
	s.WndDebug.PreferredDisplay = 1;
	s.WndDebug.bEnableHDR = false;
	sprintf_s(s.WndDebug.Title, "EngineDebug");

	s.bAutomatedTestRun = false;
	s.NumAutomatedTestFrames = 100;

	s.StartupScene = "Default";
}

void Engine::InitializeWindows(const FStartupParameters& Params)
{
	// 設定をコピーして、ウィンドウクラスを作成する
	auto fnInitializeWindow = [&](const FWindowSettings& settings, HINSTANCE hInstance, std::unique_ptr<Window>& pWin, const std::string& WindowName)
	{
		FWindowDesc desc = {};
		desc.width = settings.Width;
		desc.height = settings.Height;
		desc.hInst = hInstance;
		desc.pWndOwner = this;
		desc.pfnWndProc = WndProc;
		desc.bFullscreen = settings.DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN; // 排他的フルスクリーン設定
		desc.preferredDisplay = settings.PreferredDisplay;
		desc.iShowCmd = Params.iCmdShow;
		desc.windowName = WindowName;
		// desc.pfnRegisterWindowName = &Engine::SetWindowName;
		desc.pRegistrar = this;
		pWin.reset(new Window(settings.Title, desc));
		pWin->pOwner->OnWindowCreate(pWin->GetHWND());
	};

	fnInitializeWindow(mSettings.WndMain, Params.hExeInstance, mpWinMain, "Main Window");

	if (mSettings.bShowDebugWindow)
	{
		fnInitializeWindow(mSettings.WndDebug, Params.hExeInstance, mpWinDebug, "Debug Window");
	}
}

void Engine::InitializeInput()
{
#if ENABLE_RAW_INPUT
	// メインウィンドウのRawInputを登録する
	Input::InitRawInputDevices(mpWinMain->GetHWND());
#endif

	RegisterWindowForInput(mpWinMain);
	if (mpWinDebug) RegisterWindowForInput(mpWinDebug);
}

void Engine::RegisterWindowForInput(const std::unique_ptr<Window>& pWnd)
{
	if (!pWnd)
	{
		// LOG 
		return;
	}

	// Windowに対応するInputを作成して登録する
	mInputStates.emplace(pWnd->GetHWND(), std::move(Input()));
}

//void Engine::InitializeEngineThreads()
//{
//	const int NUM_SWAPCHAIN_BACKBUFFERS = mSettings.gfx.bUseTripleBuffering ? 3 : 2;
//	const size_t HWThreads = ThreadPool::sHardwareThreadCount;
//	const size_t HWCores = HWThreads / 2;
//	const size_t NumRuntimeWorkers = HWCores - 2; // 2スレッドはUpdateとRenderスレッドに割り当てる
//	const size_t NumLoadingWorkers = HWThreads;
//
//	mbStopAllThreads.store(false); // スレッドを実行
//
//	// アセットのロード
//	// mWorkers_ModelLoading.Initialize(NumLoadtimeWorkers, "LoadWorkers_Model");
//	// mWorkers_TextureLoading.Initialize(NumLoadtimeWorkers, "LoadWorkers_Texture");
//
//	mSimulationThread = std::thread(&Engine::SimulationThread_Main, this);
//	mWorkers_Simulation.Initialize(NumRuntimeWorkers, "SimulationWorkers");
//}

void Engine::Destroy()
{
	// スレッティングの終了
	
	// レンダラーの破棄
}

std::unique_ptr<Window>& Engine::GetWindow(HWND hwnd)
{
	if (mpWinMain->GetHWND() == hwnd)
		return mpWinMain;
	else if (mpWinDebug->GetHWND() == hwnd)
		return mpWinDebug;

	Log::Warning("VQEngine::GetWindow() : Invalid hwnd=0x%x, returning Main Window", hwnd);
	return mpWinMain;
}

const std::unique_ptr<Window>& Engine::GetWindow(HWND hwnd) const
{
	if (mpWinMain->GetHWND() == hwnd)
		return mpWinMain;
	else if (mpWinDebug->GetHWND() == hwnd)
		return mpWinDebug;

	Log::Warning("VQEngine::GetWindow() : Invalid hwnd=0x%x, returning Main Window", hwnd);
	return mpWinMain;
}

const FWindowSettings& Engine::GetWindowSettings(HWND hwnd) const
{
	if (mpWinMain->GetHWND() == hwnd)
		return mSettings.WndMain;
	else if (mpWinDebug->GetHWND() == hwnd)
		return mSettings.WndDebug;

	// TODO: handle other windows here when they're implemented

	Log::Warning("VQEngine::GetWindowSettings() : Invalid hwnd=0x%x, returning Main Window Settings", hwnd);
	return mSettings.WndMain;
}
FWindowSettings& Engine::GetWindowSettings(HWND hwnd)
{
	if (mpWinMain->GetHWND() == hwnd)
		return mSettings.WndMain;
	else if (mpWinDebug->GetHWND() == hwnd)
		return mSettings.WndDebug;

	// TODO: handle other windows here when they're implemented

	Log::Warning("VQEngine::GetWindowSettings() : Invalid hwnd=0x%x, returning Main Window Settings", hwnd);
	return mSettings.WndMain;
}