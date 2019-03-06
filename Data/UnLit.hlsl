__import ShaderCommon;
__import Shading;
cbuffer DCB : register(b0)
{
    SamplerState gSample;
    float4 gBaseColor;
    Texture2D gAlbedoTexture;
}
float4 frag(VertexOut vOut) : SV_TARGET
{
    float4 texColor = gAlbedoTexture.Sample(gSample, vOut.texC);
    return texColor*gBaseColor;
}
