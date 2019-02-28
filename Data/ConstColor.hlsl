__import Shading;
__import DefaultVS;
cbuffer DCB : register(b0)
{
    //SamplerState gSampler;
    float4 gColor;
}
float4 frag(VertexOut vOut) : SV_TARGET
{
    //float4 baseColor = gMaterial.resources.baseColor.Sample(gMaterial.resources.samplerState, vOut.texC);
    return gColor;
}
