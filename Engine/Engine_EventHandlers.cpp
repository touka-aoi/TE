#include "Engine.h"
#include "Windows.h"

//---------------------------------------------------------------------
// 
// MAIN THREAD
//
//---------------------------------------------------------------------
void Engine::MainThread_HandleEvents()
{
	// イベントキューにイベントが入っていなかったら終了
	if (mEventQueue_EnToWin_Main.IsEmpty())
		return;

	// イベントキューをスワップしスワップしたものを読み取る
	mEventQueue_EnToWin_Main.SwapBuffers();
	std::queue<EventPtr_t>& q =  mEventQueue_EnToWin_Main.GetBackContainer();

	// イベントを処理する
	std::shared_ptr<IEvent> pEvent = nullptr;
	while(!q.empty())
	{
		pEvent = std::move(q.front());
		q.pop();

		switch (pEvent->mType)
		{
		case MOUSE_CAPTURE_EVENT:
		{
			// マウスキャプチャー設定をオンに
			std::shared_ptr<SetMouseCaptureEvent> p = std::static_pointer_cast<SetMouseCaptureEvent>(pEvent);
			this->SetMouseCaptureForWindow(p->hwnd, p->bCapture, p->bReleaseAtCapturedPosition);
		} break;
		case HANDLE_WINDOW_TRANSITIONS_EVENT:
		{
			// フルスクリーン、ウィンドウモードの切り替え
			auto& pWnd = this->GetWindow(pEvent->hwnd);
			HandleWindowTransitions(pWnd, this->GetWindowSettings(pEvent->hwnd));
		} break;
		case SHOW_WINDOW_EVENT:
		{
			// ウィンドウ表示
			this->GetWindow(pEvent->hwnd)->Show();
		} break;
		}
	}
}

void Engine::HandleWindowTransitions(std::unique_ptr<Window>& pWin, const FWindowSettings& settings)
{
	if (!pWin) return;

	// メインウィンドウであるかチェック
	const bool bHandlingMainWindowTransition = pWin == mpWinMain;

	// デバックウィンドウがメインウィンドウと同じディスプレイでフルスクリーンになるのを防ぐ
	if (mpWinMain->IsFullscreen()
		&& (mSettings.WndMain.PreferredDisplay == mSettings.WndDebug.PreferredDisplay)
		&& settings.IsDisplayModeFullscreen()
		&& !bHandlingMainWindowTransition)
	{
		Log::Warning("Debug window and Main window cannot be Fullscreen on the same display!");
		pWin->SetFullscreen(false);
		return;
	}

	// Borderless fullscreen transitions are handled through Window object
	// Exclusive  fullscreen transitions are handled through the Swapchain
	// ボーダーレスウィンドウの場合はウィンドウオブジェクトが、排他的フルスクリーンの場合はスワップチェーンがハンドリングする
	
	// ボーダーレスウィンドウの場合
	if (settings.DisplayMode == EDisplayMode::BORDERLESS_FULLSCREEN)
	{
		HWND hwnd = pWin->GetHWND();
		// pWin->ToggleWindowedFullscreen(&mRenderer.GetWindowSwapChain(hwnd));

		// メインウィンドウの場合マウスキャプチャーを設定
		if (bHandlingMainWindowTransition)
			SetMouseCaptureForWindow(hwnd, true, true);
	}
}

void Engine::SetMouseCaptureForWindow(HWND hwnd, bool bCaptureMouse, bool bReleaseAtCapturedPosition)
{
	auto& pWin = this->GetWindow(hwnd);

	// ウィンドウのInputStateを取得しようとする
	if (mInputStates.find(hwnd) == mInputStates.end())
	{
		Log::Error("Warning: couldn't find InputState for hwnd=0x%x", hwnd);
	}

	// マウスキャプチャー設定
	pWin->SetMouseCapture(bCaptureMouse);

	// マウスカーソル位置の取得、保存
	if (bCaptureMouse)
	{
		// マウスカーソルの位置を取得し、キャプチャーへ保存する
		GetCursorPos(&this->mMouseCapturePosition);
#if VERBOSE_LOGGING
		Log::Info("Capturing Mouse: Last position=(%d, %d)", this->mMouseCapturePosition.x, this->mMouseCapturePosition.y);
#endif
	}
	else
	{
		// マウスキャプチャー解除
		if (bReleaseAtCapturedPosition)
		{
			// カーソルの復元
			SetCursorPos(this->mMouseCapturePosition.x, this->mMouseCapturePosition.y);
		}
#if VERBOSE_LOGGING
		Log::Info("Releasing Mouse: Setting Position=(%d, %d), bReleaseAtCapturedPosition=%d", this->mMouseCapturePosition.x, this->mMouseCapturePosition.y, bReleaseAtCapturedPosition);
#endif
	}
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// UPDATE THREAD
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "imgui.h"

/**
 * @fn
 * KeyInputをImGUIのインターフェースに与える関数
 * マウスイベントのみをImGUIに渡す
 */
static void UpdateImGui_KeyDown(KeyDownEventData data)
{
    ImGuiIO& io = ImGui::GetIO();
    const auto& key = data.mouse.wparam;
    // マウス操作を保存する
    if (data.mouse.bMouse)
    {
        const Input::EMouseButtons mouseBtn = static_cast<Input::EMouseButtons>(key);
        int btn = 0;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_LEFT  ) btn = 0;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_RIGHT ) btn = 1;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_MIDDLE) btn = 2;
        io.MouseDown[btn] = true;
    }
}

