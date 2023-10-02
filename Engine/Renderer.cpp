#include "Renderer.h"
#include "Common.h"

#include "Window.h"

#include <cassert>
#include <atomic>

using namespace SystemInfo;


#ifdef _DEBUG
#define ENABLE_DEBUG_LAYER      1
#define ENABLE_VALIDATION_LAYER 1
#else
#define ENABLE_DEBUG_LAYER      0
#define ENABLE_VALIDATION_LAYER 0
#endif

void Renderer::Initialize(const FGraphicsSettings& settings)
{
    // DirectX12デバイスの初期化
    Device* pDevice = &mDevice;
    FDeviceCreateDesc deviceDesc = {};
    deviceDesc.bEnableDebugLayer = ENABLE_DEBUG_LAYER;
    deviceDesc.bEnableGPUValidationLayer = ENABLE_VALIDATION_LAYER;
    const bool bDeviceCreateSucceeded = mDevice.Create(deviceDesc);
    ID3D12Device* pDevice = mDevice.GetDevicePtr();
    assert(bDeviceCreateSucceeded);

    // コマンドキューの初期化
    mGFXQueue.Create(pDevice, CommandQueue::EType::GFX);
    mComputeQueue.Create(pDevice, CommandQueue::EType::COMPUTE);
    mCopyQueue.Create(pDevice, CommandQueue::EType::COPY);

    // メモリの初期化
    InitializeHeaps();

    // スレッドの初期化
    mbExitUploadThread.store(false);
    mbDefaultResourcesLoaded.store(false);
    mTextureUploadThread = std::thread(&VQRenderer::TextureUploadThread_Main, this);

    const size_t HWThreads = ThreadPool::sHardwareThreadCount;
    const size_t HWCores = HWThreads >> 1;
    mWorkers_ShaderLoad.Initialize(HWThreads, "ShaderLoadWorkers");
    mWorkers_PSOLoad.Initialize(HWThreads, "PSOLoadWorkers");

    Log::Info("[Renderer] Initialized.");
}

void Renderer::InitializeHeaps()
{
    ID3D12Device* pDevice = mDevice.GetDevicePtr();

    const uint32 UPLOAD_HEAP_SIZE = (512 + 256) * MEGABYTE;
    mHeapUpload.Create(pDevice, UPLOAD_HEAP_SIZE, this->mGFXQueue.pQueue);

    constexpr uint32 NumDescsCBV = 100;
    constexpr uint32 NumDescsSRV = 8192;
    constexpr uint32 NumDescsUAV = 100;
    constexpr bool   bCPUVisible = false;
    mHeapCBV_SRV_UAV.Create(pDevice, "HeapCBV_SRV_UAV", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumDescsCBV + NumDescsSRV + NumDescsUAV, bCPUVisible);

    constexpr uint32 NumDescsRTV = 1000;
    mHeapRTV.Create(pDevice, "HeapRTV", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NumDescsRTV);

    constexpr uint32 STATIC_GEOMETRY_MEMORY_SIZE = 256 * MEGABYTE;
    constexpr bool USE_GPU_MEMORY = true;
    mStaticHeap_VertexBuffer.Create(pDevice, EBufferType::VERTEX_BUFFER, STATIC_GEOMETRY_MEMORY_SIZE, USE_GPU_MEMORY, "mStaticVertexBufferPool");
    mStaticHeap_IndexBuffer.Create(pDevice, EBufferType::INDEX_BUFFER, STATIC_GEOMETRY_MEMORY_SIZE, USE_GPU_MEMORY, "mStaticIndexBufferPool");
}

void VQRenderer::Destroy()
{
    Log::Info("VQRenderer::Exit()");
    mWorkers_PSOLoad.Destroy();
    mWorkers_ShaderLoad.Destroy();

    mbExitUploadThread.store(true);
    mSignal_UploadThreadWorkReady.NotifyAll();

    // clean up memory
    mHeapUpload.Destroy();
    mHeapCBV_SRV_UAV.Destroy();
    mHeapDSV.Destroy();
    mHeapRTV.Destroy();
    mStaticHeap_VertexBuffer.Destroy();
    mStaticHeap_IndexBuffer.Destroy();

    // clean up textures
    for (std::unordered_map<TextureID, Texture>::iterator it = mTextures.begin(); it != mTextures.end(); ++it)
    {
        it->second.Destroy();
    }
    mTextures.clear();
    mpAllocator->Release();

    // clean up root signatures and PSOs
    for (auto& pr : mRootSignatureLookup)
    {
        if (pr.second) pr.second->Release();
    }
    for (std::pair<PSO_ID, ID3D12PipelineState*> pPSO : mPSOs)
    {
        if (pPSO.second)
            pPSO.second->Release();
    }
    mPSOs.clear();

    // clean up contexts
    size_t NumBackBuffers = 0;
    for (std::unordered_map<HWND, FWindowRenderContext>::iterator it = mRenderContextLookup.begin(); it != mRenderContextLookup.end(); ++it)
    {
        auto& ctx = it->second;
        ctx.CleanupContext();
    }

    // cleanp up device
    mGFXQueue.Destroy();
    mComputeQueue.Destroy();
    mCopyQueue.Destroy();
    mDevice.Destroy();

    // clean up remaining threads
    mTextureUploadThread.join();
}

