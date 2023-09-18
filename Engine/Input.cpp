#include "Input.h"

#include <algorithm>
#include <cassert>

// KeyInputとイベントのマッピングを行うクラス

// キーマッピング
static const Input::KeyMapping KEY_MAP = []()
{
	Input::KeyMapping m;
	m["F1"] = 112;	m["F2"] = 113;	m["F3"] = 114;	m["F4"] = 115;
	m["F5"] = 116;	m["F6"] = 117;	m["F7"] = 118;	m["F8"] = 119;
	m["F9"] = 120;	m["F10"] = 121;	m["F11"] = 122;	m["F12"] = 123;

	m["0"] = 48;		m["1"] = 49;	m["2"] = 50;	m["3"] = 51;
	m["4"] = 52;		m["5"] = 53;	m["6"] = 54;	m["7"] = 55;
	m["8"] = 56;		m["9"] = 57;

	m["A"] = 65;		m["a"] = 65;	m["B"] = 66;	m["b"] = 66;
	m["C"] = 67;		m["c"] = 67;	m["N"] = 78;	m["n"] = 78;
	m["R"] = 82;		m["r"] = 82;	m["T"] = 'T';	m["t"] = 'T';
	m["F"] = 'F';		m["f"] = 'F';	m["J"] = 'J';	m["j"] = 'J';
	m["K"] = 'K';		m["k"] = 'K';	m["V"] = 'V';	m["v"] = 'V';
	m["M"] = 'M';		m["m"] = 'M';	m["G"] = 'G';	m["g"] = 'G';
	m["L"] = 'L';		m["l"] = 'L';
	m["Z"] = 'Z';		m["z"] = 'Z';

	m["\\"] = 220;		m[";"] = 186;
	m["'"] = 222;
	m["Shift"] = 16;	m["shift"] = 16;
	m["Enter"] = 13;	m["enter"] = 13;
	m["Backspace"] = 8; m["backspace"] = 8;
	m["Escape"] = 0x1B; m["escape"] = 0x1B; m["ESC"] = 0x1B; m["esc"] = 0x1B; m["Esc"] = 0x1B;
	m["PageUp"] = 33;	m["PageDown"] = 34;
	m["Space"] = VK_SPACE; m["space"] = VK_SPACE;

	m["ctrl"] = VK_CONTROL;  m["Ctrl"] = VK_CONTROL;
	m["rctrl"] = VK_RCONTROL; m["RCtrl"] = VK_RCONTROL; m["rCtrl"] = VK_RCONTROL;
	m["alt"] = VK_MENU;	m["Alt"] = VK_MENU;


	m["Numpad7"] = 103;		m["Numpad8"] = 104;			m["Numpad9"] = 105;
	m["Numpad4"] = 100;		m["Numpad5"] = 101;			m["Numpad6"] = 102;
	m["Numpad1"] = 97;		m["Numpad2"] = 98;			m["Numpad3"] = 99;
	m["Numpad+"] = VK_ADD;	m["Numpad-"] = VK_SUBTRACT;
	m["+"] = VK_ADD;	m["-"] = VK_SUBTRACT;

	return std::move(m);
}();

static constexpr bool IsMouseKey(WPARAM wparam)
{
	// マウスボタンかどうかチェック
	// (左、右、中央、左右、 中央右、 中央左、左右中央)
	return wparam == Input::EMouseButtons::MOUSE_BUTTON_LEFT 
		|| wparam == Input::EMouseButtons::MOUSE_BUTTON_RIGHT 
		|| wparam == Input::EMouseButtons::MOUSE_BUTTON_MIDDLE
		|| wparam == (Input::EMouseButtons::MOUSE_BUTTON_LEFT | Input::EMouseButtons::MOUSE_BUTTON_RIGHT)
		|| wparam == (Input::EMouseButtons::MOUSE_BUTTON_MIDDLE | Input::EMouseButtons::MOUSE_BUTTON_RIGHT)
		|| wparam == (Input::EMouseButtons::MOUSE_BUTTON_MIDDLE | Input::EMouseButtons::MOUSE_BUTTON_LEFT)
		|| wparam == (Input::EMouseButtons::MOUSE_BUTTON_LEFT | Input::EMouseButtons::MOUSE_BUTTON_RIGHT | Input::EMouseButtons::MOUSE_BUTTON_MIDDLE);
}

