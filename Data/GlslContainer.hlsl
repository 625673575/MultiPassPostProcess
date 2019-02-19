typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
typedef float2x2 mat2;
typedef float3x3 mat3;
typedef float4x4 mat4;

#define mix lerp
#define fract frac
#define atan atan2

cbuffer ToyCB : register(b0)
{
    SamplerState gSampler;
    float2 iResolution;
    float iTime;
    float3 iMouse;
    float4 iColor;
    float4 iExtra;
};
Buffer<float4> iVec4Buffer;

float mod(float x, float y)
{
    return x - y * floor(x / y);
}
