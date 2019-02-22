cbuffer PerImageCB : register(b0)
{
    Texture2D gTexture[2];
    SamplerState gSampler;
    uint gTextureCount;
    float gBlurFactor;
};
float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    float4 col1 = gTexture[0].Sample(gSampler, uv);
    float4 col2 = gTexture[1].Sample(gSampler, uv);
    return lerp(col1, col2, gBlurFactor);
}
