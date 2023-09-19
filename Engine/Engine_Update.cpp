#include "Engine.h"

void Engine::SetWindowName(HWND hwnd, const std::string& name) { mWinNameLookup[hwnd] = name; }
void Engine::SetWindowName(const std::unique_ptr<Window>& pWin, const std::string& name) { SetWindowName(pWin->GetHWND(), name); }

const std::string& Engine::GetWindowName(HWND hwnd) const
{
#if _DEBUG
    auto it = mWinNameLookup.find(hwnd);
    if (it == mWinNameLookup.end())
    {
        Log::Error("Couldn't find window<%x> name: HWND not called with SetWindowName()!", hwnd);
        // assert(false); // gonna crash at .at() call anyways.
    }
#endif
    return mWinNameLookup.at(hwnd);
}
