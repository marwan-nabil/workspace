#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <timeapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <dxgi.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\math\floats.h"
#include "directx_demo.h"

application_data GlobalApplicationData;
d3d_state GlobalD3dState;
scene_data GlobalSceneData;

inline void SafeReleaseComObject(IUnknown **Object)
{
    if (*Object != NULL)
    {
        (*Object)->Release();
        *Object = NULL;
    }
}

static void Update(f32 TimeDelta)
{
    DirectX::XMVECTOR EyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
    DirectX::XMVECTOR FocusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    DirectX::XMVECTOR UpDirection = DirectX::XMVectorSet(0, 1, 0, 0);

    GlobalD3dState.ViewMatrix = DirectX::XMMatrixLookAtLH(EyePosition, FocusPoint, UpDirection);
    GlobalD3dState.DeviceContext->UpdateSubresource
    (
        GlobalD3dState.ConstantBuffers[CBT_VIEW_MATRIX],
        0,
        NULL,
        &GlobalD3dState.ViewMatrix,
        0,
        0
    );

    DirectX::XMVECTOR RotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
    GlobalSceneData.CubeRotationAngle += 90.0f * TimeDelta;
    GlobalD3dState.WorldMatrix = DirectX::XMMatrixRotationAxis
    (
        RotationAxis,
        DirectX::XMConvertToRadians(GlobalSceneData.CubeRotationAngle)
    );

    GlobalD3dState.DeviceContext->UpdateSubresource
    (
        GlobalD3dState.ConstantBuffers[CBT_WORLD_MATRIX],
        0,
        NULL,
        &GlobalD3dState.WorldMatrix,
        0,
        0
    );
}

