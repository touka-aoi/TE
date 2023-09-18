#pragma once

#include <string>
#include <Windows.h>
#include <memory>

struct IWindow;
class SwapChain;

struct WindowClass final
{
public:
	WindowClass(const std::string& name, HINSTANCE hInst, ::WNDPROC procedure = ::DefWindowProc); 
	~WindowClass();

	const std::string& GetName() const;

	WindowClass(const WindowClass&) = delete;
	WindowClass& operator= (const WindowClass&) = delete;

private:
	std::string name_;
};

// Window Events Interface ( this class is used to notify the owner of the window about the events )
class IWindowOwner
{
public:
	virtual void OnWindowCreate(HWND hwnd_) = 0;
	virtual void OnWindowResize(HWND) = 0;
	virtual void OnToggleFullscreen(HWND) = 0;
	virtual void OnWindowMinimize(HWND hwnd_) = 0;
	virtual void OnWindowFocus(HWND hwnd_) = 0;
	virtual void OnWindowLoseFocus(HWND hwnd_) = 0;
	virtual void OnWindowClose(HWND hwnd_) = 0;
	virtual void OnWindowActivate(HWND hwnd_) = 0;
	virtual void OnWindowDeactivate(HWND hwnd_) = 0;
	virtual void OnWindowMove(HWND hwnd_, int x, int y) = 0;
	virtual void OnDisplayChange(HWND hwnd_, int ImageDepthBitsPerPixel, int ScreenWidth, int ScreenHeight) = 0;

	virtual void OnKeyDown(HWND, WPARAM) = 0;
	virtual void OnKeyUp(HWND, WPARAM) = 0;

	virtual void OnMouseButtonDown(HWND hwnd, WPARAM wParam, bool bIsDoubleClick) = 0;
	virtual void OnMouseButtonUp(HWND, WPARAM) = 0;
	virtual void OnMouseScroll(HWND hwnd, short scrollDirection) = 0;
	virtual void OnMouseMove(HWND hwnd, long x, long y) = 0;
	virtual void OnMouseInput(HWND hwnd, LPARAM lParam) = 0; // Raw Input
};

// Window Interface
struct IWindow
{
public:
	IWindow(IWindowOwner* pOwner_) : pOwner(pOwner_) {}
	IWindow() = default;
	IWindow(const IWindow&) = delete;
	IWindow& operator=(const IWindow&) = delete;

	virtual ~IWindow();

	virtual void Show() = 0;
	virtual void ToggleWindowedFullscreen(SwapChain* pSwapChain = nullptr) = 0;
	virtual void Minimize() = 0;
	virtual void SetMouseCapture(bool bCapture) = 0;
	virtual void Close() = 0;

	bool IsClosed() const;
	bool IsFullscreen() const;
	bool IsMouseCaptured() const;

	inline int GetWidth() const { return GetWidthImpl(); }
	inline int GetHeight() const { return GetHeightImpl(); }
	inline int GetFullscreenWidth() const { return GetFullscreenWidthImpl(); }
	inline int GetFullscreenHeight() const { return GetFullscreenHeightImpl(); }

	IWindowOwner* pOwner;
private:
	virtual bool IsClosedImpl() const = 0;
	virtual bool IsFullscreenImpl() const = 0;
	virtual bool IsMouseCapturedImpl() const = 0;
	virtual int GetWidthImpl() const = 0;
	virtual int GetHeightImpl() const = 0;
	virtual int GetFullscreenWidthImpl() const = 0;
	virtual int GetFullscreenHeightImpl() const = 0;
};

// window procedure
using pfnWndProc_t = LRESULT(CALLBACK*)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
struct FWindowDesc;

#define __MUST_BE_CALLED_FROM_WINMAIN_THREAD__

// Window Implementation
class Window : public IWindow
{
public:
	Window(const std::string& title, FWindowDesc& initParams);

	HWND GetHWND() const;

	void __MUST_BE_CALLED_FROM_WINMAIN_THREAD__ Show() override;
	void                                        Minimize() override;
	void                                        ToggleWindowedFullscreen(SwapChain* pSwapChain = nullptr) override;
	void __MUST_BE_CALLED_FROM_WINMAIN_THREAD__ Close() override;
	void __MUST_BE_CALLED_FROM_WINMAIN_THREAD__ SetMouseCapture(bool bCapture) override;

	inline void OnResize(int w, int h) { width_ = w; height_ = h; }
	inline void SetFullscreen(bool b) { isFullscreen_ = b; }
	inline void SetIsOnHDRCapableDisplay(bool b) { isOnHDRCapableDisplay_ = b; }

	inline bool GetIsOnHDRCapableDisplay() const { return isOnHDRCapableDisplay_; }

private:
	inline bool IsClosedImpl()  const override { return isClosed_; }
	inline bool IsFullscreenImpl() const override { return isFullscreen_; }
	inline bool IsMouseCapturedImpl() const override { return isMouseCaptured_; }
	inline int  GetWidthImpl()  const override { return width_; }
	inline int  GetHeightImpl() const override { return height_; }
	inline int  GetFullscreenWidthImpl()  const override { return FSwidth_; }
	inline int  GetFullscreenHeightImpl() const override { return FSheight_; }

private:
	std::unique_ptr<WindowClass> windowClass_;
	HWND hwnd_ = 0;
	RECT rect_;
	bool isClosed_ = false; // クローズフラグ
	int width_ = -1, height_ = -1; // ウィンドウサイズ 
	bool isFullscreen_ = false; // フルスクリーンフラグ
	UINT windowStyle_; // ウィンドウスタイル
	int FSwidth_ = -1, FSheight_ = -1; // フルスクリーン時のサイズ
	bool isMouseCaptured_ = false;
	bool isOnHDRCapableDisplay_ = false;
};
