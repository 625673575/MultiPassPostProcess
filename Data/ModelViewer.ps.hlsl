__import Shading;
__import DefaultVS;
cbuffer DCB : register(b1)
{
    SamplerState gSampler;
    Texture2D gAlbedo;
    bool gConstColor;
    float4 gAmbient;
}
cbuffer SCB : register(b2)
{
    float4 gSt;
}
/*
struct VertexOut
{
    INTERPOLATION_MODE
    float3 normalW : NORMAL;
    INTERPOLATION_MODE
    float3 bitangentW : BITANGENT;
    INTERPOLATION_MODE
    float2 texC : TEXCRD;
    INTERPOLATION_MODE
    float3 posW : POSW;
    INTERPOLATION_MODE
    float3 colorV : COLOR;
    INTERPOLATION_MODE
    float4 prevPosH : PREVPOSH;
    INTERPOLATION_MODE
    float2 lightmapC : LIGHTMAPUV;
    float4 posH : SV_POSITION;
#ifdef _SINGLE_PASS_STEREO
    INTERPOLATION_MODE float4 rightEyePosS : NV_X_RIGHT;
    uint4 viewportMask : NV_VIEWPORT_MASK;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
#endif
};*/


VertexOut vert(VertexIn vIn)
{
    VertexOut vOut;
    float4x4 worldMat = getWorldMat(vIn);
    float4 posW = mul(vIn.pos, worldMat);
    vOut.posW = posW.xyz;
    vOut.posH = mul(posW, gCamera.viewProjMat);

#ifdef HAS_TEXCRD
    vOut.texC = vIn.texC;
#else
    vOut.texC = 0;
#endif

#ifdef HAS_COLORS
    vOut.colorV = vIn.color;
#else
    vOut.colorV = 0;
#endif

#ifdef HAS_NORMAL
    vOut.normalW = mul(vIn.normal, getWorldInvTransposeMat(vIn)).xyz;
#else
    vOut.normalW = 0;
#endif

#ifdef HAS_BITANGENT
    vOut.bitangentW = mul(vIn.bitangent, (float3x3)getWorldMat(vIn));
#else
    vOut.bitangentW = 0;
#endif

#ifdef HAS_LIGHTMAP_UV
    vOut.lightmapC = vIn.lightmapC;
#else
    vOut.lightmapC = 0;
#endif

#ifdef HAS_PREV_POSITION
    float4 prevPos = vIn.prevPos;
#else
    float4 prevPos = vIn.pos;
#endif
    float4 prevPosW = mul(prevPos, gPrevWorldMat[vIn.instanceID]);
    vOut.prevPosH = mul(prevPosW, gCamera.prevViewProjMat);

#ifdef _SINGLE_PASS_STEREO
    vOut.rightEyePosS = mul(posW, gCamera.rightEyeViewProjMat).x;
    vOut.viewportMask = 0x00000001;
    vOut.renderTargetIndex = 0;
#endif

    return vOut;
}

float4 frag(VertexOut vOut) : SV_TARGET
{
    if (gConstColor)
    {
        return float4(0, 1, 0, 0.35);
    }
    else
    {
        return gAlbedo.Sample(gSampler, vOut.texC) * gAmbient;
    }
}