static void Render()
{
    Assert(GlobalD3dState.Device);
    Assert(GlobalD3dState.DeviceContext);

    ID3D11DeviceContext *DC = GlobalD3dState.DeviceContext;

    DC->ClearRenderTargetView
    (
        GlobalD3dState.RenderTargetView,
        DirectX::Colors::CornflowerBlue
    );

    DC->ClearDepthStencilView
    (
        GlobalD3dState.DepthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0F,
        0
    );

    u32 VertexStride = sizeof(vertex);
    u32 Offset = 0;

    DC->IASetVertexBuffers(0, 1, &GlobalD3dState.VertexBuffer, &VertexStride, &Offset);
    DC->IASetInputLayout(GlobalD3dState.InputLayout);
    DC->IASetIndexBuffer(GlobalD3dState.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DC->VSSetShader(GlobalD3dState.VertexShader, NULL, 0);
    DC->VSSetConstantBuffers(0, 3, GlobalD3dState.ConstantBuffers);

    DC->RSSetState(GlobalD3dState.RasterizerState);
    DC->RSSetViewports(1, &GlobalD3dState.ViewPort);

    DC->PSSetShader(GlobalD3dState.PixelShader, NULL, 0);

    DC->OMSetRenderTargets(1, &GlobalD3dState.RenderTargetView, GlobalD3dState.DepthStencilView);
    DC->OMSetDepthStencilState(GlobalD3dState.DepthStencilState, 1);

    DC->DrawIndexed(ArrayCount(GlobalSceneData.CubeVertexIndices), 0, 0);

    if (GlobalApplicationData.EnableVSync)
    {
        GlobalD3dState.SwapChain->Present(1, 0);
    }
    else
    {
        GlobalD3dState.SwapChain->Present(0, 0);
    }
}

void UnloadDemoContent()
{
    d3d_state *D3D = &GlobalD3dState;
    SafeReleaseComObject((IUnknown **)&D3D->ConstantBuffers[CBT_PROJECTION_MATRIX]);
    SafeReleaseComObject((IUnknown **)&D3D->ConstantBuffers[CBT_VIEW_MATRIX]);
    SafeReleaseComObject((IUnknown **)&D3D->ConstantBuffers[CBT_WORLD_MATRIX]);
    SafeReleaseComObject((IUnknown **)&D3D->IndexBuffer);
    SafeReleaseComObject((IUnknown **)&D3D->VertexBuffer);
    SafeReleaseComObject((IUnknown **)&D3D->InputLayout);
    SafeReleaseComObject((IUnknown **)&D3D->VertexShader);
    SafeReleaseComObject((IUnknown **)&D3D->PixelShader);
}

static void LoadDemoContent()
{
    Assert(GlobalD3dState.Device);

    D3D11_BUFFER_DESC VertexBufferDescriptor = {};
    VertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDescriptor.ByteWidth = sizeof(vertex) * ArrayCount(GlobalSceneData.CubeVertices);
    VertexBufferDescriptor.CPUAccessFlags = 0;
    VertexBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA SubResourceData = {};
    SubResourceData.pSysMem = (void *)GlobalSceneData.CubeVertices;

    HRESULT Result = GlobalD3dState.Device->CreateBuffer
    (
        &VertexBufferDescriptor,
        &SubResourceData,
        &GlobalD3dState.VertexBuffer
    );

    D3D11_BUFFER_DESC IndexBufferDescriptor = {};
    IndexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDescriptor.ByteWidth = sizeof(u16) * ArrayCount(GlobalSceneData.CubeVertexIndices);
    IndexBufferDescriptor.CPUAccessFlags = 0;
    IndexBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;

    SubResourceData.pSysMem = (void *)GlobalSceneData.CubeVertexIndices;

    Result = GlobalD3dState.Device->CreateBuffer
    (
        &IndexBufferDescriptor,
        &SubResourceData,
        &GlobalD3dState.IndexBuffer
    );

    D3D11_BUFFER_DESC ConstantBufferDescriptor = {};
    ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ConstantBufferDescriptor.ByteWidth = sizeof(DirectX::XMMATRIX);
    ConstantBufferDescriptor.CPUAccessFlags = 0;
    ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;

    Result = GlobalD3dState.Device->CreateBuffer
    (
        &ConstantBufferDescriptor,
        NULL,
        &GlobalD3dState.ConstantBuffers[CBT_PROJECTION_MATRIX]
    );

    Result = GlobalD3dState.Device->CreateBuffer
    (
        &ConstantBufferDescriptor,
        NULL,
        &GlobalD3dState.ConstantBuffers[CBT_VIEW_MATRIX]
    );

    Result = GlobalD3dState.Device->CreateBuffer
    (
        &ConstantBufferDescriptor,
        NULL,
        &GlobalD3dState.ConstantBuffers[CBT_WORLD_MATRIX]
    );

    ID3DBlob *VertexShaderBlob;
    Result = D3DReadFileToBlob(L"vertex_shader.cso", &VertexShaderBlob);
    Result = GlobalD3dState.Device->CreateVertexShader
    (
        VertexShaderBlob->GetBufferPointer(),
        VertexShaderBlob->GetBufferSize(),
        NULL,
        &GlobalD3dState.VertexShader
    );

    ID3DBlob *PixelShaderBlob;
    Result = D3DReadFileToBlob(L"pixel_shader.cso", &PixelShaderBlob);
    Result = GlobalD3dState.Device->CreatePixelShader
    (
        PixelShaderBlob->GetBufferPointer(),
        PixelShaderBlob->GetBufferSize(),
        NULL,
        &GlobalD3dState.PixelShader
    );

    D3D11_INPUT_ELEMENT_DESC InputLayoutDescriptors[2] = {};
    InputLayoutDescriptors[0].SemanticName = "POSITION";
    InputLayoutDescriptors[0].SemanticIndex = 0;
    InputLayoutDescriptors[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    InputLayoutDescriptors[0].InputSlot = 0;
    InputLayoutDescriptors[0].AlignedByteOffset = OffsetOf(vertex, Position);
    InputLayoutDescriptors[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    InputLayoutDescriptors[0].InstanceDataStepRate = 0;

    InputLayoutDescriptors[1].SemanticName = "COLOR";
    InputLayoutDescriptors[1].SemanticIndex = 0;
    InputLayoutDescriptors[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    InputLayoutDescriptors[1].InputSlot = 0;
    InputLayoutDescriptors[1].AlignedByteOffset = OffsetOf(vertex, Color);
    InputLayoutDescriptors[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    InputLayoutDescriptors[1].InstanceDataStepRate = 0;

    Result = GlobalD3dState.Device->CreateInputLayout
    (
        InputLayoutDescriptors,
        ArrayCount(InputLayoutDescriptors),
        VertexShaderBlob->GetBufferPointer(),
        VertexShaderBlob->GetBufferSize(),
        &GlobalD3dState.InputLayout
    );

    SafeReleaseComObject((IUnknown **)&VertexShaderBlob);
    SafeReleaseComObject((IUnknown **)&PixelShaderBlob);

    RECT ClientRect;
    GetClientRect(GlobalApplicationData.Handle, &ClientRect);

    f32 ClientWidth = (f32)(ClientRect.right - ClientRect.left);
    f32 ClientHeight = (f32)(ClientRect.bottom - ClientRect.top);

    GlobalD3dState.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH
    (
        DirectX::XMConvertToRadians(45.0f),
        ClientWidth / ClientHeight,
        0.1f,
        100.0f
    );

    GlobalD3dState.DeviceContext->UpdateSubresource
    (
        GlobalD3dState.ConstantBuffers[CBT_PROJECTION_MATRIX],
        0,
        NULL,
        &GlobalD3dState.ProjectionMatrix,
        0,
        0
    );
}

static void
InitializeApplicationAndSceneData()
{
    GlobalApplicationData.Width = 1280;
    GlobalApplicationData.Height = 720;
    GlobalApplicationData.Name = "DirectXDemoWindow";
    GlobalApplicationData.ClassName = "directx_demo_window_class";
    GlobalApplicationData.EnableVSync = TRUE;

    GlobalSceneData.CubeVertices[0] = {DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)};
    GlobalSceneData.CubeVertices[1] = {DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)};
    GlobalSceneData.CubeVertices[2] = {DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f)};
    GlobalSceneData.CubeVertices[3] = {DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f)};
    GlobalSceneData.CubeVertices[4] = {DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f)};
    GlobalSceneData.CubeVertices[5] = {DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f)};
    GlobalSceneData.CubeVertices[6] = {DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)};
    GlobalSceneData.CubeVertices[7] = {DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f)};

    u16 CubeVertexIndices[36] =
    {
        0, 1, 2,
        0, 2, 3,
        4, 6, 5,
        4, 7, 6,
        4, 5, 1,
        4, 1, 0,
        3, 2, 6,
        3, 6, 7,
        1, 5, 6,
        1, 6, 2,
        4, 0, 3,
        4, 3, 7
    };
    memcpy(GlobalSceneData.CubeVertexIndices, CubeVertexIndices, sizeof(CubeVertexIndices));
}

