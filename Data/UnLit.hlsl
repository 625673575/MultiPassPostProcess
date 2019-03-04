__import ShaderCommon;
__import Shading;
cbuffer DCB : register(b0)
{
    SamplerState gSample;
    float4 gBaseColor;
    Texture2D gAlbedo;
}
float4 frag(VertexOut vOut) : SV_TARGET
{
    float4 texColor = gAlbedo.Sample(gSample, vOut.texC);
    return texColor*gBaseColor;
}
