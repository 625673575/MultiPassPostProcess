cbuffer PerImageCB : register(b0)
{
    Texture2D gTexture;
    SamplerState gSampler;
};
cbuffer KuwaharaCB : register(b1)
{
    float2 iResolution;
    int radius;
};

float4 calcColor(float2 uv)
{
    float2 gTexture_size = iResolution.xy;
    float n = float((radius + 1) * (radius + 1));

    float3 m[4] = { 0, 0, 0, 0 };
    float3 s[4] = { 0, 0, 0, 0 };

    for (int j = -radius; j <= 0; ++j)
    {
        for (int i = -radius; i <= 0; ++i)
        {
            float3 c = gTexture.Sample(gSampler, uv + float2(i, j) / gTexture_size).rgb;
            m[0] += c;
            s[0] += c * c;
        }
    }

    for (int j = -radius; j <= 0; ++j)
    {
        for (int i = 0; i <= radius; ++i)
        {
            float3 c = gTexture.Sample(gSampler, uv + float2(i, j) / gTexture_size).rgb;
            m[1] += c;
            s[1] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)
    {
        for (int i = 0; i <= radius; ++i)
        {
            float3 c = gTexture.Sample(gSampler, uv + float2(i, j) / gTexture_size).rgb;
            m[2] += c;
            s[2] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)
    {
        for (int i = -radius; i <= 0; ++i)
        {
            float3 c = gTexture.Sample(gSampler, uv + float2(i, j) / gTexture_size).rgb;
            m[3] += c;
            s[3] += c * c;
        }
    }


    float min_sigma2 = 1e+2;
    for (int k = 0; k < 4; ++k)
    {
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);

        float sigma2 = s[k].r + s[k].g + s[k].b;
        if (sigma2 < min_sigma2)
        {
            min_sigma2 = sigma2;
            return float4(m[k], 1.0);
        }
    }
    return float4(1.0);
}

float4 main(in float2 texC : TEXCOORD) : SV_TARGET
{
    return calcColor(texC);
}
