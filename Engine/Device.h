#pragma once

#include <dxgiformat.h>

struct IDXGIFactory6;
struct ID3D12Device10;
struct ID3D12Device;
struct IDXGIAdapter;

struct FDeviceCreateDesc
{
	bool bEnableDebugLayer{ false };
};

class Device
{
public:
	bool Create(const FDeviceCreateDesc& desc);
	void Destroy();

	// Getters
	inline ID3D12Device* GetDevicePtr() const { return mpDevice; }
	inline ID3D12Device10* GetDevice10Ptr() const { return mpDevice10; }
	inline IDXGIAdapter* GetAdapterPtr() const { return mpAdapter; }

private:
	ID3D12Device* mpDevice{ nullptr };
	ID3D12Device10* mpDevice10{ nullptr };
	IDXGIAdapter* mpAdapter{ nullptr };
};