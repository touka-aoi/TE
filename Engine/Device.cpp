#include "Device.h"
#include "Renderer.h"

#include "Platform.h"
#include "Log.h"
#include "utils.h"

#include <d3d12.h>
#include <dxgi.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef _DEBUG
#pragma comment(lib, "dxguid.lib")
#include <DXGIDebug.h>
#endif 

#include <cassert>
#include <vector>

#define TRUE 1

using namespace SystemInfo;

bool Device::Create(const FDeviceCreateDesc& desc)
{
    HRESULT hr = {};

    // デバッグ・バリデーションレイヤーの有効化
    if (desc.bEnableDebugLayer)
    {
        ID3D12Debug1* pDebugController = nullptr;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        if (hr == S_OK)
        {
            pDebugController->EnableDebugLayer();
            if (desc.bEnableGPUValidationLayer)
            {
                pDebugController->SetEnableGPUBasedValidation(TRUE);
                pDebugController->SetEnableSynchronizedCommandQueueValidation(TRUE);
            }
            pDebugController->Release();
            Log::Info("Device::Create(): Enabled Debug %s", (desc.bEnableGPUValidationLayer ? "and GPU Validation layers" : "layer"));
        }
        else
        {
            Log::Warning("Device::Create(): D3D12GetDebugInterface() returned != S_OK : %l", hr);
        }
    }

    // デバイスの列挙
    std::vector<FGPUInfo> vAdapters = Renderer::EnumerateDX12Adapters(desc.bEnableDebugLayer, desc.pFactory);
    assert(vAdapters.size() > 0);

    FGPUInfo& adapter = vAdapters[0];

    // DXアダプターの所有権を移譲
    this->mpAdapter = std::move(adapter.pAdapter);
    hr = D3D12CreateDevice(this->mpAdapter, adapter.MaxSupportedFeatureLevel, IID_PPV_ARGS(&mpDevice));

}


