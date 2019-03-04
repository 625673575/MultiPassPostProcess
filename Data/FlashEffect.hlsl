__import Shading;
__import DefaultVS;
cbuffer DCB : register(b0)
{
    SamplerState gSampler;
    Texture2D gAlbedo;
    Texture2D gFlashTexture;
    float4 gFlashColor;
    float4 gFlashFactor;
    float gFlashStrength;
    float3 gLightDir;
    float gTime;
}
float4 frag(VertexOut vOut) : SV_TARGET
{
    float4 albedo = gAlbedo.Sample(gSampler, vOut.texC);
    half3 normal = normalize(vOut.normalW);
    half3 light = normalize(gLightDir);
    float diff = max(0, dot(normal, light));
    half2 flashuv = vOut.posW.xy * gFlashFactor.zw + gFlashFactor.xy * gTime;
    float4 flash = gFlashTexture.Sample(gSampler,flashuv) * gFlashColor * gFlashStrength;
    float4 c;
    c.rgb = diff * albedo.rgb + flash.rgb;
    c.a = 1;
    return c;
}
