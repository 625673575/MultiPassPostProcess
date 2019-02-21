__import Shading;
__import DefaultVS;
cbuffer PerFrameCB : register(b0)
{
    float3 color;
};

float4 main(VertexOut vOut) : SV_TARGET
{
    return float4(color, 1.0);
}
