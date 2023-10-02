#pragma once

#include "Device.h"

#include "Types.h"
#include "Platform.h"
#include "Settings.h"
#include "SystemInfo.h"
#include "ResourceHeaps.h"
#include "CommandQueue.h"

#include <vector>
#include <unordered_map>
#include <array>
#include <queue>
#include <set>

#include <d3d12.h>
#include <dxgi.h>

class Window;
class ID3D12RootSignature;
class ID3D12PipelineState;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// RENDERER
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer
{
public:
    void Initialize(const FGraphicsSettings& settings);
    void Destroy();

private:
    void InitializeHeaps();

private:

    Device       mDevice;
    CommandQueue mGFXQueue;
    CommandQueue mComputeQueue;
    CommandQueue mCopyQueue;

    // CPU-visible heaps ----------------------------------------
    UploadHeap                     mHeapUpload;
    StaticResourceViewHeap         mHeapRTV;
    // GPU-visible heaps ----------------------------------------
    StaticResourceViewHeap         mHeapCBV_SRV_UAV;
    StaticBufferHeap               mStaticHeap_VertexBuffer;
    StaticBufferHeap               mStaticHeap_IndexBuffer;

public:
    static std::vector< SystemInfo::FGPUInfo > EnumerateDX12Adapters(bool bEnableDebugLayer, bool bEnumerateSoftwareAdapters = false, IDXGIFactory6* pFactory = nullptr);
};
