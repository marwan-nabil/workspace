#pragma once

#include <windows.h>
#include <GL\GL.h>
#include "portable\shared\base_types.h"
#include "win32\third_party\imgui\imgui.h"

struct opengl2_backend_data
{
    GLuint FontTexture;
};

extern HGLRC GlobalOpenGLRenderingContext;

b32 OpenGl2_CreateDevice(HWND Window, HDC *ResultDeviceContext);
void OpenGl2_CleanupDeviceWGL(HWND Window, HDC DeviceContext);
b32 OpenGl2_Initialize();
void OpenGl2_NewFrame();
void OpenGl2_RenderDrawData(ImDrawData *DrawData);
void OpenGl2_Shutdown();