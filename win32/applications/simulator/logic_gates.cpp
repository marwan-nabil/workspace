#include <Windows.h>
#include <stdint.h>
#include <math.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
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

void AndGateUpdate(logic_gate_2_1 *And)
{
    if
    (
        (And->B->CurrentCharge > And->LogicThreshold) &&
        (And->A->CurrentCharge > And->LogicThreshold)
    )
    {
        And->Q->NextCharge = And->LogicHigh;
    }
    else
    {
        And->Q->NextCharge = And->LogicLow;
    }
}

void OrGateUpdate(logic_gate_2_1 *Or)
{
    if
    (
        (Or->B->CurrentCharge > Or->LogicThreshold) ||
        (Or->A->CurrentCharge > Or->LogicThreshold)
    )
    {
        Or->Q->NextCharge = Or->LogicHigh;
    }
    else
    {
        Or->Q->NextCharge = Or->LogicLow;
    }
}

void XorGateUpdate(logic_gate_2_1 *Xor)
{
    if
    (
        (Xor->B->CurrentCharge > Xor->LogicThreshold) ^
        (Xor->A->CurrentCharge > Xor->LogicThreshold)
    )
    {
        Xor->Q->NextCharge = Xor->LogicHigh;
    }
    else
    {
        Xor->Q->NextCharge = Xor->LogicLow;
    }
}