DXGI_RATIONAL QueryRefreshRate(u32 ScreenWidth, u32 ScreenHeight, b32 EnableVSync)
{
    DXGI_RATIONAL FoundRefreshRate = {0, 1};

    if (EnableVSync)
    {
        IDXGIFactory *DxgiFactory;
        IDXGIAdapter *DxgiAdapter;
        IDXGIOutput *DxgiAdapterOutput;
        DXGI_MODE_DESC *DxgiDisplayModes;

        HRESULT Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&DxgiFactory);
        Result = DxgiFactory->EnumAdapters(0, &DxgiAdapter);
        Result = DxgiAdapter->EnumOutputs(0, &DxgiAdapterOutput);

        u32 NumberOfDisplayModes;
        Result = DxgiAdapterOutput->GetDisplayModeList
        (
            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_ENUM_MODES_INTERLACED,
            &NumberOfDisplayModes,
            NULL
        );

        DxgiDisplayModes =
            (DXGI_MODE_DESC *)
            malloc(sizeof(DXGI_MODE_DESC) * NumberOfDisplayModes);
        Assert(DxgiDisplayModes);

        Result = DxgiAdapterOutput->GetDisplayModeList
        (
            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_ENUM_MODES_INTERLACED,
            &NumberOfDisplayModes,
            DxgiDisplayModes
        );

        for (u32 Index = 0; Index < NumberOfDisplayModes; Index++)
        {
            if
            (
                (DxgiDisplayModes[Index].Width == ScreenWidth) &&
                (DxgiDisplayModes[Index].Height == ScreenHeight)
            )
            {
                FoundRefreshRate = DxgiDisplayModes[Index].RefreshRate;
            }
        }

        free(DxgiDisplayModes);
        SafeReleaseComObject((IUnknown **)&DxgiAdapterOutput);
        SafeReleaseComObject((IUnknown **)&DxgiAdapter);
        SafeReleaseComObject((IUnknown **)&DxgiFactory);
    }

    return FoundRefreshRate;
}

