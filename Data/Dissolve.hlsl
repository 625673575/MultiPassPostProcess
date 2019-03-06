__import Shading;
__import DefaultVS;
cbuffer DCB : register(b0)
{
    float gDissolve;
    Texture2D gDissolveTexture;
}
float4 frag(VertexOut vOut) : SV_TARGET
{
    float4 dissolve = gDissolveTexture.Sample(gMaterial.resources.samplerState, vOut.texC);
    //if(dissolve.x<gDissolve)
    //    discard;
    clip(dissolve.x - gDissolve);
    ShadingData sd = prepareShadingData(vOut, gMaterial, gCamera.posW);
    float3 color = 0;

    for (uint l = 0; l < gLightsCount; l++)
    {
        color += evalMaterial(sd, gLights[l], 1).color.rgb;
    }

    return float4(color, 1);
}
