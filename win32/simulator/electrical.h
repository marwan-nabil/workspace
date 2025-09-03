#pragma once

struct electric_point
{
    f32 CurrentCharge;
    f32 NextCharge;
};

struct conductor
{
    electric_point *A;
    electric_point *B;
    f32 Conductivity;
};

struct capacitor
{
    electric_point *A;
    electric_point *B;

    f32 CurrentChargePlateA;
    f32 NextChargePlateA;

    f32 CurrentChargePlateB;
    f32 NextChargePlateB;

    f32 Capacitance;
    f32 Factor0;
    f32 Factor1;
};