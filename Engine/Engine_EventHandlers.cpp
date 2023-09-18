#include "Engine.h"
#include "Windows.h"

//---------------------------------------------------------------------
// 
// MAIN THREAD
//
//---------------------------------------------------------------------
void Engine::MainThread_HandleEvents()
{
	// �C�x���g�L���[�ɃC�x���g�������Ă��Ȃ�������I��
	if (mEventQueue_EnToWin_Main.IsEmpty())
		return;

	// �C�x���g�L���[���X���b�v���X���b�v�������̂�ǂݎ��
	mEventQueue_EnToWin_Main.SwapBuffers();
	std::queue<EventPtr_t>& q =  mEventQueue_EnToWin_Main.GetBackContainer();

	// �C�x���g����������
	std::shared_ptr<IEvent> pEvent = nullptr;
	while(!q.empty())
	{
		pEvent = std::move(q.front());
		q.pop();

		switch (pEvent->mType)
		{
		case MOUSE_CAPTURE_EVENT:
		{
			// �}�E�X�L���v�`���[�ݒ���I����
			std::shared_ptr<SetMouseCaptureEvent> p = std::static_pointer_cast<SetMouseCaptureEvent>(pEvent);
			this->SetMouseCaptureForWindow(p->hwnd, p->bCapture, p->bReleaseAtCapturedPosition);
		} break;
		case HANDLE_WINDOW_TRANSITIONS_EVENT:
		{
			// �t���X�N���[���A�E�B���h�E���[�h�̐؂�ւ�
			auto& pWnd = this->GetWindow(pEvent->hwnd);
			HandleWindowTransitions(pWnd, this->GetWindowSettings(pEvent->hwnd));
		} break;
		case SHOW_WINDOW_EVENT:
		{
			// �E�B���h�E�\��
			this->GetWindow(pEvent->hwnd)->Show();
		} break;
		}
	}
}

void Engine::HandleWindowTransitions(std::unique_ptr<Window>& pWin, const FWindowSettings& settings)
{
	if (!pWin) return;

	// ���C���E�B���h�E�ł��邩�`�F�b�N
	const bool bHandlingMainWindowTransition = pWin == mpWinMain;

	// �f�o�b�N�E�B���h�E�����C���E�B���h�E�Ɠ����f�B�X�v���C�Ńt���X�N���[���ɂȂ�̂�h��
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
	// �{�[�_�[���X�E�B���h�E�̏ꍇ�̓E�B���h�E�I�u�W�F�N�g���A�r���I�t���X�N���[���̏ꍇ�̓X���b�v�`�F�[�����n���h�����O����
	
	// �{�[�_�[���X�E�B���h�E�̏ꍇ
	if (settings.DisplayMode == EDisplayMode::BORDERLESS_FULLSCREEN)
	{
		HWND hwnd = pWin->GetHWND();
		// pWin->ToggleWindowedFullscreen(&mRenderer.GetWindowSwapChain(hwnd));

		// ���C���E�B���h�E�̏ꍇ�}�E�X�L���v�`���[��ݒ�
		if (bHandlingMainWindowTransition)
			SetMouseCaptureForWindow(hwnd, true, true);
	}
}

void Engine::SetMouseCaptureForWindow(HWND hwnd, bool bCaptureMouse, bool bReleaseAtCapturedPosition)
{
	auto& pWin = this->GetWindow(hwnd);

	// �E�B���h�E��InputState���擾���悤�Ƃ���
	if (mInputStates.find(hwnd) == mInputStates.end())
	{
		Log::Error("Warning: couldn't find InputState for hwnd=0x%x", hwnd);
	}

	// �}�E�X�L���v�`���[�ݒ�
	pWin->SetMouseCapture(bCaptureMouse);

	// �}�E�X�J�[�\���ʒu�̎擾�A�ۑ�
	if (bCaptureMouse)
	{
		// �}�E�X�J�[�\���̈ʒu���擾���A�L���v�`���[�֕ۑ�����
		GetCursorPos(&this->mMouseCapturePosition);
#if VERBOSE_LOGGING
		Log::Info("Capturing Mouse: Last position=(%d, %d)", this->mMouseCapturePosition.x, this->mMouseCapturePosition.y);
#endif
	}
	else
	{
		// �}�E�X�L���v�`���[����
		if (bReleaseAtCapturedPosition)
		{
			// �J�[�\���̕���
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