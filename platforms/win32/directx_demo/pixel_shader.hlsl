struct shader_inputs
{
    float4 Color: COLOR;
};

float4 SimplePixelShader(shader_inputs Inputs): SV_TARGET
{
    return Inputs.Color;
}