#include "Engine.h"
#include "Input.h"

#include <windowsx.h>

constexpr int MIN_WINDOW_SIZE = 128;


#define LOG_WINDOW_MESSAGE_EVENTS 0
#define LOG_RAW_INPUT             0
#define LOG_CALLBACKS             1
// static void LogWndMsg(UINT msg, HWND hwnd);

// =================================================================================================
// WINDOW PROCEDURE
// =================================================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// LogWndMsg(uMsg, hwnd);
	// ユーザー指定のウィンドウクラスを取得
	IWindow* pWindow = reinterpret_cast<IWindow*> (::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	// ユーザーウィンドウクラスが取得できなかった場合
	if (!pWindow)
	{
#if 0
		Log::Warning("WndProc::pWindow=nullptr");
#endif
		// デフォルトのウィンドウプロシージャに処理を任せる
		return ::DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	
	// HANDLE EVENT
	switch (uMsg)
	{
		//
		// WINDOW
		// 

		// 基本的に親ウィンドウの関数を呼び出す
	case WM_SIZE: if (pWindow->pOwner) pWindow->pOwner->OnWindowResize(hwnd); return 0;
	case WM_GETMINMAXINFO: // 規定の最小・最高ウィンドウサイズを取得する
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = MIN_WINDOW_SIZE;
		lpMMI->ptMinTrackSize.y = MIN_WINDOW_SIZE;
		break;
	}
	case WM_PAINT: // ウィンドウの塗りつぶし ( 初期色 )
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_SETFOCUS: if (pWindow->pOwner) pWindow->pOwner->OnWindowFocus(hwnd); return 0;
	case WM_KILLFOCUS: if (pWindow->pOwner) pWindow->pOwner->OnWindowLoseFocus(hwnd); return 0;
	case WM_CLOSE: if (pWindow->pOwner) pWindow->pOwner->OnWindowClose(hwnd); return 0;
	case WM_DESTROY: return 0;
	case WM_ACTIVATE:
	{
		if (pWindow->pOwner)
		{
			// **** **** **** **** **** **** **** **** = wParam <-- (32-bits)
			// 0000 0000 0000 0000 1111 1111 1111 1111 = 0xFFFF <-- LOWORD(Wparam)
			UINT wparam_hi = HIWORD(wParam);
			UINT wparam_low = LOWORD(wParam);

			// wParam
			// LWORD : ウィンドウがアクティブかどうか
			// HWORD : 最小化しているかどうか、0以外だと最小化している
			const bool bWindowInactive = (wparam_low == WA_INACTIVE);
			const bool bWindowActivation = (wparam_low == WA_ACTIVE) || (wparam_low == WA_CLICKACTIVE);

			// lParam
			// アクティブ化されるウィンドウのハンドル
			HWND hwnd = reinterpret_cast<HWND>(lParam);
			// bWindowInactiveがINACTIVEの場合、ACTIVE化
			// bWindowInactiveがACTIVEの場合、DEACTIVE化
			if (bWindowInactive)
				pWindow->pOwner->OnWindowActivate(hwnd);
			else if (hwnd != NULL)
				pWindow->pOwner->OnWindowDeactivate(hwnd);
		}
		return 0;
	}
	case WM_MOVE:
	{
		if (pWindow->pOwner)
		{
			int xPos = (int)(short)LOWORD(lParam);
			int yPos = (int)(short)HIWORD(lParam);
			pWindow->pOwner->OnWindowMove(hwnd, xPos, yPos);
		}
		return 0;
	}
	case WM_DISPLAYCHANGE: // ディスプレイ解像度変更時
	{
		if (pWindow->pOwner)
		{
			int ImageDepthBitsPerPixel = (int)wParam;
			int ScreenResolutionX = LOWORD(lParam);
			int ScreenResolutionY = HIWORD(lParam);
			pWindow->pOwner->OnDisplayChange(hwnd, ImageDepthBitsPerPixel, ScreenResolutionX, ScreenResolutionY);
		}
		return 0;
	}
	//
	// KEYBOARD
	//
	case WM_KEYDOWN: if (pWindow->pOwner) pWindow->pOwner->OnKeyDown(hwnd, wParam); return 0;
	case WM_KEYUP:   if (pWindow->pOwner) pWindow->pOwner->OnKeyUp(hwnd, wParam);   return 0;
		// F10かALTをチェックする
	case WM_SYSKEYDOWN:
		// Check ALT+ENTER to FullScreen
		// ( Enter ) + ( ALT ) をチェック
		if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
		{
			if (pWindow->pOwner)
			{
				pWindow->pOwner->OnToggleFullscreen(hwnd);
				return 0;
			}
		} break;
		// メニュー展開、非対応キーが押された時
	case WM_MENUCHAR: return MNC_CLOSE << 16;

		// 
		// MOUSE
		//
	case WM_MBUTTONDOWN: // wParam encodes which mouse key is pressed, unlike *BUTTONUP events
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (pWindow->pOwner) pWindow->pOwner->OnMouseButtonDown(hwnd, wParam, false);
		return 0;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		if (pWindow->pOwner)
		{
			pWindow->pOwner->OnMouseButtonDown(hwnd, wParam, true);
			pWindow->pOwner->OnMouseButtonDown(hwnd, wParam, true);
		}
		return 0;
	// MouseUpイベントは、wParamが0のため個別にイベントを書く
	case WM_MBUTTONUP: if (pWindow->pOwner) pWindow->pOwner->OnMouseButtonUp(hwnd, MK_MBUTTON); return 0;
	case WM_RBUTTONUP: if (pWindow->pOwner) pWindow->pOwner->OnMouseButtonUp(hwnd, MK_RBUTTON); return 0;
	case WM_LBUTTONUP: if (pWindow->pOwner) pWindow->pOwner->OnMouseButtonUp(hwnd, MK_LBUTTON); return 0;

#if ENABLE_RAW_INPUT // https://msdn.microsoft.com/en-us/library/windows/desktop/ee418864.aspx
	case WM_INPUT: if (pWindow->pOwner) pWindow->pOwner->OnMouseInput(hwnd, lParam); return 0;
#endif
	}

	return ::DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

// =================================================================================================
// WINDOW EVENTS
// =================================================================================================
// 必要な設定をして、Eventを作成、キューに追加する

void Engine::OnWindowResize(HWND hwnd)
{
	RECT clientRect = {};
	GetClientRect(hwnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	
	mEventQueue_WinToE_Renderer.AddItem(std::make_unique<WindowResizeEvent>(width, height, hwnd));
	mEventQueue_EnToWin_Main.AddItem(std::make_unique<WindowResizeEvent>(width, height, hwnd));
}

void Engine::OnToggleFullscreen(HWND hwnd)
{
	mEventQueue_WinToE_Renderer.AddItem(std::make_unique<ToggleFullscreenEvent>(hwnd));
	mEventQueue_EnToWin_Main.AddItem(std::make_unique<ToggleFullscreenEvent>(hwnd));
}

void Engine::OnWindowActivate(HWND hwnd)
{
	if (IsWindowRegistered(hwnd))
	{
#if LOG_CALLBACKS
		Log::Warning("OnWindowActivate<%0x, %s>", hwnd, GetWindowName(hwnd).c_str());
#endif
	}
}

void Engine::OnWindowDeactivate(HWND hwnd)
{
	if (IsWindowRegistered(hwnd))
	{ 
#if LOG_CALLBACKS
		Log::Warning("OnWindowDeactivate<%0x, %s> ", hwnd, GetWindowName(hwnd).c_str());
#endif
	}
}

void Engine::OnWindowMove(HWND hwnd, int xPos, int yPos)
{
#if LOG_CALLBACKS
	Log::Warning("OnWindowMove<%0x, %s>: (%d, %d)", hwnd, GetWindowName(hwnd).c_str(), xPos, yPos);
#endif
}

void Engine::OnDisplayChange(HWND hwnd, int ImageDepthBitsPerPixel, int ScreenResolutionX, int ScreenResolutionY)
{
}

void Engine::OnWindowMinimize(HWND hwnd)
{
}

void Engine::OnWindowCreate(HWND hwnd)
{
#if LOG_CALLBACKS
	Log::Info("OnWindowCreate<%0x, %s> ", hwnd, GetWindowName(hwnd).c_str());
#endif
}

// レンダリングパスの停止を確認後終了する
void Engine::OnWindowClose(HWND hwnd)
{
	std::shared_ptr<WindowCloseEvent> ptr = std::make_shared<WindowCloseEvent>(hwnd);
	mEventQueue_WinToE_Renderer.AddItem(ptr);

	// レンダリングパスの停止を確認
	//ptr->Signal_WindowDependentResourcesDestroyed.Wait();
	if (hwnd == mpWinMain->GetHWND())
	{
		PostQuitMessage(0); // 必ずMainThreadで送る
	}
	GetWindow(hwnd)->Close();
}

void Engine::OnWindowFocus(HWND hwnd)
{
	const bool bMainWindowFocused = hwnd == mpWinMain->GetHWND();

#if LOG_CALLBACKS
	Log::Warning("OnWindowFocus<%x, %s>", hwnd, this->GetWindowName(hwnd).c_str());
#endif
}

void Engine::OnWindowLoseFocus(HWND hwnd)
{
#if LOG_CALLBACKS
	Log::Warning("OnWindowLoseFocus<%x, %s>", hwnd, this->GetWindowName(hwnd).c_str());
#endif

	// Mainウィンドウがフォーカスを失ったらマウスキャプチャーを終了する
	if (hwnd == mpWinMain->GetHWND() && mpWinMain->IsMouseCaptured())
		this->SetMouseCaptureForWindow(mpWinMain->GetHWND(), false, true);
}

// ===================================================================================================================================
// KEYBOARD EVENTS
// ===================================================================================================================================
// Due to multi-threading, this thread will record the events and 
// Update Thread will process the queue at the beginning of an update loop
void Engine::OnKeyDown(HWND hwnd, WPARAM wParam)
{
	constexpr bool bIsMouseEvent = false;
	mEventQueue_WinToE_Update.AddItem(std::make_unique<KeyDownEvent>(hwnd, wParam, bIsMouseEvent));
}

void Engine::OnKeyUp(HWND hwnd, WPARAM wParam)
{
	constexpr bool bIsMouseEvent = false;
	mEventQueue_WinToE_Update.AddItem(std::make_unique<KeyUpEvent>(hwnd, wParam, bIsMouseEvent));
}


// ===================================================================================================================================
// MOUSE EVENTS
// ===================================================================================================================================
// Due to multi-threading, this thread will record the events and 
// Update Thread will process the queue at the beginning of an update loop
void Engine::OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick)
{
	constexpr bool bIsMouseEvent = true;
	mEventQueue_WinToE_Update.AddItem(std::make_unique<KeyDownEvent>(hwnd, wParam, bIsMouseEvent, bIsDoubleClick));
}

void Engine::OnMouseButtonUp(HWND hwnd, WPARAM wParam)
{
	constexpr bool bIsMouseEvent = true;
	mEventQueue_WinToE_Update.AddItem(std::make_unique<KeyUpEvent>(hwnd, wParam, bIsMouseEvent));
}

void Engine::OnMouseScroll(HWND hwnd, short scroll)
{
	mEventQueue_WinToE_Update.AddItem(std::make_unique<MouseScrollEvent>(hwnd, scroll));
}


void Engine::OnMouseMove(HWND hwnd, long x, long y)
{
	Log::Info("MouseMove : (%ld, %ld)", x, y);
	mEventQueue_WinToE_Update.AddItem(std::make_unique<MouseMoveEvent>(hwnd, x, y));
}


void Engine::OnMouseInput(HWND hwnd, LPARAM lParam)
{
	MouseInputEventData data = {};
	const bool bMouseInputEvent = Input::ReadRawInput_Mouse(lParam, &data);

	if (bMouseInputEvent)
	{
		mEventQueue_WinToE_Update.AddItem(std::make_shared<MouseInputEvent>(data, hwnd));
	}
}

// ===================================================================================================================================
// MISC
// ===================================================================================================================================

static void LogWndMsg(UINT uMsg, HWND hwnd)
{
#if LOG_WINDOW_MESSAGE_EVENTS
#define HANDLE_CASE(EVENT)    case EVENT: Log::Info(#EVENT"\t(0x%04x)\t\t<hwnd=0x%x>", EVENT, hwnd); break
	switch (uMsg)
	{
		// https://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm
		HANDLE_CASE(WM_MOUSELAST);
		HANDLE_CASE(WM_ACTIVATE);
		HANDLE_CASE(WM_ACTIVATEAPP);
		HANDLE_CASE(WM_AFXFIRST);
		HANDLE_CASE(WM_AFXLAST);
		HANDLE_CASE(WM_APP);
		HANDLE_CASE(WM_APPCOMMAND);
		HANDLE_CASE(WM_ASKCBFORMATNAME);
		HANDLE_CASE(WM_CANCELJOURNAL);
		HANDLE_CASE(WM_CANCELMODE);
		HANDLE_CASE(WM_CAPTURECHANGED);
		HANDLE_CASE(WM_CHANGECBCHAIN);
		HANDLE_CASE(WM_CHANGEUISTATE);
		HANDLE_CASE(WM_CHAR);
		HANDLE_CASE(WM_CHARTOITEM);
		HANDLE_CASE(WM_CHILDACTIVATE);
		HANDLE_CASE(WM_CLEAR);
		HANDLE_CASE(WM_CLOSE);
		HANDLE_CASE(WM_COMMAND);
		HANDLE_CASE(WM_COMMNOTIFY);
		HANDLE_CASE(WM_COMPACTING);
		HANDLE_CASE(WM_COMPAREITEM);
		HANDLE_CASE(WM_CONTEXTMENU);
		HANDLE_CASE(WM_COPY);
		HANDLE_CASE(WM_COPYDATA);
		HANDLE_CASE(WM_CREATE);
		HANDLE_CASE(WM_CTLCOLORBTN);
		HANDLE_CASE(WM_CTLCOLORDLG);
		HANDLE_CASE(WM_CTLCOLOREDIT);
		HANDLE_CASE(WM_CTLCOLORLISTBOX);
		HANDLE_CASE(WM_CTLCOLORMSGBOX);
		HANDLE_CASE(WM_CTLCOLORSCROLLBAR);
		HANDLE_CASE(WM_CTLCOLORSTATIC);
		HANDLE_CASE(WM_CUT);
		HANDLE_CASE(WM_DEADCHAR);
		HANDLE_CASE(WM_DELETEITEM);
		HANDLE_CASE(WM_DESTROY);
		HANDLE_CASE(WM_DESTROYCLIPBOARD);
		HANDLE_CASE(WM_DEVICECHANGE);
		HANDLE_CASE(WM_DEVMODECHANGE);
		HANDLE_CASE(WM_DISPLAYCHANGE);
		HANDLE_CASE(WM_DRAWCLIPBOARD);
		HANDLE_CASE(WM_DRAWITEM);
		HANDLE_CASE(WM_DROPFILES);
		HANDLE_CASE(WM_ENABLE);
		HANDLE_CASE(WM_ENDSESSION);
		HANDLE_CASE(WM_ENTERIDLE);
		HANDLE_CASE(WM_ENTERMENULOOP);
		HANDLE_CASE(WM_ENTERSIZEMOVE);
		HANDLE_CASE(WM_ERASEBKGND);
		HANDLE_CASE(WM_EXITMENULOOP);
		HANDLE_CASE(WM_EXITSIZEMOVE);
		HANDLE_CASE(WM_FONTCHANGE);
		HANDLE_CASE(WM_GETDLGCODE);
		HANDLE_CASE(WM_GETFONT);
		HANDLE_CASE(WM_GETHOTKEY);
		HANDLE_CASE(WM_GETICON);
		HANDLE_CASE(WM_GETMINMAXINFO);
		HANDLE_CASE(WM_GETOBJECT);
		HANDLE_CASE(WM_GETTEXT);
		HANDLE_CASE(WM_GETTEXTLENGTH);
		HANDLE_CASE(WM_HANDHELDFIRST);
		HANDLE_CASE(WM_HANDHELDLAST);
		HANDLE_CASE(WM_HELP);
		HANDLE_CASE(WM_HOTKEY);
		HANDLE_CASE(WM_HSCROLL);
		HANDLE_CASE(WM_HSCROLLCLIPBOARD);
		HANDLE_CASE(WM_ICONERASEBKGND);
		HANDLE_CASE(WM_IME_CHAR);
		HANDLE_CASE(WM_IME_COMPOSITION);
		HANDLE_CASE(WM_IME_COMPOSITIONFULL);
		HANDLE_CASE(WM_IME_CONTROL);
		HANDLE_CASE(WM_IME_ENDCOMPOSITION);
		HANDLE_CASE(WM_IME_KEYDOWN);
		HANDLE_CASE(WM_IME_KEYUP);
		HANDLE_CASE(WM_IME_NOTIFY);
		HANDLE_CASE(WM_IME_REQUEST);
		HANDLE_CASE(WM_IME_SELECT);
		HANDLE_CASE(WM_IME_SETCONTEXT);
		HANDLE_CASE(WM_IME_STARTCOMPOSITION);
		HANDLE_CASE(WM_INITDIALOG);
		HANDLE_CASE(WM_INITMENU);
		HANDLE_CASE(WM_INITMENUPOPUP);
		HANDLE_CASE(WM_INPUT);
		HANDLE_CASE(WM_INPUTLANGCHANGE);
		HANDLE_CASE(WM_INPUTLANGCHANGEREQUEST);
		HANDLE_CASE(WM_KEYDOWN);
		HANDLE_CASE(WM_KEYLAST);
		HANDLE_CASE(WM_KEYUP);
		HANDLE_CASE(WM_KILLFOCUS);
		HANDLE_CASE(WM_LBUTTONDBLCLK);
		HANDLE_CASE(WM_LBUTTONDOWN);
		HANDLE_CASE(WM_LBUTTONUP);
		HANDLE_CASE(WM_MBUTTONDBLCLK);
		HANDLE_CASE(WM_MBUTTONDOWN);
		HANDLE_CASE(WM_MBUTTONUP);
		HANDLE_CASE(WM_MDIACTIVATE);
		HANDLE_CASE(WM_MDICASCADE);
		HANDLE_CASE(WM_MDICREATE);
		HANDLE_CASE(WM_MDIDESTROY);
		HANDLE_CASE(WM_MDIGETACTIVE);
		HANDLE_CASE(WM_MDIICONARRANGE);
		HANDLE_CASE(WM_MDIMAXIMIZE);
		HANDLE_CASE(WM_MDINEXT);
		HANDLE_CASE(WM_MDIREFRESHMENU);
		HANDLE_CASE(WM_MDIRESTORE);
		HANDLE_CASE(WM_MDISETMENU);
		HANDLE_CASE(WM_MDITILE);
		HANDLE_CASE(WM_MEASUREITEM);
		HANDLE_CASE(WM_MENUCHAR);
		HANDLE_CASE(WM_MENUCOMMAND);
		HANDLE_CASE(WM_MENUDRAG);
		HANDLE_CASE(WM_MENUGETOBJECT);
		HANDLE_CASE(WM_MENURBUTTONUP);
		HANDLE_CASE(WM_MENUSELECT);
		HANDLE_CASE(WM_MOUSEACTIVATE);
		HANDLE_CASE(WM_MOUSEFIRST);
		HANDLE_CASE(WM_MOUSEHOVER);
		HANDLE_CASE(WM_MOUSELEAVE);
		HANDLE_CASE(WM_MOUSEWHEEL);
		HANDLE_CASE(WM_MOVE);
		HANDLE_CASE(WM_MOVING);
		HANDLE_CASE(WM_NCACTIVATE);
		HANDLE_CASE(WM_NCCALCSIZE);
		HANDLE_CASE(WM_NCCREATE);
		HANDLE_CASE(WM_NCDESTROY);
		HANDLE_CASE(WM_NCHITTEST);
		HANDLE_CASE(WM_NCLBUTTONDBLCLK);
		HANDLE_CASE(WM_NCLBUTTONDOWN);
		HANDLE_CASE(WM_NCLBUTTONUP);
		HANDLE_CASE(WM_NCMBUTTONDBLCLK);
		HANDLE_CASE(WM_NCMBUTTONDOWN);
		HANDLE_CASE(WM_NCMBUTTONUP);
		HANDLE_CASE(WM_NCMOUSEHOVER);
		HANDLE_CASE(WM_NCMOUSELEAVE);
		HANDLE_CASE(WM_NCMOUSEMOVE);
		HANDLE_CASE(WM_NCPAINT);
		HANDLE_CASE(WM_NCRBUTTONDBLCLK);
		HANDLE_CASE(WM_NCRBUTTONDOWN);
		HANDLE_CASE(WM_NCRBUTTONUP);
		HANDLE_CASE(WM_NCXBUTTONDBLCLK);
		HANDLE_CASE(WM_NCXBUTTONDOWN);
		HANDLE_CASE(WM_NCXBUTTONUP);
		HANDLE_CASE(WM_NEXTDLGCTL);
		HANDLE_CASE(WM_NEXTMENU);
		HANDLE_CASE(WM_NOTIFY);
		HANDLE_CASE(WM_NOTIFYFORMAT);
		HANDLE_CASE(WM_NULL);
		HANDLE_CASE(WM_PAINT);
		HANDLE_CASE(WM_PAINTCLIPBOARD);
		HANDLE_CASE(WM_PAINTICON);
		HANDLE_CASE(WM_PALETTECHANGED);
		HANDLE_CASE(WM_PALETTEISCHANGING);
		HANDLE_CASE(WM_PARENTNOTIFY);
		HANDLE_CASE(WM_PASTE);
		HANDLE_CASE(WM_PENWINFIRST);
		HANDLE_CASE(WM_PENWINLAST);
		HANDLE_CASE(WM_POWER);
		HANDLE_CASE(WM_POWERBROADCAST);
		HANDLE_CASE(WM_PRINT);
		HANDLE_CASE(WM_PRINTCLIENT);
		HANDLE_CASE(WM_QUERYDRAGICON);
		HANDLE_CASE(WM_QUERYENDSESSION);
		HANDLE_CASE(WM_QUERYNEWPALETTE);
		HANDLE_CASE(WM_QUERYOPEN);
		HANDLE_CASE(WM_QUERYUISTATE);
		HANDLE_CASE(WM_QUEUESYNC);
		HANDLE_CASE(WM_QUIT);
		HANDLE_CASE(WM_RBUTTONDBLCLK);
		HANDLE_CASE(WM_RBUTTONDOWN);
		HANDLE_CASE(WM_RBUTTONUP);
		HANDLE_CASE(WM_RENDERALLFORMATS);
		HANDLE_CASE(WM_RENDERFORMAT);
		HANDLE_CASE(WM_SETCURSOR);
		HANDLE_CASE(WM_SETFOCUS);
		HANDLE_CASE(WM_SETFONT);
		HANDLE_CASE(WM_SETHOTKEY);
		HANDLE_CASE(WM_SETICON);
		HANDLE_CASE(WM_SETREDRAW);
		HANDLE_CASE(WM_SETTEXT);
		HANDLE_CASE(WM_SETTINGCHANGE);
		HANDLE_CASE(WM_SHOWWINDOW);
		HANDLE_CASE(WM_SIZE);
		HANDLE_CASE(WM_SIZECLIPBOARD);
		HANDLE_CASE(WM_SIZING);
		HANDLE_CASE(WM_SPOOLERSTATUS);
		HANDLE_CASE(WM_STYLECHANGED);
		HANDLE_CASE(WM_STYLECHANGING);
		HANDLE_CASE(WM_SYNCPAINT);
		HANDLE_CASE(WM_SYSCHAR);
		HANDLE_CASE(WM_SYSCOLORCHANGE);
		HANDLE_CASE(WM_SYSCOMMAND);
		HANDLE_CASE(WM_SYSDEADCHAR);
		HANDLE_CASE(WM_SYSKEYDOWN);
		HANDLE_CASE(WM_SYSKEYUP);
		HANDLE_CASE(WM_TABLET_FIRST);
		HANDLE_CASE(WM_TABLET_LAST);
		HANDLE_CASE(WM_TCARD);
		HANDLE_CASE(WM_THEMECHANGED);
		HANDLE_CASE(WM_TIMECHANGE);
		HANDLE_CASE(WM_TIMER);
		HANDLE_CASE(WM_UNDO);
		HANDLE_CASE(WM_UNINITMENUPOPUP);
		HANDLE_CASE(WM_UPDATEUISTATE);
		HANDLE_CASE(WM_USER);
		HANDLE_CASE(WM_USERCHANGED);
		HANDLE_CASE(WM_VKEYTOITEM);
		HANDLE_CASE(WM_VSCROLL);
		HANDLE_CASE(WM_VSCROLLCLIPBOARD);
		HANDLE_CASE(WM_WINDOWPOSCHANGED);
		HANDLE_CASE(WM_WINDOWPOSCHANGING);
		HANDLE_CASE(WM_WTSSESSION_CHANGE);
		HANDLE_CASE(WM_XBUTTONDBLCLK);
		HANDLE_CASE(WM_XBUTTONDOWN);
		HANDLE_CASE(WM_XBUTTONUP);

		// duplicate value events
		///HANDLE_CASE(WM_KEYFIRST);
		///HANDLE_CASE(WM_IME_KEYLAST);
		///HANDLE_CASE(WM_MOUSEMOVE);
		///HANDLE_CASE(WM_UNICHAR);
		///HANDLE_CASE(WM_WININICHANGE);
	default: Log::Warning("LogWndMsg not defined for msg=0x%x", uMsg); break;
	}
#undef HANDLE_CASE
#endif
}

