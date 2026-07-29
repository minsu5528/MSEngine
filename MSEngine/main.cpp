#define INITGUID

#include <array>
#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "Core/Memory/ResourceTracker.h"
#include "Core/Math/Transform.h"
#include "Renderer/Camera.h"
#include "Core/Time/Timer.h"

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct ConstantBuffer {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

ComPtr<ID3D11Debug> RunApp(HINSTANCE hInstance, int nCmdShow) {
    ComPtr<ID3D11Debug> debug; // ReportLiveDeviceObjects 교차검증용 — 다른 리소스 소멸 후에도 살아있어야 해서 반환값으로 전달

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"Window class registration failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    HWND hwnd = CreateWindowEx(
        0,
        L"MyWindowClass",
        L"My Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 800,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hwnd)
    {
        MessageBox(nullptr, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> deviceContext;
    ComPtr<IDXGISwapChain> swapChain;

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 1;
    scDesc.BufferDesc.Width = 800; scDesc.BufferDesc.Height = 800;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = hwnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.Windowed = 1;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &scDesc,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        nullptr,
        deviceContext.GetAddressOf()
    );

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Device/SwapChain creation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    device.As(&debug);

    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());

    ComPtr<ID3D11RenderTargetView> renderTargetView;
    device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

    Camera camera;
    Timer timer;

    struct Vertex {
        float x, y, z;
    };

    Vertex vertices[] = {
        { -0.5f, -0.5f, -0.5f }, // 0
        {  0.5f, -0.5f, -0.5f }, // 1
        {  0.5f,  0.5f, -0.5f }, // 2
        { -0.5f,  0.5f, -0.5f }, // 3
        { -0.5f, -0.5f,  0.5f }, // 4
        {  0.5f, -0.5f,  0.5f }, // 5
        {  0.5f,  0.5f,  0.5f }, // 6
        { -0.5f,  0.5f,  0.5f }, // 7
    };

    ComPtr<ID3D11Buffer> vertexBufferTemp;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&bd, &initData, vertexBufferTemp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> vertexBuffer(vertexBufferTemp, bd.ByteWidth, "VertexBuffer_Cube");

    UINT indices[36] = {
        3,2,0  ,2,1,0, // 앞
        4,6,7  ,4,5,6, // 뒤
        7,6,3  ,6,2,3, // 위
        0,1,4  ,1,5,4, // 아래
        2,6,1  ,6,5,1, // 오른쪽
        7,3,4  ,3,0,4  // 왼쪽
    };

    ComPtr<ID3D11Buffer> indexBufferTemp;

    D3D11_BUFFER_DESC id = {};
    id.Usage = D3D11_USAGE_IMMUTABLE;
    id.ByteWidth = sizeof(indices);
    id.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData1 = {};
    initData1.pSysMem = indices;

    device->CreateBuffer(&id, &initData1, indexBufferTemp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> indexBuffer(indexBufferTemp, id.ByteWidth, "IndexBuffer_Cube");

    ComPtr<ID3D11Buffer> constantBufferTemp;

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&cbd, nullptr, constantBufferTemp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> constantBuffer(constantBufferTemp, cbd.ByteWidth, "ConstantBuffer_Cube");

    std::array<Transform, 3> cubes;
    cubes[0].position = { -2.0f, 0.0f, 0.0f };
    cubes[0].rotation = { 0.5f, 0.5f, 0.0f };

    cubes[1].position = { 0.0f, 0.0f, 0.0f };
    cubes[1].rotation = { 0.5f, 0.5f, 0.0f };

    cubes[2].position = { 2.0f, 0.0f, 0.0f };
    cubes[2].rotation = { 0.5f, 0.5f, 0.0f };

    XMMATRIX view;
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 800.0f, 0.01f, 100.0f);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 800;
    depthDesc.Height = 800;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthTextureTemp;
    device->CreateTexture2D(&depthDesc, nullptr, depthTextureTemp.GetAddressOf());

    ComPtr<ID3D11DepthStencilView> depthStencilView;
    device->CreateDepthStencilView(depthTextureTemp.Get(), nullptr, depthStencilView.GetAddressOf());

    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_BACK;
    ComPtr<ID3D11RasterizerState> rasterState;
    HRESULT hrRast = device->CreateRasterizerState(&rastDesc, rasterState.GetAddressOf());
    if (FAILED(hrRast))
    {
        OutputDebugString(L"CreateRasterizerState failed\n");
    }
    deviceContext->RSSetState(rasterState.Get());

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hrVS = D3DCompileFromFile(
        L"Renderer/Shaders/VertexShader.hlsl",
        nullptr, nullptr,
        "VS", "vs_5_0",
        0, 0,
        &vsBlob, errorBlob.GetAddressOf()
    );

    if (FAILED(hrVS))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        MessageBox(nullptr, L"Vertex shader compilation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    ComPtr<ID3D11VertexShader> vertexShaderTemp;
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vertexShaderTemp.GetAddressOf());
    TrackedComResource<ID3D11VertexShader> vertexShader(vertexShaderTemp, vsBlob->GetBufferSize(), "VertexShader_Cube");

    ComPtr<ID3DBlob> psBlob;
    HRESULT hrPS = D3DCompileFromFile(
        L"Renderer/Shaders/PixelShader.hlsl",
        nullptr, nullptr,
        "PS", "ps_5_0",
        0, 0,
        &psBlob, nullptr
    );

    if (FAILED(hrPS))
    {
        MessageBox(nullptr, L"Pixel shader compilation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    ComPtr<ID3D11PixelShader> pixelShaderTemp;
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShaderTemp.GetAddressOf());
    TrackedComResource<ID3D11PixelShader> pixelShader(pixelShaderTemp, psBlob->GetBufferSize(), "PixelShader_Cube");

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ComPtr<ID3D11InputLayout> inputLayout;
    device->CreateInputLayout(layout, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    deviceContext->IASetInputLayout(inputLayout.Get());
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    deviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf()); // register(b0)
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800.0f;
    viewport.Height = 800.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    deviceContext->RSSetViewports(1, &viewport);

    bool running = true;
    MSG msg = {};
    while (running)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { running = false; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            timer.Tick();
            float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
            deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            if (GetAsyncKeyState('W') & 0x8000)
                camera.MoveForward(5.0f * timer.GetDeltaTime());
            if (GetAsyncKeyState('S') & 0x8000)
                camera.MoveForward(-5.0f * timer.GetDeltaTime());
            if (GetAsyncKeyState('A') & 0x8000)
                camera.MoveRight(-5.0f * timer.GetDeltaTime());
            if (GetAsyncKeyState('D') & 0x8000)
                camera.MoveRight(5.0f * timer.GetDeltaTime());

            view = camera.GetViewMatrix();

            for (auto& cube : cubes)
            {
                cube.rotation.y += 0.01f;

                D3D11_MAPPED_SUBRESOURCE mapped = {};
                deviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                ConstantBuffer* cb = (ConstantBuffer*)mapped.pData;
                cb->world = XMMatrixTranspose(cube.GetWorldMatrix());
                cb->view = XMMatrixTranspose(view);
                cb->projection = XMMatrixTranspose(projection);
                deviceContext->Unmap(constantBuffer.Get(), 0);

                deviceContext->DrawIndexed(36, 0, 0);
            }

            swapChain->Present(1, 0);
        }
    }

    // ReportLiveDeviceObjects 검증 전 파이프라인 바인딩 해제
    deviceContext->ClearState();
    deviceContext->Flush();

    return debug;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ComPtr<ID3D11Debug> debug = RunApp(hInstance, nCmdShow);

    AllocConsole();

    if (debug) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
    else {
        OutputDebugString(L"Debug interface unavailable\n");
    }

    FILE* dummyOut;
    freopen_s(&dummyOut, "CONOUT$", "w", stdout);

    FILE* dummyIn;
    freopen_s(&dummyIn, "CONIN$", "r", stdin);

    g_metrics.PrintReport();

    std::cin.get();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}