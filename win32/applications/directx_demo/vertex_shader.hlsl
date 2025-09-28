cbuffer per_application_constants: register(b0)
{
    matrix ProjectionMatrix;
}

cbuffer per_frame_constants: register(b1)
{
    matrix ViewMatrix;
}

cbuffer per_object_constants: register(b2)
{
    matrix WorldMatrix;
}

struct shader_inputs
{
    float3 Position: POSITION;
    float3 Color: COLOR;
};

struct shader_outputs
{
    float4 Color: COLOR;
    float4 Position: SV_POSITION;
};

shader_outputs SimpleVertexShader(shader_inputs Inputs)
{
    shader_outputs Outputs;
    matrix ModelViewProjection = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));
    Outputs.Position = mul(ModelViewProjection, float4(Inputs.Position, 1.0f));
    Outputs.Color = float4(Inputs.Color, 1.0f);
    return Outputs;
}