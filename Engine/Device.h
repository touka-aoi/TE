#pragma once

#include <dxgiformat.h>
#include <unordered_map>

struct IDXGIFactory6;
struct ID3D12Device4;
struct ID3D12Device;
struct IDXGIAdapter;

// Device Settings
struct FDeviceCreateDesc
{
    bool bEnableDebugLayer = false;
    bool bEnableGPUValidationLayer = false;
    IDXGIFactory6* pFactory = nullptr;
};

class Device
{
public:
    bool Create(const FDeviceCreateDesc& desc);
    void Destroy();

    // Getters
    inline ID3D12Device4* GetDevice4Ptr() const { return mpDevice4; }
    inline ID3D12Device* GetDevicePtr() const { return mpDevice; }
    inline IDXGIAdapter* GetAdapterPtr() const { return mpAdapter; }

private:
    ID3D12Device4* mpDevice4 = nullptr;
    ID3D12Device* mpDevice = nullptr;
    IDXGIAdapter* mpAdapter = nullptr;
};

