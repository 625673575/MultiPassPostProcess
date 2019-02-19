#include "GlslContainer.hlsl"

float rand(float n)
{
    return fract(sin(n) * 43758.5453123);
}

vec2 rand2(in vec2 p)
{
    return fract(vec2(sin(p.x * 591.32 + p.y * 154.077), cos(p.x * 391.32 + p.y * 49.077)));
}

#define v4White vec4(1.0, 1.0, 1.0, 1.0)
#define v4Black vec4(0.0, 0.0, 0.0, 1.0)
#define v4Grey  vec4(0.5, 0.5, 0.5, 1.0)
#define v4DarkRed vec4(0.2, 0.1, 0.1, 1.0)
#define v4LightRed vec4(0.7, 0.1, 0.1, 1.0)

vec4 getRBCColor(float x)
{
    float poly = 0.0347197 * x +
        0.247408  * x*x +
        5.69306   * x*x*x -
        19.1026   * x*x*x*x +
        28.6689   * x*x*x*x*x -
        15.5415   * x*x*x*x*x*x + 0.2;
    return mix(v4DarkRed, v4LightRed, poly);
}

vec4 mainImage(in vec2 uv)
{
    float freq = 7.0;
    float gap = 1. / freq;
    vec2 param_pos = (uv + vec2(iTime / 5.0, 0.0));

    //param_pos = uv;

    vec2 closest_center = floor(param_pos * freq + vec2(0.5, 0.5)) / freq;

    float ballrad = (0.25 + 0.1 * rand(closest_center.x + 37.0 + closest_center.y)) * gap;
    float jitterrad = 0.5 * gap - ballrad;
    float far = (0.35 * gap - ballrad) / 0.1;

    float black_or_white = 0.5 + 0.5 * sin(
        2.0 * 3.14159 *
        (rand((closest_center.x + 347.0) * (closest_center.y + 129.0)) + iTime * 1.0));

    closest_center = closest_center + jitterrad * 1.0 *
        sin((iTime * 0.1 + rand2(closest_center)) * 6.28 +
            sin((iTime * 0.2 + rand2(closest_center.yx)) * 6.28) +
            sin((iTime * 0.5 + rand2(closest_center.xy * 93.0 + 127.0)) * 6.28)
            );

    float dist = length(param_pos - closest_center);

    float s = (dist * dist) / (ballrad * ballrad);

    vec4 color = mix(
        mix(getRBCColor(dist / ballrad), v4DarkRed, far),
        v4DarkRed,
        smoothstep(ballrad*0.95, ballrad*1.05, dist));

    return color;
}
float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    float4 color;
    color=mainImage(uv);
    return color;

}
