#include "Engine.h"

void Engine::UpdateThread_Main()
{
    Log::Info("UpdateThread Created");

    UpdateThread_Inititalize();

    float dt = 0.0f;
    bool bQuit = false;
    while (!mbStopAllThreads && !bQuit)
    {
        dt = mTimer.Tick();

        UpdateThread_Tick(dt);

        // UpdateThread_Logging()
        constexpr int LOGGING_PERIOD = 4; // seconds
        static float LAST_LOG_TIME = 0;
        const float TotalTime = mTimer.TotalTime();
        if (TotalTime - LAST_LOG_TIME > 4)
        {
            Log::Info("UpdateTick() : dt=%.2f ms", (dt * 1000.0f) /* - (dt_RenderWaitTime * 1000.0f)*/);
            LAST_LOG_TIME = TotalTime;
        }

        UpdateThread_Exit();
        Log::Info("UpdateThread_Main() : Exit");
    }
}

void Engine::UpdateThread_Inititalize()
{
    // InitializeUI(mpWinMain->GetHWND());

    mTimer.Reset();
    mTimer.Start();
}

void Engine::UpdateThread_Tick(const float dt)
{
    float dt_RenderWaitTime = 0.0f;

    // TODO : Windows PIX

    // dt_RenderWaitTime = UpdateThread_WaitForRenderThread();

    UpdateThread_HandleEvents();

    UpdateThread_PreUpdate();

    UpdateThread_UpdateAppState(dt);

    UpdateThread_PostUpdate();
}

void Engine::UpdateThread_Exit()
{
    // ExitUI();
}

void Engine::UpdateThread_PreUpdate()
{
    // TODO : Windows PIX

    // const int NUM_BACK_BUFFERS = mRenderer.GetSwapChainBackBufferCount(mpWinMain->GetHWND());

    // TODO : Scene Update
}

/**
 * @fn
 * アプリケーションの状態を管理する関数
 */
void Engine::UpdateThread_UpdateAppState(const float dt)
{
    // TODO ; Windows PIX

    switch (mAppState)
    {
    case EAppState::INITIALIZING:
        mAppState = EAppState::LOADING;
        break;
    case EAppState::LOADING:
        mAppState = EAppState::SIMULATING;
        break;
    case EAppState::SIMULATING:
        // UpdateThread_UpdateScene_MainWnd(dt);
        // UpdateThread_UpdateScene_DebugWnd(dt);
        break;
    }
}

void Engine::UpdateThread_PostUpdate()
{
    // TODO : Windows PIX

    // 各々のWindowハンドルを取得、InputのUpdateを行う
    for (auto it = mInputStates.begin(); it != mInputStates.end(); ++it)
    {
        const HWND& hwnd = it->first;
        mInputStates.at(hwnd).PostUpdate();
    }
}

void Engine::SetWindowName(HWND hwnd, const std::string& name) { mWinNameLookup[hwnd] = name; }
void Engine::SetWindowName(const std::unique_ptr<Window>& pWin, const std::string& name) { SetWindowName(pWin->GetHWND(), name); }

const std::string& Engine::GetWindowName(HWND hwnd) const
{
#if _DEBUG
    auto it = mWinNameLookup.find(hwnd);
    if (it == mWinNameLookup.end())
    {
        Log::Error("Couldn't find window<%x> name: HWND not called with SetWindowName()!", hwnd);
        // assert(false); // gonna crash at .at() call anyways.
    }
#endif
    return mWinNameLookup.at(hwnd);
}

bool Engine::IsWindowRegistered(HWND hwnd) const
{
    auto it = mWinNameLookup.find(hwnd);
    return it != mWinNameLookup.end();
}
