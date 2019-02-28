__import Shading;
__import DefaultVS;
cbuffer DCB : register(b0)
{
    SamplerState gSampler;
    Texture2D gAlbedoTexture;
    float gTooniness;
}

float4 frag(VertexOut vOut) : SV_TARGET
{
    float4 albedo = gAlbedoTexture.Sample(gSampler, vOut.texC);
    return floor(albedo * gTooniness) / gTooniness;

}
