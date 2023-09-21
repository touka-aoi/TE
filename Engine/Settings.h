#pragma once

#include <string>

// アプリケーションの設定

enum EDisplayMode
{
	WINDOWED = 0,
	BORDERLESS_FULLSCREEN,
	EXCLUSIVE_FULLSCREEN,

	NUM_DISPLAY_MODES
};

enum EReflections
{
	REFLECTIONS_OFF,
	SCREEN_SPACE_REFLECTIONS__FFX,
	RAY_TRACED_REFLECTIONS,

	NUM_REFLECTION_SETTINGS
};

struct FGraphicsSettings
{
	bool bVsync{ false }; // 垂直同期
	bool bUseTripleBuffering{false}; // トリプルバッファリング
	bool bAntiAliasing{ false }; // アンチエイリアシング
	EReflections Reflections{ EReflections::REFLECTIONS_OFF };

	float RenderScale{ 1.0f }; // What is this ?
	int   MaxFrameRate{ -1 }; // -1: Auto (RefreshRate x 1.15) | 0: Unlimited | <int>: specified value ( Hz )
	int   EnvironmentMapResolution{ 256 }; // What is this ?
};

struct FWindowSettings
{
	int Width{ -1 };
	int Height{ -1 };
	EDisplayMode DisplayMode  {EDisplayMode::WINDOWED};
	unsigned PreferredDisplay{ 0 }; // ディスプレイ設定
	char Title[64]{ "" }; // ウィンドウタイトル
	bool bEnableHDR{ false }; // HDR 

	// フルスクリーンが有効かどうかを確認
	inline bool IsDisplayModeFullscreen() const { return DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN || DisplayMode == EDisplayMode::BORDERLESS_FULLSCREEN; }
};

struct FEngineSettings
{
	// グラフィック設定
	FGraphicsSettings gfx;

	// ウィンドウ設定
	FWindowSettings WndMain;
	FWindowSettings WndDebug;

	// デバッグウィンドウの表示
	bool bShowDebugWindow{ false };

	// What is this ?
	bool bAutomatedTestRun{ false }; // 自動テスト実行
	int NumAutomatedTestFrames{ -1 };

	// TODO :: Check this parameter
	std::string StartupScene;
};
