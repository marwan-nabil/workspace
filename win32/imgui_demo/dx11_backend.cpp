#include <windows.h>
#include <stdint.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\imgui\imgui.h"
#include "dx11_backend.h"

// Backend data stored in ImGuiIoInterface.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static dx11_backend_data *Dx11_GetBackendData()
{
    if (ImGui::GetCurrentContext())
    {
        dx11_backend_data *Result = (dx11_backend_data *)ImGui::GetIO().BackendRendererUserData;
        return Result;
    }
    else
    {
        return NULL;
    }
}

static void Dx11_InvalidateDeviceObjects()
{
    dx11_backend_data *BackendData = Dx11_GetBackendData();
    if (!BackendData->D3dDevice)
    {
        return;
    }

    if (BackendData->FontSampler)
    {
        BackendData->FontSampler->Release();
        BackendData->FontSampler = NULL;
    }

    // We copied data->FontTextureView to ImGuiIoInterface.Fonts->TexID so let's clear that as well.
    if (BackendData->FontTextureView)
    {
        BackendData->FontTextureView->Release();
        BackendData->FontTextureView = NULL;
        ImGui::GetIO().Fonts->SetTexID(0);
    }

    if (BackendData->PixelBuffer)
    {
        BackendData->PixelBuffer->Release();
        BackendData->PixelBuffer = NULL;
    }

    if (BackendData->VertexBuffer)
    {
        BackendData->VertexBuffer->Release();
        BackendData->VertexBuffer = NULL;
    }

    if (BackendData->BlendState)
    {
        BackendData->BlendState->Release();
        BackendData->BlendState = NULL;
    }

    if (BackendData->DepthStencilState)
    {
        BackendData->DepthStencilState->Release();
        BackendData->DepthStencilState = NULL;
    }

    if (BackendData->RasterizerState)
    {
        BackendData->RasterizerState->Release();
        BackendData->RasterizerState = NULL;
    }

    if (BackendData->PixelShader)
    {
        BackendData->PixelShader->Release();
        BackendData->PixelShader = NULL;
    }

    if (BackendData->VertexConstantBuffer)
    {
        BackendData->VertexConstantBuffer->Release();
        BackendData->VertexConstantBuffer = NULL;
    }

    if (BackendData->InputLayout)
    {
        BackendData->InputLayout->Release();
        BackendData->InputLayout = NULL;
    }

    if (BackendData->VertexShader)
    {
        BackendData->VertexShader->Release();
        BackendData->VertexShader = NULL;
    }
}

