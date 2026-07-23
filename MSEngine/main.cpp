#define INITGUID

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "Core/Memory/ResourceTracker.h"
#include "Core/Math/Transform.h"

using Microsoft::WRL::ComPtr;


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// WndProc은 WinMain보다 먼저 "선언"이 필요하다.
// 이유: WNDCLASSEX.lpfnWndProc에 이 함수 주소를 등록하려면,
// 컴파일러가 WinMain을 컴파일하는 시점에 이 함수의 존재를 이미 알아야 하기 때문.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct ConstantBuffer {
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

ComPtr<ID3D11Debug> RunApp(HINSTANCE hInstance, int nCmdShow) {
    ComPtr<ID3D11Debug> debug;

    // 1. WNDCLASSEX 채우고 RegisterClassEx 호출
    WNDCLASSEX wc = {};                    // 일단 전부 0으로 초기화
    wc.cbSize = sizeof(WNDCLASSEX);        // 필수 — 이거 빠지면 RegisterClassEx 실패
    wc.lpfnWndProc = WndProc;              // 함수 이름 자체가 함수 포인터로 변환됨
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";   // L 붙여야 함 (UNICODE 빌드라서)

    if (!RegisterClassEx(&wc))
    {
        // 여기 걸리면 cbSize나 lpfnWndProc 안 채운 게 원인일 확률 높음
        MessageBox(nullptr, L"Window class registration failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    // 2. CreateWindowEx 호출 → HWND 받기
    //    (여기서 쓰는 클래스 이름 문자열이 위 등록한 이름과 반드시 일치해야 함)
    HWND hwnd = CreateWindowEx(
        0,                          // dwExStyle: 확장 스타일, 지금은 0
        L"MyWindowClass",           // 여기가 핵심 — RegisterClassEx에 등록한 이름과 "글자 하나까지" 일치해야 함
        L"My Window",               // 창 제목 (타이틀바 텍스트) — 네가 정하는 값
        WS_OVERLAPPEDWINDOW,        // dwStyle: 테두리+최소화/최대화 버튼 있는 일반 창
        CW_USEDEFAULT, CW_USEDEFAULT,  // X, Y 위치 — OS가 알아서 배치하게 두는 매크로
        800, 800,                   // 너비, 높이 — 네가 정하는 값
        nullptr,                    // 부모 창 (없음)
        nullptr,                    // 메뉴 (없음)
        hInstance,
        nullptr                     // 추가 전달 데이터 (없음)
    );

    if (!hwnd)
    {
        // 실패 시 원인 파악: GetLastError() 찍어보면 에러 코드 나온다
        MessageBox(nullptr, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    // 3. ShowWindow(hwnd, nCmdShow), UpdateWindow(hwnd)
    //    → 이거 호출 안 하면 창은 생성돼 있어도 화면에 안 보인다
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    ComPtr<ID3D11Device> device;
    ComPtr <ID3D11DeviceContext> deviceContext;
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
        nullptr,                    // 어떤 그래픽 어댑터(GPU) 쓸지 — nullptr면 기본 어댑터
        D3D_DRIVER_TYPE_HARDWARE,   // GPU로 처리 (소프트웨어 렌더러 아님)
        nullptr,                    // 소프트웨어 렌더러 모듈 (안 씀)
        D3D11_CREATE_DEVICE_DEBUG,                          // 생성 플래그 (디버그 레이어 켤 때 여기 씀, 지금은 0)
        nullptr, 0,                 // 사용할 Feature Level 배열 — nullptr,0이면 지원 가능한 최신 버전 자동 선택
        D3D11_SDK_VERSION,          // 고정값, 항상 이 매크로 그대로 씀
        &scDesc,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        nullptr,                    // 실제로 선택된 Feature Level 받고 싶으면 포인터 넘김, 지금은 필요없어서 nullptr
        deviceContext.GetAddressOf()
    );

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Device/SwapChain creation failed", L"Error", MB_OK | MB_ICONERROR);
        return debug;
    }

    device.As(&debug);

    ComPtr<ID3D11Texture2D>backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());

    ComPtr<ID3D11RenderTargetView> renderTargetView;
    device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

    ComPtr<ID3D11Buffer> Buffer_temp; // vertexBuffer를 위한 temp

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

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&bd, &initData, Buffer_temp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> vertexBuffer(Buffer_temp, bd.ByteWidth, "VertexBuffer_Triangle");

    UINT indices[36] = {
        3,2,0  ,2,1,0, // 앞
        4,6,7  ,4,5,6, // 뒤
        7,6,3  ,6,2,3, // 위
        0,1,4  ,1,5,4, // 아래
        2,6,1  ,6,5,1, // 오른쪽
        7,3,4  ,3,0,4 // 왼쪽
    };

    ComPtr<ID3D11Buffer> Index_Buffer_temp; // indexBuffer를 위한 temp

    D3D11_BUFFER_DESC id = {};
    id.Usage = D3D11_USAGE_IMMUTABLE;
    id.ByteWidth = sizeof(indices);
    id.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData1 = {};
    initData1.pSysMem = indices;

    device->CreateBuffer(&id, &initData1, Index_Buffer_temp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> indexvertexBuffer(Index_Buffer_temp, id.ByteWidth, "IndexVertexBuffer_Triangle");

    ConstantBuffer constant = {};

    ComPtr<ID3D11Buffer> constant_Buffer_temp; // indexBuffer를 위한 temp
    
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&cbd, nullptr, constant_Buffer_temp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> constantvertexBuffer(constant_Buffer_temp, cbd.ByteWidth, "constantVertexBuffer_Triangle");

    Transform cubeTransform;
    cubeTransform.rotation = { 0.5f, 0.5f, 0.0f };

    XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
    XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 800.0f / 800.0f, 0.01f, 100.0f);

    

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 800;
    depthDesc.Height = 800;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // 깊이값 24비트 + 스텐실 8비트
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;   // 지금까지 안 쓴 새 BindFlags 값

    ComPtr<ID3D11Texture2D> depthTexture_temp;
    device->CreateTexture2D(&depthDesc, nullptr, depthTexture_temp.GetAddressOf());

    ComPtr<ID3D11DepthStencilView> depthStencilView;
    device->CreateDepthStencilView(depthTexture_temp.Get(), nullptr, depthStencilView.GetAddressOf());




    D3D11_RASTERIZER_DESC rastDesc = {}; // 컬링
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

    ComPtr<ID3D11VertexShader> VertexShader_temp; // VertexShader를 위한 temp
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, VertexShader_temp.GetAddressOf());
    TrackedComResource<ID3D11VertexShader> vertexShader(VertexShader_temp, vsBlob->GetBufferSize(), "VertexShader_Triangle");


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

    ComPtr <ID3D11PixelShader> pixelShader_temp;
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader_temp.GetAddressOf());
    TrackedComResource<ID3D11PixelShader> pixelShader(pixelShader_temp, psBlob->GetBufferSize(), "PixelShader_Triangle");


    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ComPtr<ID3D11InputLayout> inputLayout;
    device->CreateInputLayout(layout, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());

    UINT stride = sizeof(Vertex);   // 정점 하나의 크기 (바이트)
    UINT offset = 0;                // 버퍼 시작 지점에서 얼마나 떨어진 곳부터 읽을지

    deviceContext->IASetInputLayout(inputLayout.Get());
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(indexvertexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    deviceContext->VSSetConstantBuffers(0, 1, constantvertexBuffer.GetAddressOf());  // 0번 슬롯 = register(b0)
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

    cubeTransform.rotation.y += 0.01f;   // DeltaTime은 Week 7에 연결 예정, 지금은 고정값으로
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
            float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // R, G, B, A — 검정
            deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
            deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            cubeTransform.rotation.y += 0.01f;   // DeltaTime은 Week 7에 연결 예정, 지금은 고정값으로

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            deviceContext->Map(constantvertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            ConstantBuffer* cb = (ConstantBuffer*)mapped.pData;
            cb->world = XMMatrixTranspose(cubeTransform.GetWorldMatrix());
            cb->view = XMMatrixTranspose(view);
            cb->projection = XMMatrixTranspose(projection);
            deviceContext->Unmap(constantvertexBuffer.Get(), 0);

            //deviceContext->Draw(6, 0);
            deviceContext->DrawIndexed(36, 0, 0);

            swapChain->Present(1, 0);
        }
    }

    // 4. 메시지 루프 (GetMessage / TranslateMessage / DispatchMessage)
    /*MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }*/

    deviceContext->ClearState();   // 파이프라인에 바인딩된 모든 리소스 해제
    deviceContext->Flush();        // GPU에 아직 안 보낸 명령 큐를 실제로 실행시킴

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
        OutputDebugString(L"Debug interface unavailable — RunApp may have exited early or debug layer not enabled.\n");
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