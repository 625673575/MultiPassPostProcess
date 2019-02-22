cbuffer PerImageCB : register(b0)
{
    Texture2D gTexture;
    Texture2D gLut;
    SamplerState gSampler;
    float fLUT_AmountChroma; //0-1
    float fLUT_AmountLuma; //0-1
};
static const float fLUT_TileSizeXY = 32;
static const float fLUT_TileAmount = 1024 / 32;

float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    float4 color = gTexture.Sample(gSampler, uv);
    float2 texelsize = 1.0 / fLUT_TileSizeXY;
    texelsize.x /= fLUT_TileAmount;

    float3 lutcoord = float3((color.xy * fLUT_TileSizeXY - color.xy + 0.5) * texelsize.xy, color.z * fLUT_TileSizeXY - color.z);
    float lerpfact = frac(lutcoord.z);
    lutcoord.x += (lutcoord.z - lerpfact) * texelsize.y;

    float3 lutcolor = lerp(gLut.Sample(gSampler, lutcoord.xy).xyz, gLut.Sample(gSampler, float2(lutcoord.x + texelsize.y, lutcoord.y)).xyz, lerpfact);

    color.xyz = lerp(normalize(color.xyz), normalize(lutcolor.xyz), fLUT_AmountChroma) *
	            lerp(length(color.xyz), length(lutcolor.xyz), fLUT_AmountLuma);

    return color;
}
