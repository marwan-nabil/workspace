#include <Windows.h>
#include <stdint.h>
#include <math.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\math\rectangle2.h"
#include "win32\shared\rasterizer\rasterizer.h"

#include "electrical.h"
#include "logic_gates.h"
#include "internal_types.h"
#include "simulation.h"

inline void
RenderKeypad(rendering_buffer *Buffer, simulation_state *SimulationState)
{
    v4 InactiveColor = V4(0.98f, 0.678f, 0.678f, 1.0f);
    v4 ActiveColor = V4(1.0f, 0, 0, 1.0f); // red

    if (SimulationState->Up)
    {
        DrawRectangle(Buffer, v2{100, 70}, v2 {130, 100}, ActiveColor);
    }
    else
    {
        DrawRectangle(Buffer, v2{100, 70}, v2 {130, 100}, InactiveColor);
    }

    if (SimulationState->Down)
    {
        DrawRectangle(Buffer, v2{100, 30}, v2 {130, 60}, ActiveColor);
    }
    else
    {
        DrawRectangle(Buffer, v2{100, 30}, v2 {130, 60}, InactiveColor);
    }

    if (SimulationState->Left)
    {
        DrawRectangle(Buffer, v2{60, 30}, v2 {90, 60}, ActiveColor);
    }
    else
    {
        DrawRectangle(Buffer, v2{60, 30}, v2 {90, 60}, InactiveColor);
    }

    if (SimulationState->Right)
    {
        DrawRectangle(Buffer, v2{140, 30}, v2 {170, 60}, ActiveColor);
    }
    else
    {
        DrawRectangle(Buffer, v2{140, 30}, v2 {170, 60}, InactiveColor);
    }
}

void
RenderSimulation(rendering_buffer *Buffer, simulation_state *SimulationState)
{
    v4 RectColor = V4(1.0f, 0.992f, 0.608f, 1.0f);
    DrawRectangle(Buffer, V2(0, 0), V2((f32)Buffer->Width, (f32)Buffer->Height), RectColor);

    RenderKeypad(Buffer, SimulationState);
}