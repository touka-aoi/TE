#pragma once

#include "Common.h"

#include <d3d12.h>
#include <string>

class ResourceView;

enum EResourceHeapType
{
    RTV_HEAP = 0,
    DSV_HEAP,
    CBV_SRV_UAV_HEAP,
    SAMPLER_HEAP,

    NUM_HEAP_TYPES
};

class StaticResourceViewHeap
{
public:
    void Create(ID3D12Device* pDevice, const std::string& ResourceName, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 descriptorCount, bool forceCPUVisible = false);
    void Destroy();
    bool AllocDescriptor(uint32 size, ResourceView* pRV);


    inline ID3D12DescriptorHeap* GetHeap() const { return mpHeap; }

private:
    uint32 mIndex;
    uint32 mDescriptorCount;
    uint32 mDescriptorElementSize;

    ID3D12DescriptorHeap* mpHeap;
    bool mbGPUVisible;
};

class UploadHeap
{
public:
    void Create(ID3D12Device* pDevice, SIZE_T uSize, ID3D12CommandQueue* pQueue);
    void Destroy();

    UINT8* Suballocate(SIZE_T uSize, UINT64 uAlign);

    inline UINT8* BasePtr()         const { return mpDataBegin; }
    inline ID3D12Resource* GetResource()     const { return mpUploadHeap; }
    inline ID3D12GraphicsCommandList* GetCommandList()  const { return mpCommandList; }

    void UploadToGPUAndWait(ID3D12CommandQueue* pCmdQueue = nullptr);

private:
    ID3D12Device* mpDevice = nullptr;
    ID3D12Resource* mpUploadHeap = nullptr;
    ID3D12CommandQueue* mpQueue = nullptr;

    ID3D12GraphicsCommandList* mpCommandList = nullptr;
    ID3D12CommandAllocator* mpCommandAllocator = nullptr;

    UINT8* mpDataCur = nullptr;
    UINT8* mpDataEnd = nullptr;
    UINT8* mpDataBegin = nullptr;

    ID3D12Fence* mpFence = nullptr;
    UINT64                     mFenceValue = 0;
    HANDLE                     mHEvent;
};
