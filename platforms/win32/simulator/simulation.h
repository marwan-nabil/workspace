#pragma once

struct simulation_state
{
    b32 Up;
    b32 Down;
    b32 Left;
    b32 Right;
    b32 Shift;

    union
    {
        electric_point Points[6];
        struct
        {
            electric_point A0;
            electric_point A1;
            electric_point B0;
            electric_point B1;
            electric_point C0;
            electric_point C1;
        };
    };

    union
    {
        capacitor Capacitors[1];
        struct
        {
            capacitor Cap0;
        };
    };

    union
    {
        conductor Conductors[3];
        struct
        {
            conductor WireA;
            conductor WireB;
            conductor WireC;
        };
    };
};

void InitializeSimulationState(simulation_state *SimulationState);
void UpdateSimulation(f32 TimeDelta, user_input *UserInput, simulation_state *SimulationState);