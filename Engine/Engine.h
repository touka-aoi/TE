#pragma once

#include "Window.h"
#include "Log.h"
#include "Input.h"
#include "Platform.h"
#include "Multithreading.h"

#include <memory>

class Engine : public IWindowOwner
{
public:
    Engine();

    // OS Window Events
    void OnWindowCreate(HWND hwnd) override;
    void OnWindowResize(HWND hwnd) override;
    void OnWindowMinimize(HWND hwnd) override;
    void OnWindowFocus(HWND hwnd) override;
    void OnWindowLoseFocus(HWND hwnd) override;
    void OnWindowClose(HWND hwnd) override;
    void OnToggleFullscreen(HWND hwnd) override;
    void OnWindowActivate(HWND hwnd) override;
    void OnWindowDeactivate(HWND hwnd) override;
    void OnWindowMove(HWND hwnd_, int x, int y) override;
    void OnDisplayChange(HWND hwnd_, int ImageDepthBitsPerPixel, int ScreenWidth, int ScreenHeight) override;

    // Keyboard & Mouse Events
    void OnKeyDown(HWND hwnd, WPARAM wParam) override;
    void OnKeyUp(HWND hwnd, WPARAM wParam) override;
    void OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick) override;
    void OnMouseButtonUp(HWND hwnd, WPARAM wParam) override;
    void OnMouseScroll(HWND hwnd, short scroll) override;
    void OnMouseMove(HWND hwnd, long x, long y) override;
    void OnMouseInput(HWND hwnd, LPARAM lParam) override;

    // ---------------------------------------------------------
    // Main Thread
    // ---------------------------------------------------------
    void MainThread_Tick();
    bool Initialize(const FStartupParameters& Params);
    void Destroy();
    // inline bool ShouldExit() const { return mbExitApp.load(); }

    // ---------------------------------------------------------
    // Simulation Thread
    // ---------------------------------------------------------
    /*void SimulationThread_Main();
    void SimulationThread_Initialize();
    void SimulationThread_Exit();
    void SimulationThread_Tick(const float dt);*/

    // ---------------------------------------------------------
    // Render Thread
    // ---------------------------------------------------------
    /*void RenderThread_Main();
    void RenderThread_Tick();
    void RenderThread_Inititalize();
    void RenderThread_Exit();*/

    // ---------------------------------------------------------
    // Update Thread
    // ---------------------------------------------------------
    /*void  UpdateThread_Main();
    void  UpdateThread_Inititalize();
    void  UpdateThread_Tick(const float dt);
    void  UpdateThread_Exit();
    float UpdateThread_WaitForRenderThread();
    void  UpdateThread_SignalRenderThread();*/


    //---------------------------------------------------------
    void                       SetWindowName(HWND hwnd, const std::string& name);
    void                       SetWindowName(const std::unique_ptr<Window>& pWin, const std::string& name);
    const std::string& GetWindowName(HWND hwnd) const;
    // overload
    inline const std::string& GetWindowName(const std::unique_ptr<Window>& pWin) const { return GetWindowName(pWin->GetHWND()); }
    inline const std::string& GetWindowName(const Window* pWin) const { return GetWindowName(pWin->GetHWND()); }

private:

    using EventPtr_t = std::shared_ptr<IEvent>;
    using EventQueue_t = BufferedContainer<std::queue<EventPtr_t>, EventPtr_t>;

    // windows
    std::unique_ptr<Window>         mpWinMain;                              // ウィンドウハンドル
    std::unique_ptr<Window>         mpWinDebug;                             // デバッグウィンドウハンドル

    POINT                           mMouseCapturePosition;  // マウスキャプチャ位置

    // Renderer
    // Renderer                      mRenderer;

    // input
    std::unordered_map<HWND, Input> mInputStates; // ウィンドウごとのInputクラスを保持する

    // events
    EventQueue_t                    mEventQueue_EnToWin_Main;
    EventQueue_t                    mEventQueue_WinToE_Renderer;
    EventQueue_t                    mEventQueue_WinToE_Update;

    // threads
    /*std::thread                     mSimulationThread;
    ThreadPool                      mWorkers_Simulation;

    ThreadPool                      mWorkers_ModelLoading;
    ThreadPool                      mWorkers_TextureLoading;*/

    // sync
    //std::atomic<bool>               mbStopAllThreads;

    // system & settings
    FEngineSettings                 mSettings;

    // timer / profiler
    // Timer                           mTimer;
    // Timer                           mTimerRender;

    using WindowNameLookup_t = std::unordered_map<HWND, std::string>;

    WindowNameLookup_t              mWinNameLookup;


private:
    void                            InitializeInput();
    void                            InitializeEngineSettings(const FStartupParameters& Params);
    void                            InitializeWindows(const FStartupParameters& Params);
    // void                            InitializeHDRProfiles();
    // void                            InitializeEnvironmentMaps();
    // void                            InitializeScenes();
    // void                            InitializeUI(HWND hwnd);
    // void                            InitializeEngineThreads();

    void                            RegisterWindowForInput(const std::unique_ptr<Window>& pWnd);
    // void                            UnregisterWindowForInput(const std::unique_ptr<Window>& pWnd);

    // uint64                          mNumSimulationTicks;

    //
    // Events
    //
    void                            MainThread_HandleEvents();
    void                            HandleWindowTransitions(std::unique_ptr<Window>& pWin, const FWindowSettings& settings);
    void                            SetMouseCaptureForWindow(HWND hwnd, bool bCaptureMouse, bool bReleaseAtCapturedPosition);

    //
    // HELPER
    //
    std::unique_ptr<Window>& GetWindow(HWND hwnd);
    const std::unique_ptr<Window>& GetWindow(HWND hwnd) const;
    const FWindowSettings& GetWindowSettings(HWND hwnd) const;
    FWindowSettings& GetWindowSettings(HWND hwnd);

};

struct FWindowDesc
{
    int width{ -1 };
    int height{ -1 };
    HINSTANCE hInst{ NULL };
    pfnWndProc_t pfnWndProc{ nullptr }; // ウィンドウプロシージャ
    IWindowOwner* pWndOwner{ nullptr }; // ウィンドウイベント
    bool bFullscreen{ false };
    int preferredDisplay{ 0 };
    int iShowCmd;
    std::string windowName;

    using Registrar_t = Engine; // エンジンクラスのエイリアス
    void (Registrar_t::* pfnRegisterWindowName)(HWND hwnd, const std::string& WindowName); // 関数ポインタ
    Registrar_t* pRegistrar;
};
