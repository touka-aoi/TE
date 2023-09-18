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