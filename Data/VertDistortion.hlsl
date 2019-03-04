__import ShaderCommon;
__import Shading;
cbuffer DCB : register(b0)
{
    float gTime;
    float3 gDistortionDir;
}
VertexOut vert(VertexIn vIn)
{
    VertexOut vOut;
    float4x4 worldMat = getWorldMat(vIn);
    float sinTime = sin(gTime);
    float3 modelPos = vIn.pos.xyz;
    modelPos = modelPos + sinTime * gDistortionDir;
    float4 posW = mul(float4(modelPos, vIn.pos.w), worldMat);
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
    ShadingData sd = prepareShadingData(vOut, gMaterial, gCamera.posW);

    float3 color = 0;

    for (uint l = 0; l < gLightsCount; l++)
    {
        color += evalMaterial(sd, gLights[l], 1).color.rgb;
    }

    return float4(color, 1.f);
}
