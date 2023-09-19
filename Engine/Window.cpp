#include "Engine.h"
#include "Window.h"
#include "SwapChain.h"

// ウィンドウをディス例の中心に移動させる関数
static RECT CenterScreen(const RECT& screenRect, const RECT& wndRect)
{
    RECT centered = {};

    const int szWndX = wndRect.right - wndRect.left;
    const int szWndY = wndRect.bottom - wndRect.top;
    const int offsetX = (screenRect.right - screenRect.left - szWndX) / 2;
    const int offsetY = (screenRect.bottom - screenRect.top - szWndY) / 2;

    centered.left = screenRect.left + offsetX;
    centered.right = centered.left + szWndX;
    centered.top = screenRect.top + offsetY;
    centered.bottom = centered.top + szWndY;

    return centered;
}

// 設定したディスプレイを取得してそのディスプレイのRECTを取得する
// その後、CenterScreenを適応し、RECTを返す
static RECT GetScreenRectOnPreferredDisplay(const RECT& preferredRect, int PreferredDisplayIndex, bool* pbMonitorFound)
{
    // handle preferred display
    struct MonitorEnumCallbackParams
    {
        int PreferredMonitorIndex = 0;
        const RECT* pRectOriginal = nullptr;
        RECT* pRectNew = nullptr;
        RECT RectDefault;
    };

    // Default RECT
    RECT preferredScreenRect = { CW_USEDEFAULT , CW_USEDEFAULT , CW_USEDEFAULT , CW_USEDEFAULT };

    // モニターパラメータ
    MonitorEnumCallbackParams p = {};
    p.PreferredMonitorIndex = PreferredDisplayIndex; // 設定したモニター番号
    p.pRectOriginal = &preferredRect; // 設定前のRECT
    p.pRectNew = &preferredScreenRect; // 新しく設定したRECT

    // 列挙されたモニターのインデックス番号
    auto fnCallbackMonitorEnum = [](HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) -> BOOL
    {
        // return Flag
        BOOL b = TRUE;
        // ユーザーパラメータのキャスト
        MonitorEnumCallbackParams* pParam = (MonitorEnumCallbackParams*)Arg4;

        // モニター情報の取得
        MONITORINFOEXA monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFOEXA);
        GetMonitorInfoA(Arg1, &monitorInfo);

        CHAR* test = monitorInfo.szDevice;
        std::string monitorName(monitorInfo.szDevice); // モニター例  "///./DISPLAY1"
        monitorName = monitorName.substr(monitorName.size() - 1);
        std::string strMonitorIndex = monitorName.substr(monitorName.size() - 1); // strMonitorIndex is "1" for "///./DISPLAY1"
        const int monitorIndex = std::atoi(strMonitorIndex.c_str()) - 1;          // monitorIndex    is  0  for "///./DISPLAY1"

        // モニターインデックスのチェック
        if (monitorIndex == pParam->PreferredMonitorIndex)
        {
            *pParam->pRectNew = *Arg3; // モニターのRECTを設定
        }
        if (monitorIndex == 0)
        {
            pParam->RectDefault = *Arg3; // モニターインデックスが0の場合、デフォルトに設定
        }
        return b;
    };

    EnumDisplayMonitors(NULL, NULL, fnCallbackMonitorEnum, (LPARAM)&p);

    // RECTがデフォルト設定の場合、見つからなかったとする
    const bool bPreferredDisplayNotFound =
        (preferredScreenRect.right == preferredScreenRect.left
            && preferredScreenRect.left == preferredScreenRect.top
            && preferredScreenRect.top == preferredScreenRect.bottom)
        && (preferredScreenRect.right == CW_USEDEFAULT);

    *pbMonitorFound = !bPreferredDisplayNotFound;

    return bPreferredDisplayNotFound ? p.RectDefault : CenterScreen(preferredScreenRect, preferredRect);
}

