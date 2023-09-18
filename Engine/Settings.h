#pragma once

#include <string>

// �A�v���P�[�V�����̐ݒ�

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
	bool bVsync{ false }; // ��������
	bool bUseTripleBuffering{false}; // �g���v���o�b�t�@�����O
	bool bAntiAliasing{ false }; // �A���`�G�C���A�V���O
	EReflections Reflections{ EReflections::REFLECTIONS_OFF };

	float RenderScale{ 1.0f }; // What is this ?
	int   MaxFrameRate{ -1 }; // -1: Auto (RefreshRate x 1.15) | 0: Unlimited | <int>: specified value
	int   EnvironmentMapResolution{ 256 }; // What is this ?
};

struct FWindowSettings
{
	int Width{ -1 };
	int Height{ -1 };
	EDisplayMode DisplayMode  {EDisplayMode::WINDOWED};
	unsigned PreferredDisplay{ 0 }; // �f�B�X�v���C�ݒ�
	char Title[64]{ "" }; // �E�B���h�E�^�C�g��
	bool bEnableHDR{ false }; // HDR 

	// �t���X�N���[�����L�����ǂ������m�F
	inline bool IsDisplayModeFullscreen() const { return DisplayMode == EDisplayMode::EXCLUSIVE_FULLSCREEN || DisplayMode == EDisplayMode::BORDERLESS_FULLSCREEN; }
};

struct FEngineSettings
{
	// �O���t�B�b�N�ݒ�
	FGraphicsSettings gfx;

	// �E�B���h�E�ݒ�
	FWindowSettings WndMain;
	FWindowSettings WndDebug;

	// �f�o�b�O�E�B���h�E�̕\��
	bool bShowDebugWindow{ false };

	// What is this ?
	bool bAutomatedTestRun{ false }; // �����e�X�g���s
	int NumAutomatedTestFrames{ -1 };

	// TODO :: Check this parameter
	std::string StartupScene;
};