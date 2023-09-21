#include "Engine.h"

#include <dwmapi.h>


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

/**
* @fn
* フレームレートを一定にする関数
*/
float Engine::FramePacing(float dt)
{
    float SleepTime = 0.0f;
    // フレームレートの制限が設定されている場合
    if (mEffectiveFrameRateLimit_ms != 0.0f)
    {
        // TODO : Windows PIX

        // 消費すべき時間の算出
        const float TimeBudgetLeft_ms = mEffectiveFrameRateLimit_ms - dt;
        // 待機時間がある場合
        if (TimeBudgetLeft_ms)
        {
            float Acc = TimeBudgetLeft_ms / 1000;
            Timer SleepTimer;
            SleepTimer.Start();
            while (Acc > 0.0f)
            {
                Sleep(0);
                Acc -= SleepTimer.Tick();
            }
        }
    }
    return SleepTime;
}

void Engine::SetEffectiveFrameRateLimit()
{
    // Auto
    if (mSettings.gfx.MaxFrameRate == -1)
    {
        // モニターのリフレッシュレートを取得
        DWM_TIMING_INFO dti = {};
        dti.cbSize = sizeof(DWM_TIMING_INFO);
        HRESULT hr = DwmGetCompositionTimingInfo(NULL, &dti);
        assert(dti.rateRefresh.uiDenominator != 0 && dti.rateRefresh.uiNumerator != 0);
        const float DisplayRefreshRate = static_cast<float>(dti.rateRefresh.uiNumerator) / dti.rateRefresh.uiDenominator;
        Log::Info("Getting Monitor Refresh Rate: %.1fHz", DisplayRefreshRate);
        // 周期に余裕を持たせHzをmsに変換
        mEffectiveFrameRateLimit_ms = 1000.0f / (DisplayRefreshRate * 1.15f);
    }
    // Unlimited
    else if ( mSettings.gfx.MaxFrameRate == 0)
    {
        mEffectiveFrameRateLimit_ms = 0.0f;
    }
    // Custom
    else
    {
        mEffectiveFrameRateLimit_ms = 1000.0f / mSettings.gfx.MaxFrameRate;
    }
    const bool bUnlimitedFrameRate = mEffectiveFrameRateLimit_ms == 0.0f;
    if (bUnlimitedFrameRate) Log::Info("FrameRateLimit : Unlimited");
    else                     Log::Info("FrameRateLimit : %.2fms | %d FPS", mEffectiveFrameRateLimit_ms, static_cast<int>(1000.0f / mEffectiveFrameRateLimit_ms));
}