// コンストラクタ
Window::Window(const std::string& title, FWindowDesc& initParams)
    : IWindow(initParams.pWndOwner)
    , width_(initParams.width)
    , height_(initParams.height)
    , isFullscreen_(initParams.bFullscreen)
{
    // ウィンドウスタイル
    UINT FlagWindowStyle = WS_OVERLAPPEDWINDOW;

    // RECTの仮設定
    ::RECT rect;
    ::SetRect(&rect, 0, 0, width_, height_);
    ::AdjustWindowRect(&rect, FlagWindowStyle, FALSE);

    // 親ハンドル
    HWND hwnd_parent = NULL;

    // ウィンドウクラスの作成・設定
    windowClass_.reset(new WindowClass("WindowClass", initParams.hInst, initParams.pfnWndProc));

    // ディスプレイのフラグ
    bool bPreferredDisplayFound = false;
    // ディスプレイに対するRECTの設定
    RECT preferredScreenRect = GetScreenRectOnPreferredDisplay(rect, initParams.preferredDisplay, &bPreferredDisplayFound);

    // フルスクリーン時のウィンドウサイズを設定する
    this->FSwidth_ = preferredScreenRect.right - preferredScreenRect.left;
    this->FSheight_ = preferredScreenRect.bottom - preferredScreenRect.top;

    // ウィンドウ作成
    hwnd_ = ::CreateWindowExA(
        NULL,
        windowClass_->GetName().c_str(), // window class name
        title.c_str(), // window title
        FlagWindowStyle,
        bPreferredDisplayFound ? preferredScreenRect.left : CW_USEDEFAULT, // position x
        bPreferredDisplayFound ? preferredScreenRect.top : CW_USEDEFAULT, // position y
        rect.right - rect.left, // wdith size
        rect.bottom - rect.top, // height size
        hwnd_parent,
        NULL,
        initParams.hInst, 
        NULL
    );

    // エンジンのインスタンス、ウィンドウ名を取得する関数が設定されていた場合、登録する
    if (initParams.pRegistrar && initParams.pfnRegisterWindowName)
    {
        (initParams.pRegistrar->*initParams.pfnRegisterWindowName)(hwnd_, initParams.windowName);
    }

    // ウィンドウスタイルの保存
    windowStyle_ = FlagWindowStyle;

    // ウィンドウの表示
    ::ShowWindow(hwnd_, initParams.iShowCmd);

    // ウィンドウハンドルにウィンドウクラスを設定
    ::SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR> (this));

    if (!bPreferredDisplayFound)
    {
        // TODO: LOG
    }
}

// デコンストラクタ
IWindow::~IWindow() {}

bool IWindow::IsClosed()     const { return IsClosedImpl(); }
bool IWindow::IsFullscreen() const { return IsFullscreenImpl(); }
bool IWindow::IsMouseCaptured() const { return IsMouseCapturedImpl(); }

HWND Window::GetHWND() const
{
    return hwnd_;
}

void Window::Show()
{
    ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd_);
}

void Window::Minimize()
{
    ::ShowWindow(hwnd_, SW_MINIMIZE);
}

void Window::Close()
{
    this->isClosed_ = true;
    ::ShowWindow(hwnd_, FALSE);
    ::DestroyWindow(hwnd_);
}