void Input::InitRawInputDevices(HWND hwnd)
{
	// マウスの登録
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = (USHORT)0x01;	// HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = (USHORT)0x02;	// HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = hwnd;
	// RawInput登録失敗時
	if (FALSE == (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))))	
	{
		//OutputDebugString("Failed to register raw input device!");
	}

	// デバイス情報の取得
	//-----------------------------------------------------
	
	// デバイスの数を取得
	UINT numDevices = 0;
	GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if (numDevices == 0) return;

	// デバイスを取得 (RAWINPUTDEVICELISTはRawInputDeviceのハンドル付き)
	std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
	GetRawInputDeviceList(
		&deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

	// デバイス情報を取得
	std::vector<wchar_t> deviceNameData;
	std::wstring deviceName;
	for (UINT i = 0; i < numDevices; ++i)
	{
		const RAWINPUTDEVICELIST& device = deviceList[i];
		// マウスデバイスの時
		if (device.dwType == RIM_TYPEMOUSE)
		{
			char info[1024];
			sprintf_s(info, "Mouse: Handle=0x%08p\n", device.hDevice);
			//OutputDebugString(info);

			UINT dataSize = 0;
			// デバイス名を取得するためのバッファサイズを取得 ( RIDI_DEVICENAME )
			GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
			if (dataSize)
			{
				// サイズの変更
				deviceNameData.resize(dataSize);
				UINT result = GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICENAME, &deviceNameData[0], &dataSize);
				// エラーチェック
				if (result != UINT_MAX)
				{
					deviceName.assign(deviceNameData.begin(), deviceNameData.end());

					// デバイス名を取得
					// char info[1024];
					// const std::string ndeviceName = StrUtil::UnicodeToASCII(deviceNameData.data());
					// sprintf_s(info, "  Name=%s\n", ndeviceName.c_str());
					//OutputDebugString(info);
				}
			}

			RID_DEVICE_INFO deviceInfo;
			deviceInfo.cbSize = sizeof deviceInfo;
			dataSize = sizeof deviceInfo;
			UINT result = GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &dataSize);
			if (result != UINT_MAX)
			{
#ifdef _DEBUG
				assert(deviceInfo.dwType == RIM_TYPEMOUSE);
#endif
				char info[1024];
				sprintf_s(info,
					"  Id=%u, Buttons=%u, SampleRate=%u, HorizontalWheel=%s\n",
					deviceInfo.mouse.dwId,
					deviceInfo.mouse.dwNumberOfButtons,
					deviceInfo.mouse.dwSampleRate,
					deviceInfo.mouse.fHasHorizontalWheel ? "1" : "0");
				//OutputDebugString(info);
			}
		}
	}
}

