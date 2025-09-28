#include <windows.h>
#include <stdint.h>
#include <GL\GL.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\third_party\imgui\imgui.h"
#include "opengl2_backend.h"

HGLRC GlobalOpenGLRenderingContext;

b32 OpenGl2_CreateDevice(HWND Window, HDC *ResultDeviceContext)
{
    HDC DeviceContext = GetDC(Window);

    PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};
    PixelFormatDescriptor.nSize = sizeof(PixelFormatDescriptor);
    PixelFormatDescriptor.nVersion = 1;
    PixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    PixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    PixelFormatDescriptor.cColorBits = 32;

    i32 PixelFormat = ChoosePixelFormat(DeviceContext, &PixelFormatDescriptor);
    if (PixelFormat == 0)
    {
        return FALSE;
    }
    if (SetPixelFormat(DeviceContext, PixelFormat, &PixelFormatDescriptor) == FALSE)
    {
        return FALSE;
    }
    ReleaseDC(Window, DeviceContext);

    *ResultDeviceContext = GetDC(Window);
    if (!GlobalOpenGLRenderingContext)
    {
        GlobalOpenGLRenderingContext = wglCreateContext(*ResultDeviceContext);
    }

    return TRUE;
}

void OpenGl2_CleanupDeviceWGL(HWND Window, HDC DeviceContext)
{
    wglMakeCurrent(NULL, NULL);
    ReleaseDC(Window, DeviceContext);
}

// Backend data stored in ImGuiIoInterface.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
opengl2_backend_data *OpenGl2_GetBackendData()
{
    if (ImGui::GetCurrentContext())
    {
        return (opengl2_backend_data *)ImGui::GetIO().BackendRendererUserData;
    }
    else
    {
        return NULL;
    }
}

b32 OpenGl2_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();
    opengl2_backend_data *BackendData = OpenGl2_GetBackendData();

    u8 *Pixels;
    i32 Width, Height;
    ImGuiIoInterface->Fonts->GetTexDataAsRGBA32(&Pixels, &Width, &Height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    // (Bilinear sampling is required by default. Set 'ImGuiIoInterface->Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = FALSE' to allow point/nearest sampling)
    GLint LastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &LastTexture);
    glGenTextures(1, &BackendData->FontTexture);
    glBindTexture(GL_TEXTURE_2D, BackendData->FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);

    // Store our identifier
    ImGuiIoInterface->Fonts->SetTexID((ImTextureID)(intptr_t)BackendData->FontTexture);

    // Restore state
    glBindTexture(GL_TEXTURE_2D, LastTexture);

    return TRUE;
}

void OpenGl2_DestroyFontsTexture()
{
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();
    opengl2_backend_data *BackendData = OpenGl2_GetBackendData();
    if (BackendData->FontTexture)
    {
        glDeleteTextures(1, &BackendData->FontTexture);
        ImGuiIoInterface->Fonts->SetTexID(0);
        BackendData->FontTexture = 0;
    }
}