static void
CleanupD3dState()
{
    d3d_state *D3D = &GlobalD3dState;
    SafeReleaseComObject((IUnknown **)&D3D->DepthStencilView);
    SafeReleaseComObject((IUnknown **)&D3D->RenderTargetView);
    SafeReleaseComObject((IUnknown **)&D3D->DepthStencilBuffer);
    SafeReleaseComObject((IUnknown **)&D3D->DepthStencilState);
    SafeReleaseComObject((IUnknown **)&D3D->RasterizerState);
    SafeReleaseComObject((IUnknown **)&D3D->SwapChain);
    SafeReleaseComObject((IUnknown **)&D3D->DeviceContext);
    SafeReleaseComObject((IUnknown **)&D3D->Device);
}

static void
InitializeD3dState()
{
    Assert(GlobalApplicationData.Handle);

    RECT ClientRectangle;
    GetClientRect(GlobalApplicationData.Handle, &ClientRectangle);

    u32 ClientAreaWidth = ClientRectangle.right - ClientRectangle.left;
    u32 ClientAreaHeight = ClientRectangle.bottom - ClientRectangle.top;

    DXGI_SWAP_CHAIN_DESC SwapChainDescriptor = {};
    ZeroMemory(&SwapChainDescriptor, sizeof(DXGI_SWAP_CHAIN_DESC));
    SwapChainDescriptor.BufferCount = 1;
    SwapChainDescriptor.BufferDesc.Width = ClientAreaWidth;
    SwapChainDescriptor.BufferDesc.Height = ClientAreaHeight;
    SwapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescriptor.BufferDesc.RefreshRate =
        QueryRefreshRate(ClientAreaWidth, ClientAreaHeight, GlobalApplicationData.EnableVSync);
    SwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescriptor.OutputWindow = GlobalApplicationData.Handle;
    SwapChainDescriptor.SampleDesc.Count = 1;
    SwapChainDescriptor.SampleDesc.Quality = 0;
    SwapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDescriptor.Windowed = TRUE;

    u32 DeviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL FeatureLevelOptions[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D_FEATURE_LEVEL FeatureLevelPicked;

    HRESULT Result = D3D11CreateDeviceAndSwapChain
    (
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        DeviceCreationFlags,
        FeatureLevelOptions,
        ArrayCount(FeatureLevelOptions),
        D3D11_SDK_VERSION,
        &SwapChainDescriptor,
        &GlobalD3dState.SwapChain,
        &GlobalD3dState.Device,
        &FeatureLevelPicked,
        &GlobalD3dState.DeviceContext
    );

    ID3D11Texture2D *BackBuffer;
    Result = GlobalD3dState.SwapChain->GetBuffer
    (
        0, __uuidof(ID3D11Texture2D), (void **)&BackBuffer
    );

    Result = GlobalD3dState.Device->CreateRenderTargetView
    (
        BackBuffer, NULL, &GlobalD3dState.RenderTargetView
    );

    SafeReleaseComObject((IUnknown **)&BackBuffer);

    D3D11_TEXTURE2D_DESC DepthStencilBufferDescriptor = {};
    DepthStencilBufferDescriptor.ArraySize = 1;
    DepthStencilBufferDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthStencilBufferDescriptor.CPUAccessFlags = 0;
    DepthStencilBufferDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilBufferDescriptor.Width = ClientAreaWidth;
    DepthStencilBufferDescriptor.Height = ClientAreaHeight;
    DepthStencilBufferDescriptor.MipLevels = 1;
    DepthStencilBufferDescriptor.SampleDesc.Count = 1;
    DepthStencilBufferDescriptor.SampleDesc.Quality = 0;
    DepthStencilBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;

    Result = GlobalD3dState.Device->CreateTexture2D
    (
        &DepthStencilBufferDescriptor, NULL,
        &GlobalD3dState.DepthStencilBuffer
    );

    Result = GlobalD3dState.Device->CreateDepthStencilView
    (
        GlobalD3dState.DepthStencilBuffer, NULL, &GlobalD3dState.DepthStencilView
    );

    D3D11_DEPTH_STENCIL_DESC DepthStencilDescriptor = {};
    DepthStencilDescriptor.DepthEnable = TRUE;
    DepthStencilDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilDescriptor.DepthFunc = D3D11_COMPARISON_LESS;
    DepthStencilDescriptor.StencilEnable = FALSE;

    Result = GlobalD3dState.Device->CreateDepthStencilState
    (
        &DepthStencilDescriptor, &GlobalD3dState.DepthStencilState
    );

    D3D11_RASTERIZER_DESC RasterizerDescriptor = {};
    RasterizerDescriptor.AntialiasedLineEnable = FALSE;
    RasterizerDescriptor.CullMode = D3D11_CULL_BACK;
    RasterizerDescriptor.DepthBias = 0;
    RasterizerDescriptor.DepthBiasClamp = 0.0f;
    RasterizerDescriptor.DepthClipEnable = TRUE;
    RasterizerDescriptor.FillMode = D3D11_FILL_SOLID;
    RasterizerDescriptor.FrontCounterClockwise = FALSE;
    RasterizerDescriptor.MultisampleEnable = FALSE;
    RasterizerDescriptor.ScissorEnable = FALSE;
    RasterizerDescriptor.SlopeScaledDepthBias = 0.0f;

    Result = GlobalD3dState.Device->CreateRasterizerState
    (
        &RasterizerDescriptor, &GlobalD3dState.RasterizerState
    );

    GlobalD3dState.ViewPort.Width = (f32)ClientAreaWidth;
    GlobalD3dState.ViewPort.Height = (f32)ClientAreaHeight;
    GlobalD3dState.ViewPort.TopLeftX = 0;
    GlobalD3dState.ViewPort.TopLeftY = 0;
    GlobalD3dState.ViewPort.MinDepth = 0;
    GlobalD3dState.ViewPort.MaxDepth = 1.0f;
}

LRESULT CALLBACK
MainWindowCallback(HWND Window, u32 Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            HDC DC = BeginPaint(Window, &PaintStruct);
            EndPaint(Window, &PaintStruct);
        } break;

        case WM_DESTROY:
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

int CALLBACK WinMain
(
    HINSTANCE Instance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    i32 nCmdShow
)
{
    InitializeApplicationAndSceneData();

    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = &MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GlobalApplicationData.ClassName;

    RegisterClassEx(&WindowClass);

    RECT WindowRectangle;
    WindowRectangle.left = 0;
    WindowRectangle.top = 0;
    WindowRectangle.right = GlobalApplicationData.Width;
    WindowRectangle.bottom = GlobalApplicationData.Height;
    AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, FALSE);

    GlobalApplicationData.Handle = CreateWindowA
    (
        GlobalApplicationData.ClassName,
        GlobalApplicationData.Name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WindowRectangle.right - WindowRectangle.left,
        WindowRectangle.bottom - WindowRectangle.top,
        NULL, NULL, Instance, NULL
    );

    InitializeD3dState();

    ShowWindow(GlobalApplicationData.Handle, nCmdShow);
    UpdateWindow(GlobalApplicationData.Handle);

    LoadDemoContent();

    MSG Message = {};

    f32 TargetFrameRate = 30.0f;
    f32 TargetFrameTime = 1.0f / TargetFrameRate;
    DWORD PreviousFrameTime = timeGetTime();

    while (Message.message != WM_QUIT)
    {
        if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else
        {
            DWORD CurrentTime = timeGetTime();
            f32 TimeDelta = (CurrentTime - PreviousFrameTime) / 1000.0f;
            PreviousFrameTime = CurrentTime;

            TimeDelta = Min(TimeDelta, TargetFrameTime);

            Update(TimeDelta);
            Render();
        }
    }

    UnloadDemoContent();
    CleanupD3dState();

    return (i32)Message.wParam;
}