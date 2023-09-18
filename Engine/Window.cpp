#include "Engine.h"
#include "Window.h"
#include "SwapChain.h"

// �E�B���h�E���f�B�X��̒��S�Ɉړ�������֐�
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

// �ݒ肵���f�B�X�v���C���擾���Ă��̃f�B�X�v���C��RECT���擾����
// ���̌�ACenterScreen��K�����ARECT��Ԃ�
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

    // ���j�^�[�p�����[�^
    MonitorEnumCallbackParams p = {};
    p.PreferredMonitorIndex = PreferredDisplayIndex; // �ݒ肵�����j�^�[�ԍ�
    p.pRectOriginal = &preferredRect; // �ݒ�O��RECT
    p.pRectNew = &preferredScreenRect; // �V�����ݒ肵��RECT

    // �񋓂��ꂽ���j�^�[�̃C���f�b�N�X�ԍ�
    auto fnCallbackMonitorEnum = [](HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) -> BOOL
    {
        // return Flag
        BOOL b = TRUE;
        // ���[�U�[�p�����[�^�̃L���X�g
        MonitorEnumCallbackParams* pParam = (MonitorEnumCallbackParams*)Arg4;

        // ���j�^�[���̎擾
        MONITORINFOEXA monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFOEXA);
        GetMonitorInfoA(Arg1, &monitorInfo);

        CHAR* test = monitorInfo.szDevice;
        std::string monitorName(monitorInfo.szDevice); // ���j�^�[��  "///./DISPLAY1"
        monitorName = monitorName.substr(monitorName.size() - 1);
        std::string strMonitorIndex = monitorName.substr(monitorName.size() - 1); // strMonitorIndex is "1" for "///./DISPLAY1"
        const int monitorIndex = std::atoi(strMonitorIndex.c_str()) - 1;          // monitorIndex    is  0  for "///./DISPLAY1"

        // ���j�^�[�C���f�b�N�X�̃`�F�b�N
        if (monitorIndex == pParam->PreferredMonitorIndex)
        {
            *pParam->pRectNew = *Arg3; // ���j�^�[��RECT��ݒ�
        }
        if (monitorIndex == 0)
        {
            pParam->RectDefault = *Arg3; // ���j�^�[�C���f�b�N�X��0�̏ꍇ�A�f�t�H���g�ɐݒ�
        }
        return b;
    };

    EnumDisplayMonitors(NULL, NULL, fnCallbackMonitorEnum, (LPARAM)&p);

    // RECT���f�t�H���g�ݒ�̏ꍇ�A������Ȃ������Ƃ���
    const bool bPreferredDisplayNotFound =
        (preferredScreenRect.right == preferredScreenRect.left
            && preferredScreenRect.left == preferredScreenRect.top
            && preferredScreenRect.top == preferredScreenRect.bottom)
        && (preferredScreenRect.right == CW_USEDEFAULT);

    *pbMonitorFound = !bPreferredDisplayNotFound;

    return bPreferredDisplayNotFound ? p.RectDefault : CenterScreen(preferredScreenRect, preferredRect);
}

// �R���X�g���N�^
Window::Window(const std::string& title, FWindowDesc& initParams)
    : IWindow(initParams.pWndOwner)
    , width_(initParams.width)
    , height_(initParams.height)
    , isFullscreen_(initParams.bFullscreen)
{
    // �E�B���h�E�X�^�C��
    UINT FlagWindowStyle = WS_OVERLAPPEDWINDOW;

    // RECT�̉��ݒ�
    ::RECT rect;
    ::SetRect(&rect, 0, 0, width_, height_);
    ::AdjustWindowRect(&rect, FlagWindowStyle, FALSE);

    // �e�n���h��
    HWND hwnd_parent = NULL;

    // �E�B���h�E�N���X�̍쐬�E�ݒ�
    windowClass_.reset(new WindowClass("WindowClass", initParams.hInst, initParams.pfnWndProc));

    // �f�B�X�v���C�̃t���O
    bool bPreferredDisplayFound = false;
    // �f�B�X�v���C�ɑ΂���RECT�̐ݒ�
    RECT preferredScreenRect = GetScreenRectOnPreferredDisplay(rect, initParams.preferredDisplay, &bPreferredDisplayFound);

    // �t���X�N���[�����̃E�B���h�E�T�C�Y��ݒ肷��
    this->FSwidth_ = preferredScreenRect.right - preferredScreenRect.left;
    this->FSheight_ = preferredScreenRect.bottom - preferredScreenRect.top;

    // �E�B���h�E�쐬
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

    // �G���W���̃C���X�^���X�A�E�B���h�E�����擾����֐����ݒ肳��Ă����ꍇ�A�o�^����
    if (initParams.pRegistrar && initParams.pfnRegisterWindowName)
    {
        (initParams.pRegistrar->*initParams.pfnRegisterWindowName)(hwnd_, initParams.windowName);
    }

    // �E�B���h�E�X�^�C���̕ۑ�
    windowStyle_ = FlagWindowStyle;

    // �E�B���h�E�̕\��
    ::ShowWindow(hwnd_, initParams.iShowCmd);

    // �E�B���h�E�n���h���ɃE�B���h�E�N���X��ݒ�
    ::SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR> (this));

    if (!bPreferredDisplayFound)
    {
        // TODO: LOG
    }
}

