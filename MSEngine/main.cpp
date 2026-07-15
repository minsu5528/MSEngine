#define INITGUID

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "Core/Memory/ResourceTracker.h"
using Microsoft::WRL::ComPtr;


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// WndProc은 WinMain보다 먼저 "선언"이 필요하다.
// 이유: WNDCLASSEX.lpfnWndProc에 이 함수 주소를 등록하려면,
// 컴파일러가 WinMain을 컴파일하는 시점에 이 함수의 존재를 이미 알아야 하기 때문.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
    {  -0.5f,  0.5f, 0.0f },
    {  0.5f, 0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f },
    { 0.5f, 0.5f, 0.0f },
    { 0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&bd, &initData, Buffer_temp.GetAddressOf());
    TrackedComResource<ID3D11Buffer> vertexBuffer(Buffer_temp, bd.ByteWidth, "VertexBuffer_Triangle");

    ComPtr<ID3DBlob> vsBlob;
    HRESULT hrVS = D3DCompileFromFile(
        L"Renderer/Shaders/VertexShader.hlsl",
        nullptr, nullptr,
        "VS", "vs_5_0",
        0, 0,
        &vsBlob, nullptr
    );

    if (FAILED(hrVS))
    {
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
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800.0f;
    viewport.Height = 800.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    deviceContext->RSSetViewports(1, &viewport);

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // R, G, B, A — 검정
    deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);

    deviceContext->Draw(6, 0);

    swapChain->Present(1, 0);

    // 4. 메시지 루프 (GetMessage / TranslateMessage / DispatchMessage)
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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