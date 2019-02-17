#include "ColorGrading.func.hlsl"

cbuffer PerImageCB : register(b0)
{
    Texture2D gTexture;
    SamplerState gSampler;
};
cbuffer KuwaharaCB : register(b1)
{
    sampler2D _UserLut;
    float4 _UserLut_Params;
};

float4 calcColor(float2 uv)
{
    float4 color = gTexture.Sample(gSampler, uv);
    float3 colorGraded;
    colorGraded = ApplyLut2d(_UserLut, color.xyz, _UserLut_Params.xyz);
    color.xyz = lerp(color.xyz, colorGraded, _UserLut_Params.w);
    return color;
}

float4 main(in float2 texC : TEXCOORD) : SV_TARGET
{
    return calcColor(texC);
}
