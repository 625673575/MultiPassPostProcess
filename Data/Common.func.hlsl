#define UNITY_PI        3.14159265
#define HALF_MAX        65504.0
#define EPSILON         1.0e-4
#define UNITY_PI_2      (UNITY_PI * 2.0)

inline half Min3(half3 x)
{
    return min(x.x, min(x.y, x.z));
}
inline half Min3(half x, half y, half z)
{
    return min(x, min(y, z));
}

inline half Max3(half3 x)
{
    return max(x.x, max(x.y, x.z));
}
inline half Max3(half x, half y, half z)
{
    return max(x, max(y, z));
}

inline half Min4(half4 x)
{
    return min(x.x, min(x.y, min(x.z, x.w)));
}
inline half Min4(half x, half y, half z, half w)
{
    return min(x, min(y, min(z, w)));
}

inline half Max4(half4 x)
{
    return max(x.x, max(x.y, max(x.z, x.w)));
}
inline half Max4(half x, half y, half z, half w)
{
    return max(x, max(y, min(z, w)));
}

inline half Pow2(half x)
{
    return x * x;
}
inline half2 Pow2(half2 x)
{
    return x * x;
}
inline half3 Pow2(half3 x)
{
    return x * x;
}
inline half4 Pow2(half4 x)
{
    return x * x;
}

inline half Pow3(half x)
{
    return x * x * x;
}
inline half2 Pow3(half2 x)
{
    return x * x * x;
}
inline half3 Pow3(half3 x)
{
    return x * x * x;
}
inline half4 Pow3(half4 x)
{
    return x * x * x;
}

#ifndef UNITY_STANDARD_BRDF_INCLUDED
inline half Pow4(half x)
{
    return x * x * x * x;
}
inline half2 Pow4(half2 x)
{
    return x * x * x * x;
}
inline half3 Pow4(half3 x)
{
    return x * x * x * x;
}
inline half4 Pow4(half4 x)
{
    return x * x * x * x;
}
#endif

// Returns the largest floattor of v1 and v2
inline half2 MaxV(half2 v1, half2 v2)
{
    return dot(v1, v1) < dot(v2, v2) ? v2 : v1;
}
inline half3 MaxV(half3 v1, half3 v2)
{
    return dot(v1, v1) < dot(v2, v2) ? v2 : v1;
}
inline half4 MaxV(half4 v1, half4 v2)
{
    return dot(v1, v1) < dot(v2, v2) ? v2 : v1;
}

// Clamp HDR value within a safe range
inline half SafeHDR(half c)
{
    return min(c, HALF_MAX);
}
inline half2 SafeHDR(half2 c)
{
    return min(c, HALF_MAX);
}
inline half3 SafeHDR(half3 c)
{
    return min(c, HALF_MAX);
}
inline half4 SafeHDR(half4 c)
{
    return min(c, HALF_MAX);
}
// Tonemapper from http://gpuopen.com/optimized-reversible-tonemapper-for-resolve/
float4 FastToneMap(in float4 color)
{
    return float4(color.rgb * rcp(Max3(color.rgb) + 1.), color.a);
}

float4 FastToneMap(in float4 color, in float weight)
{
    return float4(color.rgb * rcp(weight * Max3(color.rgb) + 1.), color.a);
}

float4 FastToneUnmap(in float4 color)
{
    return float4(color.rgb * rcp(1. - Max3(color.rgb)), color.a);
}

// Interleaved gradient function from Jimenez 2014 http://goo.gl/eomGso
float GradientNoise(float2 uv)
{
    uv = floor(uv * float2(1920, 1080));
    float f = dot(float2(0.06711056, 0.00583715), uv);
    return frac(52.9829189 * frac(f));
}

float rand2d(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float rand(float n)
{
    return frac(sin(n) * 43758.5453123);
}

float noise(float p)
{
    float fl = floor(p);
    float fc = frac(p);
    return lerp(rand(fl), rand(fl + 1.0), fc);
}

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float map(float val, float amin, float amax, float bmin, float bmax)
{
    float n = (val - amin) / (amax - amin);
    float m = bmin + n * (bmax - bmin);
    return m;
}

float snoise(float p)
{
    return map(noise(p), 0.0, 1.0, -1.0, 1.0);
}

float threshold(float val, float cut)
{
    float v = clamp(abs(val) - cut, 0.0, 1.0);
    v = sign(val) * v;
    float scale = 1.0 / (1.0 - cut);
    return v * scale;
}