bool Input::ReadRawInput_Mouse(LPARAM lParam, MouseInputEventData* pDataOut)
{
	constexpr UINT RAW_INPUT_SIZE_IN_BYTES = 48;

	UINT rawInputSize = RAW_INPUT_SIZE_IN_BYTES;
	LPBYTE inputBuffer[RAW_INPUT_SIZE_IN_BYTES]; // バイトデータのポインタ
	ZeroMemory(inputBuffer, RAW_INPUT_SIZE_IN_BYTES); // バッファの初期化

	GetRawInputData(
		(HRAWINPUT)lParam,
		RID_INPUT,				// INPUT情報を取得
		inputBuffer,			// バッファ
		&rawInputSize,			// バッファサイズ	
		sizeof(RAWINPUTHEADER)	// ヘッダサイズ
	);

	// RAWINPUT構造体のポインタにキャスト
	RAWINPUT* raw = (RAWINPUT*)inputBuffer; assert(raw);
	RAWMOUSE rawMouse = raw->data.mouse;
	bool bIsMouseInptut = false;

	// マウスホイールの移動量を取得
	// マウスホイール処理だけを取り出して、マウスホイール処理かチェックする
	if ((rawMouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL ||
		(rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL)
	{
		// マウスホイールの移動量
		static const unsigned long defaultScrollLinesPerWheelDelta = 3;
		static const unsigned long defaultScrollCharsPerWheelDelta = 1;

		float wheelDelta = (float)(short)rawMouse.usButtonData; // 回転方向の取得
		float numTicks = wheelDelta / WHEEL_DELTA; // ホイールの回転量の取得

		bool isHorizontalScroll = (rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL; // 平行スクロールをチェック
		bool isScrollByPage = false;
		float scrollDelta = numTicks;

		if (isHorizontalScroll)
		{
			// イベントへインプット量を渡す
			pDataOut->scrollChars = defaultScrollCharsPerWheelDelta;
			// スクロール量の取得
			SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &pDataOut->scrollChars, 0);
			scrollDelta *= pDataOut->scrollChars;
		}
		else
		{
			pDataOut->scrollLines = defaultScrollLinesPerWheelDelta;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &pDataOut->scrollLines, 0);
			if (pDataOut->scrollLines == WHEEL_PAGESCROLL)
				isScrollByPage = true;
			else 
				scrollDelta *= pDataOut->scrollLines;
		}

		pDataOut->scrollDelta = scrollDelta;
		bIsMouseInptut = true;
	}

	// マウスの移動を取得
	if ((rawMouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
	{
		bool isVirtualDesktop = (rawMouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;

		// デスクトップの幅、高さを取得
		int width = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
		int height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

		// マウスの絶対座標を取得 ( 65535 で割ることで、0.0f ～ 1.0f の範囲に正規化 )
		int absoluteX = int((rawMouse.lLastX / 65535.0f) * width); 
		int absoluteY = int((rawMouse.lLastY / 65535.0f) * height);
	}
	else if (rawMouse.lLastX != 0 || rawMouse.lLastY != 0)
	{
		pDataOut->relativeX= rawMouse.lLastX;
		pDataOut->relativeY = rawMouse.lLastY;
		bIsMouseInptut = true;
	}

	return bIsMouseInptut;
}

Input::Input()
	: mbIgnoreInput(false)
	, mMouseDelta{ 0, 0 }
	, mMousePosition{ 0, 0 }
	, mMouseScroll(0)
{

	// マウスを初期化 ( 何も押していない状態へ )
	mMouseButtons[EMouseButtons::MOUSE_BUTTON_LEFT] = 0;
	mMouseButtons[EMouseButtons::MOUSE_BUTTON_RIGHT] = 0;
	mMouseButtons[EMouseButtons::MOUSE_BUTTON_MIDDLE] = 0;

	mMouseButtonDoubleClicks = mMouseButtonsPrevious = mMouseButtons;

	// キーボードの初期化 ( 何も押していない状態へ )
	mKeys.fill(0);
	mKeysPrevious.fill(0);
}

Input::Input(Input&& other)
	: mbIgnoreInput(other.mbIgnoreInput.load())
	, mMouseDelta(other.mMouseDelta)
	, mMousePosition(other.mMousePosition)
	, mMouseScroll(other.mMouseScroll)
	, mMouseButtons(other.mMouseButtons)
	, mMouseButtonDoubleClicks(other.mMouseButtonDoubleClicks)
	, mMouseButtonsPrevious(other.mMouseButtonsPrevious)
	, mKeys(other.mKeys)
	, mKeysPrevious(other.mKeysPrevious)
{}

// キー入力をマッピングする
void Input::UpdateKeyDown(KeyDownEventData data)
{
	const auto& key = data.mouse.wparam;

	// マウス
	if (data.mouse.bMouse)
	{
		const EMouseButtons mouseBtn = static_cast<EMouseButtons>(key);
		// キー入力をステートマップにマッピングする ( 同時押しのビットマップ状態を別の形式に変更する )
		if (mouseBtn & EMouseButtons::MOUSE_BUTTON_LEFT) mMouseButtons[EMouseButtons::MOUSE_BUTTON_LEFT] = true;
		if (mouseBtn & EMouseButtons::MOUSE_BUTTON_RIGHT) mMouseButtons[EMouseButtons::MOUSE_BUTTON_RIGHT] = true;
		if (mouseBtn & EMouseButtons::MOUSE_BUTTON_MIDDLE) mMouseButtons[EMouseButtons::MOUSE_BUTTON_MIDDLE] = true;
		if (data.mouse.bDoubleClick)
		{
			if (mouseBtn & EMouseButtons::MOUSE_BUTTON_LEFT) mMouseButtonDoubleClicks[EMouseButtons::MOUSE_BUTTON_LEFT] = true;
			if (mouseBtn & EMouseButtons::MOUSE_BUTTON_RIGHT) mMouseButtonDoubleClicks[EMouseButtons::MOUSE_BUTTON_RIGHT] = true;
			if (mouseBtn & EMouseButtons::MOUSE_BUTTON_MIDDLE) mMouseButtonDoubleClicks[EMouseButtons::MOUSE_BUTTON_MIDDLE] = true;
		}
	}
	// キーボード
	else
	{
		mKeys[key] = true;
	}
}

// キーを離したときのマッピング
void Input::UpdateKeyUp(KeyCode key, bool bIsMouseKey)
{
	if (bIsMouseKey)
	{
		const EMouseButtons mouseBtn = static_cast<EMouseButtons>(key);

		mMouseButtons[mouseBtn] = false;
		mMouseButtonDoubleClicks[mouseBtn] = false;
	}
	else
		mKeys[key] = false;
}

// マウスの位置を保存する ( 最大値1, 最小値-1 )
void Input::UpdateMousePos(long x, long y, short scroll)
{
	mMouseDelta[0] = static_cast<float>((std::max)(-1l, (std::min)(x - mMousePosition[0], 1l)));
	mMouseDelta[1] = static_cast<float>((std::max)(-1l, (std::min)(y - mMousePosition[1], 1l)));

	mMousePosition[0] = x;
	mMousePosition[1] = y;

	mMouseScroll = scroll;
}


// 移動量を保存する
void Input::UpdateMousePos_Raw(int relativeX, int relativeY, short scroll)
{
	mMouseDelta[0] += static_cast<float>(relativeX);
	mMouseDelta[1] += static_cast<float>(relativeY);

	mMousePosition[0] = 0;
	mMousePosition[1] = 0;

	mMouseScroll = scroll;
}

// フレーム処理が終わった後に呼ばれる
void Input::PostUpdate()
{
	mKeysPrevious = mKeys;
	mMouseButtonsPrevious = mMouseButtons;

	// マウスデータをリセット
	mMouseDelta[0] = mMouseDelta[1] = 0;
	mMouseScroll = 0;
}

bool Input::IsKeyDown(KeyCode key) const
{
	return mKeys[key] && !mbIgnoreInput;
}

bool Input::IsKeyDown(const char* key) const
{
	const KeyCode& code = KEY_MAP.at(key);
	return mKeys[code] && !mbIgnoreInput;
}

bool Input::IsKeyDown(const std::string& key) const
{
	const KeyCode& code = KEY_MAP.at(key.c_str());
	return mKeys[code] && !mbIgnoreInput;
}

bool Input::IsKeyUp(const char* key) const
{
	const KeyCode& code = KEY_MAP.at(key);
	return (!mKeys[code] && mKeysPrevious[code]) && !mbIgnoreInput;
}

bool Input::IsKeyTriggered(KeyCode key) const
{
	// 前のキーと違うキーが押されたときトリガーとみなす
	return !mKeysPrevious[key] && mKeys[key] && !mbIgnoreInput;
}

bool Input::IsKeyTriggered(const char* key) const
{
	const KeyCode code = KEY_MAP.at(key);
	return !mKeysPrevious[code] && mKeys[code] && !mbIgnoreInput;
}

bool Input::IsKeyTriggered(const std::string& key) const
{
	const KeyCode code = KEY_MAP.at(key.data());
	return !mKeysPrevious[code] && mKeys[code] && !mbIgnoreInput;
}

bool Input::IsMouseDown(EMouseButtons mbtn) const
{
	return !mbIgnoreInput && mMouseButtons.at(mbtn);
}

bool Input::IsMouseDoubleClick(EMouseButtons mbtn) const
{
	return !mbIgnoreInput && mMouseButtonDoubleClicks.at(mbtn);
}

bool Input::IsMouseUp(EMouseButtons mbtn) const
{
	// マウスボタンが離れたらUp下とみなす
	const bool bButtonUp = !mMouseButtons.at(mbtn) && mMouseButtonsPrevious.at(mbtn);
	return !mbIgnoreInput && bButtonUp;
}

bool Input::IsMouseTriggered(EMouseButtons mbtn) const
{
	const bool bButtonTriggered = mMouseButtons.at(mbtn) && !mMouseButtonsPrevious.at(mbtn);
	return !mbIgnoreInput && bButtonTriggered;
}

bool Input::IsMouseReleased(EMouseButtons mbtn) const
{
	const bool bButtonReleased = !mMouseButtons.at(mbtn) && mMouseButtonsPrevious.at(mbtn);
	return !mbIgnoreInput && bButtonReleased;
}

bool Input::IsMouseScrollUp() const
{
	return mMouseScroll > 0 && !mbIgnoreInput;
}

bool Input::IsMouseScrollDown() const
{
	return mMouseScroll < 0 && !mbIgnoreInput;
}

bool Input::IsAnyMouseDown() const
{
	return
		mMouseButtons.at(EMouseButtons::MOUSE_BUTTON_LEFT)
		|| mMouseButtons.at(EMouseButtons::MOUSE_BUTTON_RIGHT)
		|| mMouseButtons.at(EMouseButtons::MOUSE_BUTTON_MIDDLE)

		|| mMouseButtonDoubleClicks.at(EMouseButtons::MOUSE_BUTTON_RIGHT)
		|| mMouseButtonDoubleClicks.at(EMouseButtons::MOUSE_BUTTON_MIDDLE)
		|| mMouseButtonDoubleClicks.at(EMouseButtons::MOUSE_BUTTON_LEFT)
		;
}