static void Dx11_SetupRenderState(ImDrawData *DrawData, ID3D11DeviceContext *D3dDeviceContext)
{
    dx11_backend_data *BackendData = Dx11_GetBackendData();

    D3D11_VIEWPORT D3dViewPort = {};
    D3dViewPort.Width = DrawData->DisplaySize.x;
    D3dViewPort.Height = DrawData->DisplaySize.y;
    D3dViewPort.MinDepth = 0.0f;
    D3dViewPort.MaxDepth = 1.0f;
    D3dViewPort.TopLeftX = 0;
    D3dViewPort.TopLeftY = 0;
    D3dDeviceContext->RSSetViewports(1, &D3dViewPort);

    // Setup shader and vertex buffers
    u32 Stride = sizeof(ImDrawVert);
    u32 Offset = 0;
    D3dDeviceContext->IASetInputLayout(BackendData->InputLayout);
    D3dDeviceContext->IASetVertexBuffers(0, 1, &BackendData->VertexBuffer, &Stride, &Offset);
    D3dDeviceContext->IASetIndexBuffer(BackendData->PixelBuffer, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    D3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3dDeviceContext->VSSetShader(BackendData->VertexShader, NULL, 0);
    D3dDeviceContext->VSSetConstantBuffers(0, 1, &BackendData->VertexConstantBuffer);
    D3dDeviceContext->PSSetShader(BackendData->PixelShader, NULL, 0);
    D3dDeviceContext->PSSetSamplers(0, 1, &BackendData->FontSampler);
    D3dDeviceContext->GSSetShader(NULL, NULL, 0);
    D3dDeviceContext->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
    D3dDeviceContext->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
    D3dDeviceContext->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

    // Setup blend state
    f32 BlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    D3dDeviceContext->OMSetBlendState(BackendData->BlendState, BlendFactor, 0xffffffff);
    D3dDeviceContext->OMSetDepthStencilState(BackendData->DepthStencilState, 0);
    D3dDeviceContext->RSSetState(BackendData->RasterizerState);
}

void Dx11_RenderDrawData(ImDrawData *DrawData)
{
    // Avoid rendering when minimized
    if ((DrawData->DisplaySize.x <= 0.0f) || (DrawData->DisplaySize.y <= 0.0f))
    {
        return;
    }

    dx11_backend_data *BackendData = Dx11_GetBackendData();
    ID3D11DeviceContext *D3dDeviceContext = BackendData->D3dDeviceContext;

    // Create and grow vertex/index buffers if needed
    if
    (
        !BackendData->VertexBuffer ||
        (BackendData->VertexBufferSize < DrawData->TotalVtxCount)
    )
    {
        if (BackendData->VertexBuffer)
        {
            BackendData->VertexBuffer->Release();
            BackendData->VertexBuffer = NULL;
        }
        BackendData->VertexBufferSize = DrawData->TotalVtxCount + 5000;

        D3D11_BUFFER_DESC BufferDescriptor = {};
        BufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescriptor.ByteWidth = BackendData->VertexBufferSize * sizeof(ImDrawVert);
        BufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        BufferDescriptor.MiscFlags = 0;
        if (BackendData->D3dDevice->CreateBuffer(&BufferDescriptor, NULL, &BackendData->VertexBuffer) < 0)
        {
            return;
        }
    }

    if
    (
        !BackendData->PixelBuffer ||
        (BackendData->IndexBufferSize < DrawData->TotalIdxCount)
    )
    {
        if (BackendData->PixelBuffer)
        {
            BackendData->PixelBuffer->Release();
            BackendData->PixelBuffer = NULL;
        }
        BackendData->IndexBufferSize = DrawData->TotalIdxCount + 10000;

        D3D11_BUFFER_DESC BufferDescriptor = {};
        BufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
        BufferDescriptor.ByteWidth = BackendData->IndexBufferSize * sizeof(ImDrawIdx);
        BufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
        BufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (BackendData->D3dDevice->CreateBuffer(&BufferDescriptor, NULL, &BackendData->PixelBuffer) < 0)
        {
            return;
        }
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    D3D11_MAPPED_SUBRESOURCE VertexResource, IndexResource;
    if (D3dDeviceContext->Map(BackendData->VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VertexResource) != S_OK)
    {
        return;
    }

    if (D3dDeviceContext->Map(BackendData->PixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &IndexResource) != S_OK)
    {
        return;
    }

    ImDrawVert *VertexDestination = (ImDrawVert *)VertexResource.pData;
    ImDrawIdx *IndexDestination = (ImDrawIdx *)IndexResource.pData;

    for (u32 CommandListIndex = 0; CommandListIndex < DrawData->CmdListsCount; CommandListIndex++)
    {
        ImDrawList *CommandList = DrawData->CmdLists[CommandListIndex];
        memcpy(VertexDestination, CommandList->VtxBuffer.Data, CommandList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(IndexDestination, CommandList->IdxBuffer.Data, CommandList->IdxBuffer.Size * sizeof(ImDrawIdx));
        VertexDestination += CommandList->VtxBuffer.Size;
        IndexDestination += CommandList->IdxBuffer.Size;
    }
    D3dDeviceContext->Unmap(BackendData->VertexBuffer, 0);
    D3dDeviceContext->Unmap(BackendData->PixelBuffer, 0);

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from DrawData->DisplayPos (top left) to DrawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    D3D11_MAPPED_SUBRESOURCE MappedResource;

    HRESULT Result = D3dDeviceContext->Map(BackendData->VertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (Result != S_OK)
    {
        return;
    }

    dx11_vertex_constant_buffer *ConstantBuffer = (dx11_vertex_constant_buffer *)MappedResource.pData;
    f32 L = DrawData->DisplayPos.x;
    f32 R = DrawData->DisplayPos.x + DrawData->DisplaySize.x;
    f32 T = DrawData->DisplayPos.y;
    f32 B = DrawData->DisplayPos.y + DrawData->DisplaySize.y;

    f32 MVP[4][4] =
    {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };
    memcpy(&ConstantBuffer->Mvp, MVP, sizeof(MVP));
    D3dDeviceContext->Unmap(BackendData->VertexConstantBuffer, 0);

    // Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
    dx11_backup_state OldDx11State = {};
    OldDx11State.ScissorsRectanglesCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    OldDx11State.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3dDeviceContext->RSGetScissorRects(&OldDx11State.ScissorsRectanglesCount, OldDx11State.ScissorsRectangles);
    D3dDeviceContext->RSGetViewports(&OldDx11State.ViewportsCount, OldDx11State.Viewports);
    D3dDeviceContext->RSGetState(&OldDx11State.RasterizerState);
    D3dDeviceContext->OMGetBlendState(&OldDx11State.BlendState, OldDx11State.BlendFactor, &OldDx11State.SampleMask);
    D3dDeviceContext->OMGetDepthStencilState(&OldDx11State.DepthStencilState, &OldDx11State.StencilReference);
    D3dDeviceContext->PSGetShaderResources(0, 1, &OldDx11State.PixelShaderResourceView);
    D3dDeviceContext->PSGetSamplers(0, 1, &OldDx11State.PixelShaderSampler);
    OldDx11State.PixelShaderInstanceCount = OldDx11State.VertexShaderInstanceCount = OldDx11State.GeometryShaderInstanceCount = 256;
    D3dDeviceContext->PSGetShader(&OldDx11State.PixelShader, OldDx11State.PixelShaderInstances, &OldDx11State.PixelShaderInstanceCount);
    D3dDeviceContext->VSGetShader(&OldDx11State.VertexShader, OldDx11State.VertexShaderInstances, &OldDx11State.VertexShaderInstanceCount);
    D3dDeviceContext->VSGetConstantBuffers(0, 1, &OldDx11State.VertexShaderConstantBuffer);
    D3dDeviceContext->GSGetShader(&OldDx11State.GeometryShader, OldDx11State.GeometryShaderInstances, &OldDx11State.GeometryShaderInstanceCount);
    D3dDeviceContext->IAGetPrimitiveTopology(&OldDx11State.PrimitiveTopology);
    D3dDeviceContext->IAGetIndexBuffer(&OldDx11State.IndexBuffer, &OldDx11State.IndexBufferFormat, &OldDx11State.IndexBufferOffset);
    D3dDeviceContext->IAGetVertexBuffers(0, 1, &OldDx11State.VertexBuffer, &OldDx11State.VertexBufferStride, &OldDx11State.VertexBufferOffset);
    D3dDeviceContext->IAGetInputLayout(&OldDx11State.InputLayout);

    // Setup desired DX state
    Dx11_SetupRenderState(DrawData, D3dDeviceContext);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own Offset into them)
    i32 GlobalIndexOffset = 0;
    i32 GlobalVertexOffset = 0;
    ImVec2 ClipOffset = DrawData->DisplayPos;

    for (i32 CommandListIndex = 0; CommandListIndex < DrawData->CmdListsCount; CommandListIndex++)
    {
        ImDrawList *CommandList = DrawData->CmdLists[CommandListIndex];
        for (i32 CommandIndex = 0; CommandIndex < CommandList->CmdBuffer.Size; CommandIndex++)
        {
            ImDrawCmd *Command = &CommandList->CmdBuffer[CommandIndex];
            if (Command->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (Command->UserCallback == ImDrawCallback_ResetRenderState)
                {
                    Dx11_SetupRenderState(DrawData, D3dDeviceContext);
                }
                else
                {
                    Command->UserCallback(CommandList, Command);
                }
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 ClipMinimum(Command->ClipRect.x - ClipOffset.x, Command->ClipRect.y - ClipOffset.y);
                ImVec2 ClipMaximum(Command->ClipRect.z - ClipOffset.x, Command->ClipRect.w - ClipOffset.y);
                if ((ClipMaximum.x <= ClipMinimum.x) || (ClipMaximum.y <= ClipMinimum.y))
                {
                    continue;
                }

                // Apply scissor/clipping rectangle
                D3D11_RECT Rectangle =
                {
                    (LONG)ClipMinimum.x,
                    (LONG)ClipMinimum.y,
                    (LONG)ClipMaximum.x,
                    (LONG)ClipMaximum.y
                };
                D3dDeviceContext->RSSetScissorRects(1, &Rectangle);

                // Bind texture, Draw
                ID3D11ShaderResourceView *TextureShaderResourceView = (ID3D11ShaderResourceView *)Command->GetTexID();
                D3dDeviceContext->PSSetShaderResources(0, 1, &TextureShaderResourceView);
                D3dDeviceContext->DrawIndexed(Command->ElemCount, Command->IdxOffset + GlobalIndexOffset, Command->VtxOffset + GlobalVertexOffset);
            }
        }
        GlobalIndexOffset += CommandList->IdxBuffer.Size;
        GlobalVertexOffset += CommandList->VtxBuffer.Size;
    }

    // Restore modified DX state
    D3dDeviceContext->RSSetScissorRects(OldDx11State.ScissorsRectanglesCount, OldDx11State.ScissorsRectangles);
    D3dDeviceContext->RSSetViewports(OldDx11State.ViewportsCount, OldDx11State.Viewports);
    D3dDeviceContext->RSSetState(OldDx11State.RasterizerState); if (OldDx11State.RasterizerState) OldDx11State.RasterizerState->Release();
    D3dDeviceContext->OMSetBlendState(OldDx11State.BlendState, OldDx11State.BlendFactor, OldDx11State.SampleMask); if (OldDx11State.BlendState) OldDx11State.BlendState->Release();
    D3dDeviceContext->OMSetDepthStencilState(OldDx11State.DepthStencilState, OldDx11State.StencilReference); if (OldDx11State.DepthStencilState) OldDx11State.DepthStencilState->Release();
    D3dDeviceContext->PSSetShaderResources(0, 1, &OldDx11State.PixelShaderResourceView); if (OldDx11State.PixelShaderResourceView) OldDx11State.PixelShaderResourceView->Release();
    D3dDeviceContext->PSSetSamplers(0, 1, &OldDx11State.PixelShaderSampler); if (OldDx11State.PixelShaderSampler) OldDx11State.PixelShaderSampler->Release();
    D3dDeviceContext->PSSetShader(OldDx11State.PixelShader, OldDx11State.PixelShaderInstances, OldDx11State.PixelShaderInstanceCount); if (OldDx11State.PixelShader) OldDx11State.PixelShader->Release();

    for (UINT i = 0; i < OldDx11State.PixelShaderInstanceCount; i++)
    {
        if (OldDx11State.PixelShaderInstances[i])
        {
            OldDx11State.PixelShaderInstances[i]->Release();
        }
    }

    D3dDeviceContext->VSSetShader(OldDx11State.VertexShader, OldDx11State.VertexShaderInstances, OldDx11State.VertexShaderInstanceCount); if (OldDx11State.VertexShader) OldDx11State.VertexShader->Release();
    D3dDeviceContext->VSSetConstantBuffers(0, 1, &OldDx11State.VertexShaderConstantBuffer); if (OldDx11State.VertexShaderConstantBuffer) OldDx11State.VertexShaderConstantBuffer->Release();
    D3dDeviceContext->GSSetShader(OldDx11State.GeometryShader, OldDx11State.GeometryShaderInstances, OldDx11State.GeometryShaderInstanceCount); if (OldDx11State.GeometryShader) OldDx11State.GeometryShader->Release();

    for (UINT i = 0; i < OldDx11State.VertexShaderInstanceCount; i++)
    {
        if (OldDx11State.VertexShaderInstances[i])
        {
            OldDx11State.VertexShaderInstances[i]->Release();
        }
    }

    D3dDeviceContext->IASetPrimitiveTopology(OldDx11State.PrimitiveTopology);
    D3dDeviceContext->IASetIndexBuffer(OldDx11State.IndexBuffer, OldDx11State.IndexBufferFormat, OldDx11State.IndexBufferOffset); if (OldDx11State.IndexBuffer) OldDx11State.IndexBuffer->Release();
    D3dDeviceContext->IASetVertexBuffers(0, 1, &OldDx11State.VertexBuffer, &OldDx11State.VertexBufferStride, &OldDx11State.VertexBufferOffset); if (OldDx11State.VertexBuffer) OldDx11State.VertexBuffer->Release();
    D3dDeviceContext->IASetInputLayout(OldDx11State.InputLayout); if (OldDx11State.InputLayout) OldDx11State.InputLayout->Release();
}

static void Dx11_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();
    dx11_backend_data *BackendData = Dx11_GetBackendData();
    u8 *Pixels;
    i32 Width;
    i32 Height;
    ImGuiIoInterface->Fonts->GetTexDataAsRGBA32(&Pixels, &Width, &Height);

    // Upload texture to graphics system
    D3D11_TEXTURE2D_DESC TextureDescriptor = {};
    TextureDescriptor.Width = Width;
    TextureDescriptor.Height = Height;
    TextureDescriptor.MipLevels = 1;
    TextureDescriptor.ArraySize = 1;
    TextureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDescriptor.SampleDesc.Count = 1;
    TextureDescriptor.Usage = D3D11_USAGE_DEFAULT;
    TextureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TextureDescriptor.CPUAccessFlags = 0;

    ID3D11Texture2D *Texture = NULL;

    D3D11_SUBRESOURCE_DATA SubResource;
    SubResource.pSysMem = Pixels;
    SubResource.SysMemPitch = TextureDescriptor.Width * 4;
    SubResource.SysMemSlicePitch = 0;
    BackendData->D3dDevice->CreateTexture2D(&TextureDescriptor, &SubResource, &Texture);
    Assert(Texture != NULL);

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDescriptor = {};
    ShaderResourceViewDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ShaderResourceViewDescriptor.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDescriptor.Texture2D.MipLevels = TextureDescriptor.MipLevels;
    ShaderResourceViewDescriptor.Texture2D.MostDetailedMip = 0;

    BackendData->D3dDevice->CreateShaderResourceView(Texture, &ShaderResourceViewDescriptor, &BackendData->FontTextureView);
    Texture->Release();

    // Store our identifier
    ImGuiIoInterface->Fonts->SetTexID((ImTextureID)BackendData->FontTextureView);

    // Create texture sampler
    // (Bilinear sampling is required by default. Set 'ImGuiIoInterface->Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = FALSE' to allow point/nearest sampling)
    D3D11_SAMPLER_DESC SamplerDescriptor = {};
    SamplerDescriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDescriptor.MipLODBias = 0.f;
    SamplerDescriptor.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamplerDescriptor.MinLOD = 0.f;
    SamplerDescriptor.MaxLOD = 0.f;
    BackendData->D3dDevice->CreateSamplerState(&SamplerDescriptor, &BackendData->FontSampler);
}

b32 Dx11_CreateDeviceObjects()
{
    dx11_backend_data *BackendData = Dx11_GetBackendData();

    if (!BackendData->D3dDevice)
    {
        return FALSE;
    }

    if (BackendData->FontSampler)
    {
        Dx11_InvalidateDeviceObjects();
    }

    // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
    // If you would like to use this DX11 sample code but remove this dependency you can:
    //  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
    //  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL.

    // ------------------------
    // Create the vertex shader
    // ------------------------
    ID3DBlob *VertexShaderBlob;
    const char *VertexShader =
        "cbuffer vertexBuffer : register(b0)"
        "{"
        "    float4x4 ProjectionMatrix;"
        "};"

        "struct VS_INPUT"
        "{"
        "    float2 pos : POSITION;"
        "    float4 col : COLOR0;"
        "    float2 uv  : TEXCOORD0;"
        "};"

        "struct PS_INPUT"
        "{"
        "    float4 pos : SV_POSITION;"
        "    float4 col : COLOR0;"
        "    float2 uv  : TEXCOORD0;"
        "};"

        "PS_INPUT main(VS_INPUT input)"
        "{"
        "    PS_INPUT output;"
        "    output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));"
        "    output.col = input.col;"
        "    output.uv  = input.uv;"
        "    return output;"
        "}";

    HRESULT Result = D3DCompile
    (
        VertexShader,
        strlen(VertexShader),
        NULL,
        NULL,
        NULL,
        "main",
        "vs_4_0",
        0,
        0,
        &VertexShaderBlob,
        NULL
    );

    if (FAILED(Result))
    {
        // NB: Pass ID3DBlob *pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
        return FALSE;
    }

    Result = BackendData->D3dDevice->CreateVertexShader
    (
        VertexShaderBlob->GetBufferPointer(),
        VertexShaderBlob->GetBufferSize(),
        NULL,
        &BackendData->VertexShader
    );

    if (Result != S_OK)
    {
        VertexShaderBlob->Release();
        return FALSE;
    }

    // -----------------------
    // Create the input layout
    // -----------------------
    D3D11_INPUT_ELEMENT_DESC LocalLayout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)OffsetOf(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)OffsetOf(ImDrawVert, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)OffsetOf(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Result = BackendData->D3dDevice->CreateInputLayout
    (
        LocalLayout,
        3,
        VertexShaderBlob->GetBufferPointer(),
        VertexShaderBlob->GetBufferSize(),
        &BackendData->InputLayout
    );

    if (Result != S_OK)
    {
        VertexShaderBlob->Release();
        return FALSE;
    }
    VertexShaderBlob->Release();

    // --------------------------
    // Create the constant buffer
    // --------------------------
    D3D11_BUFFER_DESC BufferDescriptor;
    BufferDescriptor.ByteWidth = sizeof(dx11_vertex_constant_buffer);
    BufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    BufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    BufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDescriptor.MiscFlags = 0;
    BackendData->D3dDevice->CreateBuffer(&BufferDescriptor, NULL, &BackendData->VertexConstantBuffer);

    // -----------------------
    // Create the pixel shader
    // -----------------------
    ID3DBlob *PixelShaderBlob;
    const char *pixelShader =
        "struct PS_INPUT"
        "{"
        "   float4 pos : SV_POSITION;"
        "   float4 col : COLOR0;"
        "   float2 uv  : TEXCOORD0;"
        "};"

        "sampler sampler0;"
        "Texture2D texture0;"

        "float4 main(PS_INPUT input) : SV_Target"
        "{"
        "   float4 out_col = input.col * texture0.Sample(sampler0, input.uv);"
        "   return out_col;"
        "}";

    Result = D3DCompile
    (
        pixelShader,
        strlen(pixelShader),
        NULL,
        NULL,
        NULL,
        "main",
        "ps_4_0",
        0,
        0,
        &PixelShaderBlob,
        NULL
    );

    if (FAILED(Result))
    {
        // NB: Pass ID3DBlob *pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
        return FALSE;
    }

    Result = BackendData->D3dDevice->CreatePixelShader
    (
        PixelShaderBlob->GetBufferPointer(),
        PixelShaderBlob->GetBufferSize(),
        NULL,
        &BackendData->PixelShader
    );

    if (Result != S_OK)
    {
        PixelShaderBlob->Release();
        return FALSE;
    }
    PixelShaderBlob->Release();

    // -------------------------
    // Create the blending setup
    // -------------------------
    D3D11_BLEND_DESC BlendDescriptor = {};
    BlendDescriptor.AlphaToCoverageEnable = FALSE;
    BlendDescriptor.RenderTarget[0].BlendEnable = TRUE;
    BlendDescriptor.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDescriptor.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDescriptor.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDescriptor.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDescriptor.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDescriptor.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDescriptor.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    BackendData->D3dDevice->CreateBlendState(&BlendDescriptor, &BackendData->BlendState);

    // ---------------------------
    // Create the rasterizer state
    // ---------------------------
    D3D11_RASTERIZER_DESC RasterizerDescriptor = {};
    RasterizerDescriptor.FillMode = D3D11_FILL_SOLID;
    RasterizerDescriptor.CullMode = D3D11_CULL_NONE;
    RasterizerDescriptor.ScissorEnable = TRUE;
    RasterizerDescriptor.DepthClipEnable = TRUE;
    BackendData->D3dDevice->CreateRasterizerState(&RasterizerDescriptor, &BackendData->RasterizerState);

    // Create depth-stencil State
    D3D11_DEPTH_STENCIL_DESC DepthStencilDescriptor = {};
    DepthStencilDescriptor.DepthEnable = FALSE;
    DepthStencilDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilDescriptor.DepthFunc = D3D11_COMPARISON_ALWAYS;
    DepthStencilDescriptor.StencilEnable = FALSE;
    DepthStencilDescriptor.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDescriptor.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDescriptor.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDescriptor.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    DepthStencilDescriptor.BackFace = DepthStencilDescriptor.FrontFace;
    BackendData->D3dDevice->CreateDepthStencilState(&DepthStencilDescriptor, &BackendData->DepthStencilState);

    Dx11_CreateFontsTexture();

    return TRUE;
}

b32 Dx11_Initialize(ID3D11Device *D3dDevice, ID3D11DeviceContext *D3dDeviceContext)
{
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();
    Assert(ImGuiIoInterface->BackendRendererUserData == NULL && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    dx11_backend_data *BackendData = (dx11_backend_data *)ImGui::MemAlloc(sizeof(dx11_backend_data));
    ZeroMemory(BackendData, sizeof(dx11_backend_data));
    BackendData->VertexBufferSize = 5000;
    BackendData->IndexBufferSize = 10000;

    ImGuiIoInterface->BackendRendererUserData = (void *)BackendData;
    ImGuiIoInterface->BackendRendererName = "dx11";
    // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    ImGuiIoInterface->BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // Get factory from D3dDevice
    IDXGIDevice *DxgiDevice = NULL;
    IDXGIAdapter *DxgiAdapter = NULL;
    IDXGIFactory *DxgiFactory = NULL;

    HRESULT Result = D3dDevice->QueryInterface(IID_PPV_ARGS(&DxgiDevice));
    if (Result == S_OK)
    {
        Result = DxgiDevice->GetParent(IID_PPV_ARGS(&DxgiAdapter));
        if (Result == S_OK)
        {
            Result = DxgiAdapter->GetParent(IID_PPV_ARGS(&DxgiFactory));
            if (Result == S_OK)
            {
                BackendData->D3dDevice = D3dDevice;
                BackendData->D3dDeviceContext = D3dDeviceContext;
                BackendData->DxgiFactory = DxgiFactory;
            }
        }
    }

    if (DxgiDevice)
    {
        DxgiDevice->Release();
    }

    if (DxgiAdapter)
    {
        DxgiAdapter->Release();
    }

    BackendData->D3dDevice->AddRef();
    BackendData->D3dDeviceContext->AddRef();

    return TRUE;
}

void Dx11_Shutdown()
{
    dx11_backend_data *BackendData = Dx11_GetBackendData();
    Assert(BackendData != NULL && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();

    Dx11_InvalidateDeviceObjects();

    if (BackendData->DxgiFactory)
    {
        BackendData->DxgiFactory->Release();
    }

    if (BackendData->D3dDevice)
    {
        BackendData->D3dDevice->Release();
    }

    if (BackendData->D3dDeviceContext)
    {
        BackendData->D3dDeviceContext->Release();
    }

    ImGuiIoInterface->BackendRendererName = NULL;
    ImGuiIoInterface->BackendRendererUserData = NULL;
    ImGuiIoInterface->BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    ImGui::MemFree(BackendData);
}

void Dx11_NewFrame()
{
    dx11_backend_data *BackendData = Dx11_GetBackendData();
    Assert(BackendData != NULL && "Did you call Dx11_Initialize()?");

    if (!BackendData->FontSampler)
    {
        Dx11_CreateDeviceObjects();
    }
}