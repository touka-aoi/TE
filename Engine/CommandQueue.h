#pragma once

struct ID3D12CommandQueue;
class Device;

class CommandQueue
{
public:
    enum EType
    {
        GFX = 0,
        COMPUTE,
        COPY,

        NUM_COMMAND_QUEUE_TYPES
    };

public:
    void Create(Device* pDevice, EType type, const char* pName = nullptr);
    void Destroy();

    ID3D12CommandQueue* pQueue;
};