std::vector<FGPUInfo> Renderer::EnumerateDX12Adapters(bool bEnableDebugLayer, bool bEnumerateSoftwareAdapters /*= false*/, IDXGIFactory6* pFactory /*= nullptr*/)
{
    std::vector< FGPUInfo > GPUs;
    HRESULT hr = {};

    // DX12デバイスの列挙
    IDXGIAdapter1* pAdapter = nullptr;
    int iAdapter = 0; // アダプターのインデックス
    bool bAdapterFound = false;

    IDXGIFactory6* pDxgiFactory = nullptr;
    if (pFactory)
    {
        pDxgiFactory = pFactory;
    }
    else
    {
        UINT DXGIFlags = 0;
        if (bEnableDebugLayer)
        {
            DXGIFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
        hr = CreateDXGIFactory2(DXGIFlags, IID_PPV_ARGS(&pDxgiFactory));
    }
    // アダプター探索の関数を定義
    auto fnAddAdapter = [&bAdapterFound, &GPUs](IDXGIAdapter1*& pAdapter, const DXGI_ADAPTER_DESC1& desc, D3D_FEATURE_LEVEL FEATURE_LEVEL)
    {
        bAdapterFound = true;

        FGPUInfo GPUInfo = {};
        GPUInfo.DedicatedGPUMemory = desc.DedicatedVideoMemory;
        GPUInfo.DeviceID = desc.DeviceId;
        // GPUInfo.DeviceName = StrUtil::UnicodeToASCII<_countof(desc.Description)>(desc.Description);
        GPUInfo.VendorID = desc.VendorId;
        GPUInfo.MaxSupportedFeatureLevel = FEATURE_LEVEL;
        pAdapter->QueryInterface(IID_PPV_ARGS(&GPUInfo.pAdapter));
        GPUs.push_back(GPUInfo);
        int a = 5;
    };

    // アダプターを全探査する
    while (pDxgiFactory->EnumAdapterByGpuPreference(iAdapter, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        // ハードウェアアダプターかソフトウェアアダプターかを判定
        const bool bSoftwareAdapter = desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
        if ((bEnumerateSoftwareAdapters && !bSoftwareAdapter) || (!bEnumerateSoftwareAdapters && bSoftwareAdapter))
        {
            // ハードウェアアダプターを列挙する場合は、ソフトウェアアダプターをスキップする
            // ソフトウェアアダプターを列挙する場合は、ハードウェアアダプターをスキップする
            ++iAdapter;
            pAdapter->Release();
            continue;
        }

        hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr);
        // D3D_FEATURE_LEVEL_12_1がサポートされているかを確認
        if (SUCCEEDED(hr))
        {
            // GPU候補に登録
            fnAddAdapter(pAdapter, desc, D3D_FEATURE_LEVEL_12_1);
        }
        else
        {
            // const std::string AdapterDesc = StrUtil::UnicodeToASCII(desc.Description);
            // Log::Warning("Device::Create(): D3D12CreateDevice() with Feature Level 12_1 failed with adapter=%s, retrying with Feature Level 12_0", AdapterDesc.c_str());
            
            // FeatureLevel 12_0 で再試行
            hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(hr))
            {
                fnAddAdapter(pAdapter, desc, D3D_FEATURE_LEVEL_12_0);
            }
            else
            {
                // Log::Error("Device::Create(): D3D12CreateDevice() with Feature Level 12_0 failed ith adapter=%s", AdapterDesc.c_str());
            }
        }

        pAdapter->Release();
        ++iAdapter;
    }

    if (!pFactory)
    {
        pDxgiFactory->Release();
    }
    assert(bAdapterFound);

    return GPUs;
}
