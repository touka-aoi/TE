#pragma once

#include <array>
#include <unordered_map>
#include <string>
#include <atomic>

#include "windows.h"
#include "Eventes.h"

#define ENABLE_RAW_INPUT 1

#define NUM_MAX_KEYS 256

using KeyCode = WPARAM;


class Input
{
public:
	enum EMouseButtons
	{	// windows btn codes: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mbuttondown
		MOUSE_BUTTON_LEFT = MK_LBUTTON, // 左クリック
		MOUSE_BUTTON_RIGHT = MK_RBUTTON, // 右クリック
		MOUSE_BUTTON_MIDDLE = MK_MBUTTON // 中央クリック
	};
	// ---------------------------------------------------------------------------------------------
	using KeyState = char;
	using KeyMapping = std::unordered_map<std::string_view, KeyCode>;
	using ButtonStateMap_t = std::unordered_map<EMouseButtons, KeyState>;
	// ---------------------------------------------------------------------------------------------

	static void InitRawInputDevices(HWND hwnd);
	static bool ReadRawInput_Mouse(LPARAM lParam, MouseInputEventData* pDataOut);

	// ---------------------------------------------------------------------------------------------

	Input();
	Input(Input&& other);
	~Input() = default;

	inline void ToggleInputBypassing() { mbIgnoreInput = !mbIgnoreInput; }
	inline void SetInputBypassing(bool b) { mbIgnoreInput.store(b); };
	inline bool GetInputBypassing() const { return mbIgnoreInput.load(); }

	// update state (key states include mouse buttons)
	void UpdateKeyDown(KeyDownEventData);
	void UpdateKeyUp(KeyCode, bool bIsMouseKey);
	void UpdateMousePos(long x, long y, short scroll);
	void UpdateMousePos_Raw(int relativeX, int relativeY, short scroll);
	void PostUpdate();

	// state check
	bool IsKeyDown(KeyCode) const;
	bool IsKeyDown(const char*) const;
	bool IsKeyDown(const std::string&) const;

	bool IsKeyUp(const char*) const;

	bool IsKeyTriggered(KeyCode) const;
	bool IsKeyTriggered(const char*) const;
	bool IsKeyTriggered(const std::string&) const;

	inline const std::array<float, 2>& GetMouseDelta() const { return mMouseDelta; }
	inline float MouseDeltaX() const { return mMouseDelta[0] && !mbIgnoreInput; };
	inline float MouseDeltaY() const { return mMouseDelta[1] && !mbIgnoreInput; };

	bool IsMouseDown(EMouseButtons) const;
	bool IsMouseUp(EMouseButtons) const;
	bool IsMouseDoubleClick(EMouseButtons) const;
	bool IsMouseTriggered(EMouseButtons) const;
	bool IsMouseReleased(EMouseButtons) const;
	bool IsMouseScrollUp() const;
	bool IsMouseScrollDown() const;
	bool IsAnyMouseDown() const; // @mbIgnoreInput doesn't affect this


private:
	// state
	std::atomic<bool>                    mbIgnoreInput;

	// keyboard
	std::array<KeyState, NUM_MAX_KEYS>   mKeys;
	std::array<KeyState, NUM_MAX_KEYS>   mKeysPrevious;

	// mouse
	ButtonStateMap_t                     mMouseButtons;
	ButtonStateMap_t                     mMouseButtonsPrevious;
	ButtonStateMap_t                     mMouseButtonDoubleClicks;

	std::array<float, 2>                 mMouseDelta; // マウスの移動量 (x, y)
	std::array<long, 2>                  mMousePosition;
	short                                mMouseScroll;
};