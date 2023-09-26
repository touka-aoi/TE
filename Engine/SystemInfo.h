#pragma once

#include <string>
#include <vector>
#include <array>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

// fwd decls
struct IDXGIOutput;
struct IDXGIAdapter1;

namespace SystemInfo
{
    //
    // GPU
    //
    struct FGPUInfo
    {
        std::string    DeviceName;
        unsigned       DeviceID;
        unsigned       VendorID;
        size_t         DedicatedGPUMemory;
        IDXGIAdapter1* pAdapter;
        D3D_FEATURE_LEVEL MaxSupportedFeatureLevel; 

    };
}