// �f�R���X�g���N�^
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
    // �t���X�N���[�����[�h�̐؂�ւ� ( �t���X�N���[���Ȃ�E�B���h�E�ցA�E�B���h�E�Ȃ�t���X�N���[���� )

    // �t���X�N���[�����I��
    if (isFullscreen_)
    {
        // �V�����E�B���h�E�X�^�C���ɐݒ� ( WS_OVERLAPPEDWINDOW : �E�B���h�E���[�h)
        SetWindowLong(hwnd_, GWL_STYLE, windowStyle_);

        // �V�����E�B���h�E�X�^�C���̗L����
        SetWindowPos(
			hwnd_, // �E�B���h�E�n���h��
            HWND_NOTOPMOST, // �E�B���h�E��Z�I�[�_�[ ( �ŏ�ʈȊO�̂��ׂĂ̏� ) 
			rect_.left, // �E�B���h�E��X���W
			rect_.top, // �E�B���h�E��Y���W
			rect_.right - rect_.left, // �E�B���h�E�̕�
			rect_.bottom - rect_.top, // �E�B���h�E�̍���
			SWP_FRAMECHANGED | SWP_NOACTIVATE //�X�^�C���ύX | �A�N�e�B�u�����Ȃ�
		);

        // �E�B���h�E�̕\��
        ShowWindow(hwnd_, SW_NORMAL);
    }
    else // �t���X�N���[�����J�n
    {
        // RECT��rect_�ɕۑ� ( �E�B���h�E���[�h�ɖ߂��Ƃ��Ɏg�p���� )
        GetWindowRect(hwnd_, &rect_);

        // �E�B���h�E�X�^�C���̕ύX ( �E�B���h�E�X�^�C���͂��̂܂܂ɁA�^�C�g���o�[�A�ő剻�{�^���A���j���[�{�^���A�V�X�e�����j���[�A�T�C�Y�ύX�t���[�����폜 ) 
        // �{�[�_�[���X�E�B���h�E��
        SetWindowLong(hwnd_, GWL_STYLE, windowStyle_ & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

        RECT fullscreenWindowRect;

        // Swapchain����RECT���擾����
        if (pSwapChain)
        {
            // Adapter�o�͂�ۑ�
            IDXGIOutput* pOutput = nullptr;
            // swapchain�N���X��swapchain�̎��Ԃ̊֐�
            // Adapter�o�͂��擾
            pSwapChain->mpSwapChain->GetContainingOutput(&pOutput);
            DXGI_OUTPUT_DESC Desc;
            pOutput->GetDesc(&Desc);
            fullscreenWindowRect = Desc.DesktopCoordinates; // RECT���擾
            pOutput->Release();
        }
        else
        {
            // �v���C�}���f�B�X�v���C��RECT���擾
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            // ���݂̃f�B�X�v���C�̐ݒ���擾
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
        // �E�B���h�E�̈���擾
        RECT rcClip;
        GetClientRect(hwnd_, &rcClip);

        // �J�[�\�����E�B���h�E���ɐ���
        constexpr int PX_OFFSET = 15;
        constexpr int PX_WND_TITLE_OFFSET = 30;
        rcClip.left += PX_OFFSET;
        rcClip.right -= PX_OFFSET;
        rcClip.top += PX_OFFSET + PX_WND_TITLE_OFFSET;
        rcClip.bottom -= PX_OFFSET;

        // �S��ʎ��ɂ̓}�E�X�J�[�\���̑���̓Q�[�����Ɉڏ�
        int hr = ShowCursor(FALSE); // �J�[�\���̔�\��
        while (hr >= 0) hr = ShowCursor(FALSE);
        
        ClipCursor(&rcClip); // �J�[�\��������߂�
        SetForegroundWindow(hwnd_); // �E�B���h�E���őO�ʂ�
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

    // �E�B���h�E�N���X�̓o�^
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS; // �㉺�̍ĕ`��A�_�u���N���b�N�̋���
    wc.lpfnWndProc = procedure; // �E�B���h�E�v���V�[�W��s
    wc.cbClsExtra = 0; // �E�B���h�E�N���X�̊g��������
    wc.cbWndExtra = 0; // �E�B���h�E�C���X�^���X�̊g��������
    wc.hInstance = hInst;
    wc.hIcon = ::LoadIcon(hInst, IDI_APPLICATION); // �E�B���h�E�A�C�R���̐ݒ� ( �f�t�H���g )
    // �A�C�R���̓ǂݍ��݂Ɏ��s�����ꍇ
    if (wc.hIcon == NULL)
    {
        DWORD dw = GetLastError();
        // TODO : �G���[���O�o��
    }
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // �J�[�\���̐ݒ� ( �f�t�H���g )
    wc.hbrBackground = NULL; // �h��Ԃ��Ȃ�
    wc.lpszMenuName = NULL;  // ���j���[�Ȃ�
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
    // �E�B���h�E�N���X�̓o�^���� 
    // �E�B���h�E�N���X��, �C���X�^���X�n���h��
    ::UnregisterClassA(name_.c_str(), (HINSTANCE)::GetModuleHandle(nullptr)); // �Ăяo�����̃��W���[���n���h�����擾
}