void Window::ToggleWindowedFullscreen(SwapChain* pSwapChain)
{
    // フルスクリーンモードの切り替え ( フルスクリーンならウィンドウへ、ウィンドウならフルスクリーンへ )

    // フルスクリーンを終了
    if (isFullscreen_)
    {
        // 新しいウィンドウスタイルに設定 ( WS_OVERLAPPEDWINDOW : ウィンドウモード)
        SetWindowLong(hwnd_, GWL_STYLE, windowStyle_);

        // 新しいウィンドウスタイルの有効化
        SetWindowPos(
			hwnd_, // ウィンドウハンドル
            HWND_NOTOPMOST, // ウィンドウのZオーダー ( 最上位以外のすべての上 ) 
			rect_.left, // ウィンドウのX座標
			rect_.top, // ウィンドウのY座標
			rect_.right - rect_.left, // ウィンドウの幅
			rect_.bottom - rect_.top, // ウィンドウの高さ
			SWP_FRAMECHANGED | SWP_NOACTIVATE //スタイル変更 | アクティブ化しない
		);

        // ウィンドウの表示
        ShowWindow(hwnd_, SW_NORMAL);
    }
    else // フルスクリーンを開始
    {
        // RECTをrect_に保存 ( ウィンドウモードに戻すときに使用する )
        GetWindowRect(hwnd_, &rect_);

        // ウィンドウスタイルの変更 ( ウィンドウスタイルはそのままに、タイトルバー、最大化ボタン、メニューボタン、システムメニュー、サイズ変更フレームを削除 ) 
        // ボーダーレスウィンドウへ
        SetWindowLong(hwnd_, GWL_STYLE, windowStyle_ & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

        RECT fullscreenWindowRect;

        // SwapchainからRECTを取得する
        if (pSwapChain)
        {
            // Adapter出力を保存
            IDXGIOutput* pOutput = nullptr;
            // swapchainクラスのswapchainの実態の関数
            // Adapter出力を取得
            pSwapChain->mpSwapChain->GetContainingOutput(&pOutput);
            DXGI_OUTPUT_DESC Desc;
            pOutput->GetDesc(&Desc);
            fullscreenWindowRect = Desc.DesktopCoordinates; // RECTを取得
            pOutput->Release();
        }
        else
        {
            // プライマリディスプレイのRECTを取得
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            // 現在のディスプレイの設定を取得
            EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

            fullscreenWindowRect = {
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
                devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
            };
        }

        SetWindowPos(
            hwnd_,
            HWND_TOPMOST,
            fullscreenWindowRect.left,
            fullscreenWindowRect.top,
            fullscreenWindowRect.right,
            fullscreenWindowRect.bottom,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hwnd_, SW_MAXIMIZE);

        // save fullscreen width & height 
        this->FSwidth_ = fullscreenWindowRect.right - fullscreenWindowRect.left;
        this->FSheight_ = fullscreenWindowRect.bottom - fullscreenWindowRect.top;


    }

    isFullscreen_ = !isFullscreen_;
}

void Window::SetMouseCapture(bool bCpature)
{
    isMouseCaptured_ = bCpature;
    if (bCpature)
    {
        // ウィンドウ領域を取得
        RECT rcClip;
        GetClientRect(hwnd_, &rcClip);

        // カーソルをウィンドウ内に制限
        constexpr int PX_OFFSET = 15;
        constexpr int PX_WND_TITLE_OFFSET = 30;
        rcClip.left += PX_OFFSET;
        rcClip.right -= PX_OFFSET;
        rcClip.top += PX_OFFSET + PX_WND_TITLE_OFFSET;
        rcClip.bottom -= PX_OFFSET;

        // 全画面時にはマウスカーソルの操作はゲーム側に移譲
        int hr = ShowCursor(FALSE); // カーソルの非表示
        while (hr >= 0) hr = ShowCursor(FALSE);
        
        ClipCursor(&rcClip); // カーソルを閉じ込める
        SetForegroundWindow(hwnd_); // ウィンドウを最前面に
        SetFocus(hwnd_);
    }
    else
    {
        ClipCursor(nullptr);
        while (ShowCursor(TRUE) <= 0);
        SetForegroundWindow(NULL);
    }
}

WindowClass::WindowClass(const std::string& name, HINSTANCE hInst, ::WNDPROC procedure)
    :name_(name)
{
    ::WNDCLASSEXA wc = {};

    // ウィンドウクラスの登録
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS; // 上下の再描画、ダブルクリックの許可
    wc.lpfnWndProc = procedure; // ウィンドウプロシージャs
    wc.cbClsExtra = 0; // ウィンドウクラスの拡張メモリ
    wc.cbWndExtra = 0; // ウィンドウインスタンスの拡張メモリ
    wc.hInstance = hInst;
    wc.hIcon = ::LoadIcon(hInst, IDI_APPLICATION); // ウィンドウアイコンの設定 ( デフォルト )
    // アイコンの読み込みに失敗した場合
    if (wc.hIcon == NULL)
    {
        DWORD dw = GetLastError();
        // TODO : エラーログ出力
    }
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // カーソルの設定 ( デフォルト )
    wc.hbrBackground = NULL; // 塗りつぶしなし
    wc.lpszMenuName = NULL;  // メニューなし
    wc.lpszClassName = name_.c_str();
    wc.cbSize = sizeof(::WNDCLASSEXA);

    ::RegisterClassExA(&wc);
}

const std::string& WindowClass::GetName() const
{
	return name_;
}

WindowClass::~WindowClass()
{
    // ウィンドウクラスの登録解除 
    // ウィンドウクラス名, インスタンスハンドル
    ::UnregisterClassA(name_.c_str(), (HINSTANCE)::GetModuleHandle(nullptr)); // 呼び出し元のモジュールハンドルを取得
}