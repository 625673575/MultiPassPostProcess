#include "GlslContainer.hlsl"
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

cbuffer ChannelCB : register(b1)
{
    Texture2D iChannel0;
}
void mainImage(out vec4 fragColor, in vec2 uv)
{
    vec3 col = iChannel0.Sample(gSampler, uv).xyz;
    // vignetting	
    col *= 0.5 + 0.5 * pow(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.1);

    fragColor = vec4(col, 1.0);
}

float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    float4 color;
    mainImage(color, uv);
    return color;

}
