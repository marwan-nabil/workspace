#pragma once

struct logic_gate_2_1
{
    electric_point *A;
    electric_point *B;
    electric_point *Q;
    f32 LogicThreshold;
    f32 LogicHigh;
    f32 LogicLow;
};