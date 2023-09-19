////#include "Engine.h"
//
//#include <d3d12.h>
//#include <dxgi.h>
//
//// https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-error
//// Error Codes (DXGI_ERROR)
//static const std::unordered_map<HRESULT, std::string> DEVICE_REMOVED_MESSAGE_LOOKUP =
//{
//	{ DXGI_ERROR_ACCESS_DENIED, "You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access." },
//	{ DXGI_ERROR_ACCESS_LOST, "The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop."},
//	{ DXGI_ERROR_ALREADY_EXISTS, "The desired element already exists. This is returned by DXGIDeclareAdapterRemovalSupport if it is not the first time that the function is called."},
//	{ DXGI_ERROR_CANNOT_PROTECT_CONTENT, "DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection."},
//	{ DXGI_ERROR_DEVICE_HUNG, "The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed."},
//	{ DXGI_ERROR_DEVICE_REMOVED, "The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason."},
//	{ DXGI_ERROR_DEVICE_RESET, "The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device."},
//	{ DXGI_ERROR_DRIVER_INTERNAL_ERROR, "The driver encountered a problem and was put into the device removed state."},
//	{ DXGI_ERROR_FRAME_STATISTICS_DISJOINT, "An event (for example, a power cycle) interrupted the gathering of presentation statistics."},
//	{ DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, "The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership."},
//	{ DXGI_ERROR_INVALID_CALL, "The application provided invalid parameter data; this must be debugged and fixed before the application is released."},
//	{ DXGI_ERROR_MORE_DATA, "The buffer supplied by the application is not big enough to hold the requested data."},
//	{ DXGI_ERROR_NAME_ALREADY_EXISTS, "The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource."},
//	{ DXGI_ERROR_NONEXCLUSIVE, "A global counter resource is in use, and the Direct3D device can't currently use the counter resource."},
//	{ DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, "The resource or request is not currently available, but it might become available later."},
//	{ DXGI_ERROR_NOT_FOUND, "When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFactory::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range."},
//	{ DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, "Reserved"},
//	{ DXGI_ERROR_REMOTE_OUTOFMEMORY, "Reserved" },
//	{ DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, "The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed."},
//	{ DXGI_ERROR_SDK_COMPONENT_MISSING, "The operation depends on an SDK component that is missing or mismatched."},
//	{ DXGI_ERROR_SESSION_DISCONNECTED, "The Remote Desktop Services session is currently disconnected."},
//	{ DXGI_ERROR_UNSUPPORTED, "The requested functionality is not supported by the device or the driver."},
//	{ DXGI_ERROR_WAIT_TIMEOUT, "The time-out interval elapsed before the next desktop frame was available." },
//	{ DXGI_ERROR_WAS_STILL_DRAWING, "The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation." },
//	{ S_OK, "The method succeeded without an error." }
//};
//
//// ---------------------------------------------------------------------------
////
//// MAIN
//// 
//// ---------------------------------------------------------------------------
//void Engine::RenderThread_Tick()
//{
//	RenderThread_HandleEvent();
//
//	if (this->mbStopAllThreads)
//		return; 
//
//	RenderThread_PreRender();
//	RenderThread_RenderFrame();
//
//	RenderThread_HandleEvents();
//}
//
//// ---------------------------------------------------------------------------
////
//// INITIALIZE
////
//// ---------------------------------------------------------------------------
//static bool CheckInitialSwapchainResizeRequired(std::unordered_map<HWND, bool>& map, const FWindowSettings& setting, HWND hwnd)
//{
//	const bool bExclusiveFullscreen = setting.DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN;
//	if (bExclusiveFullscreen)
//	{
//		map[hwnd] = true; // ウィンドウハンドルをtrueに変更
//	}
//	return bExclusiveFullscreen;
//}
//
//
///**
//* 関数の概要説明
//* フルスクリーンのチェック、スワップチェーンの初期化、リソースのロード
//* TODO : RenderPassの追加
//*
//* @param[in] paramA 第一引数の説明
//* @param[out] paramB 第一引数の説明
//* @return int 戻り値の説明
//*/
//void Engine::RenderThread_Inititalize()
//{
//	// TODO : RenderPssの追加
//	//mRenderPasses = // manual render pass registration for now (early in dev)
//	//{
//	//	&mRenderPass_AO,
//	//	&mRenderPass_SSR,
//	//	&mRenderPass_ApplyReflections,
//	//	&mRenderPass_ZPrePass,
//	//	&mRenderPass_DepthResolve
//	//};
//
//	const bool bExclusiveFullscreen_MainWnd = CheckInitialSwapchainResizeRequired(mInitialSwapchainResizeRequiredWindowLookup, mSettings.WndMain, mpWinMain->GetHWND());
//
//	// Initialize swapchains for each rendering window
//	// all windows use the same number of swapchains as the main window
//	// スワップチェーンの初期化、各ウィンドウはメインウィンドウと同じ数のスワップチェーンを持つ ( 2 or 3 )
//	const int NUM_SWAPCHAIN_BUFFERS = mSettings.gfx.bUseTripleBuffering ? 3 : 2;
//	{
//		// const bool bIsContainingWindowOnHDRScreen = VQSystemInfo::FMonitorInfo::CheckHDRSupport(mpWinMain->GetHWND());
//		//const bool bCreateHDRSwapchain = mSettings.WndMain.bEnableHDR && bIsContainingWindowOnHDRScreen;
//		const bool bCreateHDRSwapchain = false;
//		/*if (mSettings.WndMain.bEnableHDR && !bIsContainingWindowOnHDRScreen)
//		{
//			Log::Warning("RenderThread_Initialize(): HDR Swapchain requested, but the containing monitor does not support HDR. Falling back to SDR Swapchain, and will enable HDR swapchain when the window is moved to a HDR-capable display");
//		}*/
//
//		mRenderer.InitializeRenderContext(mpWinMain.get(), NUM_SWAPCHAIN_BUFFERS, mSettings.gfx.bVsync, bCreateHDRSwapchain);
//		mEventQueue_VQEToWin_Main.AddItem(std::make_shared<HandleWindowTransitionsEvent>(mpWinMain->GetHWND()));
//	}
//	if (mpWinDebug)
//	{
//		// const bool bIsContainingWindowOnHDRScreen = VQSystemInfo::FMonitorInfo::CheckHDRSupport(mpWinDebug->GetHWND());
//		constexpr bool bCreateHDRSwapchain = false; // only main window in HDR for now
//		mRenderer.InitializeRenderContext(mpWinDebug.get(), NUM_SWAPCHAIN_BUFFERS, false, bCreateHDRSwapchain);
//		mEventQueue_VQEToWin_Main.AddItem(std::make_shared<HandleWindowTransitionsEvent>(mpWinDebug->GetHWND()));
//	}
//
//	// InitializeBuiltinMeshes();
//
//#if VQENGINE_MT_PIPELINED_UPDATE_AND_RENDER_THREADS
//	mbRenderThreadInitialized.store(true);
//#endif
//
//	// load builtin resources, compile shaders, load PSOs
//	mRenderer.Load(); // TODO: THREADED LOADING
//	RenderThread_LoadResources();
//
//	// initialize render passes
//	//std::vector<FPSOCreationTaskParameters> RenderPassPSOLoadDescs;
//	//for (IRenderPass* pPass : mRenderPasses)
//	//{
//	//	pPass->Initialize(); // initialize the render pass
//
//	//	// collect its PSO load descriptors so we can dispatch PSO compilation workers
//	// // .insert() で RenderPassPSOLoadDescsの末尾に追加
//	//	const auto vPassPSODescs = pPass->CollectPSOCreationParameters();
//	//	RenderPassPSOLoadDescs.insert(RenderPassPSOLoadDescs.end()
//	//		, std::make_move_iterator(vPassPSODescs.begin())
//	//		, std::make_move_iterator(vPassPSODescs.end())
//	//	);
//	//}
//
//	// compile PSOs (single-threaded)
//	/*for (auto& pr : RenderPassPSOLoadDescs)
//	{
//		*pr.pID = mRenderer.CreatePSO_OnThisThread(pr.Desc);
//	}*/
//
//	// load window resources
//	const bool bFullscreen = mpWinMain->IsFullscreen();
//	const int W = bFullscreen ? mpWinMain->GetFullscreenWidth() : mpWinMain->GetWidth();
//	const int H = bFullscreen ? mpWinMain->GetFullscreenHeight() : mpWinMain->GetHeight();
//
//	// Post process parameters are not initialized at this stage to determine the resolution scale
//	const float fResolutionScale = 1.0f;
//	RenderThread_LoadWindowSizeDependentResources(mpWinMain->GetHWND(), W, H, fResolutionScale);
//
//	mTimerRender.Reset();
//	mTimerRender.Start();
//}