b32 OpenGl2_Initialize()
{
    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();
    Assert(ImGuiIoInterface->BackendRendererUserData == NULL && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    opengl2_backend_data *BackendData = (opengl2_backend_data *)ImGui::MemAlloc(sizeof(opengl2_backend_data));
    *BackendData = {};
    ImGuiIoInterface->BackendRendererUserData = (void *)BackendData;
    ImGuiIoInterface->BackendRendererName = "opengl2";

    return TRUE;
}

void OpenGl2_Shutdown()
{
    opengl2_backend_data *BackendData = OpenGl2_GetBackendData();
    Assert(BackendData != NULL && "No renderer backend to shutdown, or already shutdown?");

    ImGuiIO *ImGuiIoInterface = &ImGui::GetIO();

    OpenGl2_DestroyFontsTexture();

    ImGuiIoInterface->BackendRendererName = NULL;
    ImGuiIoInterface->BackendRendererUserData = NULL;

    ImGui::MemFree(BackendData);
}

void OpenGl2_NewFrame()
{
    opengl2_backend_data *BackendData = OpenGl2_GetBackendData();
    Assert(BackendData != NULL && "Did you call OpenGl2_Initialize()?");

    if (!BackendData->FontTexture)
    {
        OpenGl2_CreateFontsTexture();
    }
}

void OpenGl2_SetupRenderState(ImDrawData *DrawData, i32 FrameBufferWidth, i32 FrameBufferHeight)
{
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // In order to composite our output buffer we need to preserve alpha
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
    // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
    // (DO NOT MODIFY THIS FILE! Add the code in your calling function)
    //   GLint last_program;
    //   glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    //   glUseProgram(0);
    //   OpenGl2_RenderDrawData(...);
    //   glUseProgram(last_program)
    // There are potentially many more states you could need to clear/setup that we can't access from default headers.
    // e.g. glBindBuffer(GL_ARRAY_BUFFER, 0), glDisable(GL_TEXTURE_CUBE_MAP).

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from DrawData->DisplayPos (top left) to DrawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    glViewport(0, 0, (GLsizei)FrameBufferWidth, (GLsizei)FrameBufferHeight);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(DrawData->DisplayPos.x, DrawData->DisplayPos.x + DrawData->DisplaySize.x, DrawData->DisplayPos.y + DrawData->DisplaySize.y, DrawData->DisplayPos.y, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

// OpenGL2 Render function.
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly.
// This is in order to be able to run within an OpenGL engine that doesn't do so.
void OpenGl2_RenderDrawData(ImDrawData *DrawData)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    i32 FrameBufferWidth = (i32)(DrawData->DisplaySize.x * DrawData->FramebufferScale.x);
    i32 FrameBufferHeight = (i32)(DrawData->DisplaySize.y * DrawData->FramebufferScale.y);
    if (FrameBufferWidth == 0 || FrameBufferHeight == 0)
    {
        return;
    }

    // Backup GL state
    GLint LastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &LastTexture);
    GLint LastPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, LastPolygonMode);
    GLint LastViewport[4];
    glGetIntegerv(GL_VIEWPORT, LastViewport);
    GLint LastScissorBox[4];
    glGetIntegerv(GL_SCISSOR_BOX, LastScissorBox);
    GLint LastShadeModel;
    glGetIntegerv(GL_SHADE_MODEL, &LastShadeModel);
    GLint LastTextureEnvironmentMode;
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &LastTextureEnvironmentMode);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

    // Setup desired GL state
    OpenGl2_SetupRenderState(DrawData, FrameBufferWidth, FrameBufferHeight);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 ClipOffset = DrawData->DisplayPos; // (0,0) unless using multi-viewports
    ImVec2 ClipScale = DrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (i32 CommandListIndex = 0; CommandListIndex < DrawData->CmdListsCount; CommandListIndex++)
    {
        ImDrawList *CommandList = DrawData->CmdLists[CommandListIndex];
        ImDrawVert *VertexBuffer = CommandList->VtxBuffer.Data;
        ImDrawIdx *IndexBuffer = CommandList->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (GLvoid *)((char *)VertexBuffer + OffsetOf(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (GLvoid *)((char *)VertexBuffer + OffsetOf(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (GLvoid *)((char *)VertexBuffer + OffsetOf(ImDrawVert, col)));

        for (i32 CommandIndex = 0; CommandIndex < CommandList->CmdBuffer.Size; CommandIndex++)
        {
            ImDrawCmd *Command = &CommandList->CmdBuffer[CommandIndex];
            if (Command->UserCallback)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (Command->UserCallback == ImDrawCallback_ResetRenderState)
                {
                    OpenGl2_SetupRenderState(DrawData, FrameBufferWidth, FrameBufferHeight);
                }
                else
                {
                    Command->UserCallback(CommandList, Command);
                }
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 ClipMinimum = {(Command->ClipRect.x - ClipOffset.x) * ClipScale.x, (Command->ClipRect.y - ClipOffset.y) * ClipScale.y};
                ImVec2 ClipMaximum = {(Command->ClipRect.z - ClipOffset.x) * ClipScale.x, (Command->ClipRect.w - ClipOffset.y) * ClipScale.y};
                if ((ClipMaximum.x <= ClipMinimum.x) || (ClipMaximum.y <= ClipMinimum.y))
                {
                    continue;
                }

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                glScissor((i32)ClipMinimum.x, (i32)((float)FrameBufferHeight - ClipMaximum.y), (i32)(ClipMaximum.x - ClipMinimum.x), (i32)(ClipMaximum.y - ClipMinimum.y));

                // Bind texture, Draw
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)Command->GetTexID());
                glDrawElements(GL_TRIANGLES, (GLsizei)Command->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, IndexBuffer + Command->IdxOffset);
            }
        }
    }

    // Restore modified GL state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)LastTexture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glPolygonMode(GL_FRONT, (GLenum)LastPolygonMode[0]); glPolygonMode(GL_BACK, (GLenum)LastPolygonMode[1]);
    glViewport(LastViewport[0], LastViewport[1], (GLsizei)LastViewport[2], (GLsizei)LastViewport[3]);
    glScissor(LastScissorBox[0], LastScissorBox[1], (GLsizei)LastScissorBox[2], (GLsizei)LastScissorBox[3]);
    glShadeModel(LastShadeModel);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, LastTextureEnvironmentMode);
}