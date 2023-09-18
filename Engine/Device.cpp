#include "Device.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


bool Device::Create(const FDeviceCreateDesc& desc)
{
	HRESULT hr{};
	// Debug & Validation Layer

	

	const bool bDeviceCreated = true;

	return bDeviceCreated;
}

void Device::Destroy()
{
	mpAdapter->Release();
	mpDevice->Release();
	if (mpDevice10)
		mpDevice10->Release();
}