#pragma once

#include <Windows.h>
#include <dxgi1_6.h>

// �C�x���g�C���^�[�t�F�[�X�����ɁA�e��C�x���g���쐬

// �C�x���g�^�C�v�ꗗ
enum EEventType
{
	// Windows -> Engine window events
	WINDOW_RESIZE_EVENT = 0,
	WINDOW_CLOSE_EVENT,
	TOGGLE_FULLSCREEN_EVENT,
	SET_FULLSCREEN_EVENT,
	SET_VSYNC_EVENT,
	SET_SWAPCHAIN_FORMAT_EVENT,

	// Windows -> Engine input events
	KEY_DOWN_EVENT,
	KEY_UP_EVENT,
	MOUSE_MOVE_EVENT,
	MOUSE_INPUT_EVENT,
	MOUSE_SCROLL_EVENT,

	// Engine -> Windows window events
	MOUSE_CAPTURE_EVENT,
	HANDLE_WINDOW_TRANSITIONS_EVENT,
	SHOW_WINDOW_EVENT,

	NUM_EVENT_TYPES // �C�x���g�^�C�v�̐�
};

/* �C�x���g�C���^�[�t�F�[�X �C�x���g�^�C�v�ƃE�B���h�E�n���h�����w�肷�� */
struct IEvent
{
	IEvent(EEventType Type, HWND hwnd_) :mType(Type), hwnd(hwnd_) {} // �R���X�g���N�^

	EEventType mType = EEventType::NUM_EVENT_TYPES;; // �C�x���g�^�C�v
	HWND hwnd = 0; // �E�B���h�E�n���h��
};

// �}�E�X�L���v�`���C�x���g�̍쐬
struct SetMouseCaptureEvent : public IEvent
{
	SetMouseCaptureEvent(HWND hwnd_, bool bCaptureIn, bool bVisibleIn, bool bReleaseAtCapturedPositionIn)
		: IEvent(EEventType::MOUSE_CAPTURE_EVENT, hwnd_)
		, bCapture(bCaptureIn)
		, bReleaseAtCapturedPosition(bReleaseAtCapturedPositionIn)
		, bVisible(bVisibleIn)
	{}
	bool bCapture = false;
	bool bReleaseAtCapturedPosition = true;
	bool bVisible = false;
};

// �E�B���h�E�J��
struct HandleWindowTransitionsEvent : public IEvent
{
	HandleWindowTransitionsEvent(HWND hwnd_) : IEvent(EEventType::HANDLE_WINDOW_TRANSITIONS_EVENT, hwnd_) {}
};

// �E�B���h�E�\��
struct ShowWindowEvent : public IEvent
{
	ShowWindowEvent(HWND hwnd_) : IEvent(EEventType::SHOW_WINDOW_EVENT, hwnd_) {}
};


//
// WINDOW EVENTS
//

// �E�B���h�E���T�C�Y
struct WindowResizeEvent : public IEvent
{
	WindowResizeEvent(int w, int h, HWND hwnd_) : IEvent(EEventType::WINDOW_RESIZE_EVENT, hwnd_), width(w), height(h) {}

	int width = 0;
	int height = 0;
};

// �E�B���h�E�N���[�Y
struct WindowCloseEvent : public IEvent
{
	WindowCloseEvent(HWND hwnd_) : IEvent(EEventType::WINDOW_CLOSE_EVENT, hwnd_) {}

	// mutable Signal Signal_WindowDependentResourcesDestroyed;
};

//�t���X�N���[���؂�ւ�
struct ToggleFullscreenEvent : public IEvent
{
	ToggleFullscreenEvent(HWND hwnd_) : IEvent(EEventType::TOGGLE_FULLSCREEN_EVENT, hwnd_) {}
};

//�t���X�N���[���ݒ�
struct SetFullscreenEvent : public IEvent
{
	SetFullscreenEvent(HWND hwnd_, bool bFullscreen) : IEvent(EEventType::SET_FULLSCREEN_EVENT, hwnd_), bToggleValue(bFullscreen) {}
	bool bToggleValue = false;
};

//VSync�ݒ�
struct SetVSyncEvent : public IEvent
{
	SetVSyncEvent(HWND hwnd_, bool bVSync) : IEvent(EEventType::SET_VSYNC_EVENT, hwnd_), bToggleValue(bVSync) {}
	bool bToggleValue = false;
};

//�X���b�v�`�F�[���t�H�[�}�b�g�ݒ�
struct SetSwapchainFormatEvent : public IEvent
{
	SetSwapchainFormatEvent(HWND hwnd_, DXGI_FORMAT format_) : IEvent(EEventType::SET_SWAPCHAIN_FORMAT_EVENT, hwnd_), format(format_) {}
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
};

//
// INPUT EVENTS
//

// �L�[���͂�ۑ�����\����
union KeyDownEventData
{
	struct Keyboard
	{
		Keyboard(WPARAM wp, bool bIsMouse) : wparam(wp), bMouse(bIsMouse) {}
		WPARAM wparam = 0;
		bool bMouse;
	} keyboard;
	struct Mouse
	{
		Mouse(WPARAM wp, bool bIsMouse, bool cl) : wparam(wp), bDoubleClick(cl), bMouse(bIsMouse) {}
		WPARAM wparam = 0;
		bool   bDoubleClick = false;
		bool   bMouse;
	} mouse;

	KeyDownEventData(WPARAM wp, bool bIsMouse, bool cl) : mouse(wp, bIsMouse, cl) {}
};

// �L�[���̓C�x���g
struct KeyDownEvent : public IEvent
{
	KeyDownEventData data;
	KeyDownEvent(HWND hwnd_, WPARAM wparam_, bool bIsMouse, bool bDoubleClick_ = false)
		: IEvent(EEventType::KEY_DOWN_EVENT, hwnd_)
		, data(wparam_, bIsMouse, bDoubleClick_)
	{}
};

// �L�[�����C�x���g
struct KeyUpEvent : public IEvent
{
	KeyUpEvent(HWND hwnd_, WPARAM wparam_, bool bIsMouse) : IEvent(EEventType::KEY_UP_EVENT, hwnd_), wparam(wparam_), bMouseEvent(bIsMouse) {}

	bool bMouseEvent = false;
	WPARAM wparam = 0;
};

// �}�E�X�ړ��C�x���g
struct MouseMoveEvent : public IEvent
{
	MouseMoveEvent(HWND hwnd_, long x_, long y_) : IEvent(EEventType::MOUSE_MOVE_EVENT, hwnd_), x(x_), y(y_) {}
	long x = 0;
	long y = 0;
};

// �}�E�X�z�C�[���C�x���g
struct MouseScrollEvent : public IEvent
{
	MouseScrollEvent(HWND hwnd_, short scr) : IEvent(EEventType::MOUSE_SCROLL_EVENT, hwnd_), scroll(scr) {}
	short scroll = 0;
};

// �}�E�X���͂�ۑ�����\����
struct MouseInputEventData
{
	int relativeX = 0;
	int relativeY = 0;
	union
	{
		unsigned long scrollChars;
		unsigned long scrollLines;
	};
	float scrollDelta = 0.0f;
};

// �}�E�X���̓C�x���g
struct MouseInputEvent : public IEvent
{
	MouseInputEvent(const MouseInputEventData& d, HWND hwnd_) : IEvent(EEventType::MOUSE_INPUT_EVENT, hwnd_), data(d) {}
	MouseInputEventData data;
};