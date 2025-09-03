#pragma once

struct dx11_backend_data
{
    ID3D11Device *D3dDevice;
    ID3D11DeviceContext *D3dDeviceContext;
    IDXGIFactory *DxgiFactory;
    ID3D11Buffer *VertexBuffer;
    ID3D11Buffer *PixelBuffer;
    ID3D11VertexShader *VertexShader;
    ID3D11InputLayout *InputLayout;
    ID3D11Buffer *VertexConstantBuffer;
    ID3D11PixelShader *PixelShader;
    ID3D11SamplerState *FontSampler;
    ID3D11ShaderResourceView *FontTextureView;
    ID3D11RasterizerState *RasterizerState;
    ID3D11BlendState *BlendState;
    ID3D11DepthStencilState *DepthStencilState;
    i32 VertexBufferSize;
    i32 IndexBufferSize;

    dx11_backend_data()
    {
        *this = {};
        // memset((void*)this, 0, sizeof(*this));
        VertexBufferSize = 5000;
        IndexBufferSize = 10000;
    }
};

struct dx11_backup_state
{
    UINT ScissorsRectanglesCount;
    UINT ViewportsCount;
    D3D11_RECT ScissorsRectangles[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    ID3D11RasterizerState *RasterizerState;
    ID3D11BlendState *BlendState;
    FLOAT BlendFactor[4];
    UINT SampleMask;
    UINT StencilReference;
    ID3D11DepthStencilState *DepthStencilState;
    ID3D11ShaderResourceView *PixelShaderResourceView;
    ID3D11SamplerState *PixelShaderSampler;
    ID3D11PixelShader *PixelShader;
    ID3D11VertexShader *VertexShader;
    ID3D11GeometryShader *GeometryShader;
    UINT PixelShaderInstanceCount;
    UINT VertexShaderInstanceCount;
    UINT GeometryShaderInstanceCount;

    // 256 is max according to PSSetShader documentation
    ID3D11ClassInstance *PixelShaderInstances[256];
    ID3D11ClassInstance *VertexShaderInstances[256];
    ID3D11ClassInstance *GeometryShaderInstances[256];
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
    ID3D11Buffer *IndexBuffer;
    ID3D11Buffer *VertexBuffer;
    ID3D11Buffer *VertexShaderConstantBuffer;
    UINT IndexBufferOffset;
    UINT VertexBufferStride;
    UINT VertexBufferOffset;
    DXGI_FORMAT IndexBufferFormat;
    ID3D11InputLayout *InputLayout;
};

struct dx11_vertex_constant_buffer
{
    f32 Mvp[4][4];
};

b32 Dx11_Initialize(ID3D11Device *D3dDevice, ID3D11DeviceContext *D3dDeviceContext);
void Dx11_NewFrame();
void Dx11_RenderDrawData(ImDrawData *DrawData);
void Dx11_Shutdown();