static void UpdateImGui_KeyUp(KeyCode key, bool bIsMouseKey)
{
    ImGuiIO& io = ImGui::GetIO();
    if (bIsMouseKey)
    {
        const Input::EMouseButtons mouseBtn = static_cast<Input::EMouseButtons>(key);
        int btn = 0;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_LEFT) btn = 0;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_RIGHT) btn = 1;
        if (mouseBtn & Input::EMouseButtons::MOUSE_BUTTON_MIDDLE) btn = 2;
        io.MouseDown[btn] = false;
    }
}

static void UpdateImGui_MousePosition(HWND hwnd)
{
    ImGuiIO& io = ImGui::GetIO();
    POINT cursor_point;
    if (GetCursorPos(&cursor_point))
    {
        if (ScreenToClient(hwnd, &cursor_point))
        {
            io.MousePos.x = static_cast<float>(cursor_point.x);
            io.MousePos.y = static_cast<float>(cursor_point.y);
            //Log::Info("io.MousePos.xy = %.2f %.2f", io.MousePos.x, io.MousePos.y);
        }
    }
}

static void UpdateImGui_MousePosition1(long x, long y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = static_cast<float>(x);
    io.MousePos.y = static_cast<float>(y);
}


void Engine::UpdateThread_HandleEvents()
{
    // イベントキューをスワップ
    mEventQueue_WinToE_Update.SwapBuffers();
    std::queue<EventPtr_t>& q = mEventQueue_WinToE_Update.GetBackContainer();

    if (q.empty()) return;

    // イベントを処理する
    std::shared_ptr<IEvent> pEvent = nullptr;
    while (!q.empty())
    {
        pEvent = std::move(q.front());
        q.pop();

        switch (pEvent->mType)
        {
        case KEY_DOWN_EVENT:
        {
            std::shared_ptr<KeyDownEvent> p = std::static_pointer_cast<KeyDownEvent>(pEvent);
            mInputStates.at(p->hwnd).UpdateKeyDown(p->data); // ウィンドウのハンドルのInputを取得し、キー入力を保存する
            // UpdateImGui_KeyDown(p->data); // ImGuiにキー入力を保存する
        } break;

        case KEY_UP_EVENT:
        {
            std::shared_ptr<KeyUpEvent> p = std::static_pointer_cast<KeyUpEvent>(pEvent);
            mInputStates.at(p->hwnd).UpdateKeyUp(p->wparam, p->bMouseEvent); // ウィンドウのハンドルのInputを取得し、キー入力を保存する
            // UpdateImGui_KeyUp(p->wparam, p->bMouseEvent); // ImGuiにキー入力を保存する
        } break;

        case MOUSE_MOVE_EVENT:
        {
            std::shared_ptr<MouseMoveEvent> p = std::static_pointer_cast<MouseMoveEvent>(pEvent);
            mInputStates.at(p->hwnd).UpdateMousePos(p->x, p->y, 0); // ウィンドウのハンドルのInputを取得し、マウス入力を保存する
            // UpdateImGui_MousePosition1(p->x, p->y);
        } break;

        case MOUSE_SCROLL_EVENT: // ScrollはImGUIに返さない
        {
            std::shared_ptr<MouseScrollEvent> p = std::static_pointer_cast<MouseScrollEvent>(pEvent);
            mInputStates.at(p->hwnd).UpdateMousePos(0, 0, p->scroll);
        } break;

        case MOUSE_INPUT_EVENT: // マウスキャプチャー時のイベント
        {
            std::shared_ptr<MouseInputEvent> p = std::static_pointer_cast<MouseInputEvent>(pEvent);

            // アプリケーションの外で起こった場合は無視する
            if (p->data.scrollDelta)
            {
                POINT pt; GetCursorPos(&pt);
                ScreenToClient(p->hwnd, &pt);

                // ウィンドウ外にあるのかをチェックする
                const bool bOutOfWindow = pt.x < 0 || pt.y < 0
                    || pt.x > this->GetWindow(p->hwnd)->GetWidth()
                    || pt.y > this->GetWindow(p->hwnd)->GetHeight();
                if (bOutOfWindow)
                {
                    p->data.scrollDelta = 0;
                }
            }

            mInputStates.at(p->hwnd).UpdateMousePos_Raw(
                    p->data.relativeX
                , p->data.relativeY
                , static_cast<short>(p->data.scrollDelta)
            );

            // ImGuiIO& io = ImGui::GetIO();
            // UpdateImGui_MousePosition(pEvent->hwnd);
            // io.MouseWheel += p->data.scrollDelta;
        } break;
        // case WINDOW_RESIZE_EVENT: UpdateThread_HandleWindowResizeEvent(pEvent); break;
        }
    }
}

void Engine::UpdateThread_HandleWindowResizeEvent(const std::shared_ptr<IEvent>& pEvent)
{
    //std::shared_ptr<WindowResizeEvent> p = std::static_pointer_cast<WindowResizeEvent>(pEvent);

    //const uint uWidth = p->width;
    //const uint uHeight = p->height;

    //// メインウィンドウかどうかチェック
    //if (p->hwnd == mpWinMain->GetHWND())
    //{
    //    SwapChain& Swapchain = mRenderer.GetWindowSwapChain(p->hwnd);
    //    const int NUM_BACK_BUFFER = Swapchain.GetNumBackBuffers();

    //    if ((uWidth | uHeight) != 0 && mpScene)
    //    {
    //        Camera& cam = mpScene->GetActiveCamera();

    //    }
    //}
}
