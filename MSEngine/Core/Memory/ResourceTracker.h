#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <cstring>

#include "Core/Memory/MemoryTracker.h"

using Microsoft::WRL::ComPtr;

// Traits
template<typename T> struct ResourceCategory;
template<> struct ResourceCategory<ID3D11Buffer> { static constexpr ResourceType value = ResourceType::Buffer; };
template<> struct ResourceCategory<ID3D11VertexShader> { static constexpr ResourceType value = ResourceType::Shader; };
template<> struct ResourceCategory<ID3D11PixelShader> { static constexpr ResourceType value = ResourceType::Shader; };
template<> struct ResourceCategory<ID3D11Texture2D> { static constexpr ResourceType value = ResourceType::Texture; };

template<typename T>
class TrackedComResource {
    ComPtr<T> resource;
    size_t byteSize = 0;

public:
    TrackedComResource(ComPtr<T> res, size_t size, const char* debugName = nullptr)
        : resource(res), byteSize(size)
    {
        g_metrics.OnResourceCreated(ResourceCategory<T>::value, byteSize);
        if (debugName != nullptr) {
            resource.Get()->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(debugName)), debugName);
        }
    }

    ~TrackedComResource()
    {
        g_metrics.OnResourceDestroyed(ResourceCategory<T>::value, byteSize);
    }

    //기존 코드 호환용 세 개
    T* Get() const { return resource.Get(); }
    T** GetAddressOf() { return resource.GetAddressOf(); }
    T* operator->() const { return resource.operator->(); }
};

