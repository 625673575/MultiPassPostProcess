cbuffer PerImageCB : register(b0)
{
    Texture2DArray gTexture;
    SamplerState gSampler;
};
Buffer<float> gRatio;
float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    uint w, h, d;
    gTexture.GetDimensions(w, h, d);
    float4 color = float4(0.0);
    for (int i = 0; i < d; i++)
    {
        color+=gTexture.Sample(gSampler, float3(uv.x, uv.y, i))*gRatio[i];
    }
    color = saturate(color);
    return color;
}
