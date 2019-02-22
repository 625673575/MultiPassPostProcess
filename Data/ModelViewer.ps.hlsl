__import Shading;
__import DefaultVS;
cbuffer PerFrameCB : register(b0)
{
    SamplerState gSampler;
    Texture2D gAlbedo;
    LightData gDirLight;
    LightData gPointLight;
    bool gConstColor;
    float3 gAmbient;
};

float4 main(VertexOut vOut) : SV_TARGET
{
    if (gConstColor)
    {
        return float4(gAmbient, 1);
    }
    else
    {
        float4 finalColor = gAlbedo.Sample(gSampler, vOut.texC);
        return finalColor * float4(gAmbient, 1.0);
    }
}
