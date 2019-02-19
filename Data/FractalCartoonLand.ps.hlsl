#include "GlslContainer.hlsl"
// "Fractal Cartoon" - former "DE edge detection" by Kali

// Cartoon-like effect using eiffies's edge detection found here: 
// https://www.shadertoy.com/view/4ss3WB
// I used my own method previously but was too complicated and not compiling everywhere.
// Thanks to the suggestion by WouterVanNifterick. 

// There are no lights and no AO, only color by normals and dark edges.

// update: Nyan Cat cameo, thanks to code from mu6k: https://www.shadertoy.com/view/4dXGWH


#ifndef CLUBBER
static const vec4 iMusic[4] = { vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0) };
static const float iTransition = 1.0;
#define CLUBBER_R 0.0
#define CLUBBER_G 0.0
#define CLUBBER_B 0.0
#define CLUBBER_A 0.0
#endif
// Clubber end

#define MUSICRAYS 1.2 * (CLUBBER_A)
#define MUSICWAVES CLUBBER_R
#define MUSICMOD1 CLUBBER_G / 4.4
#define MUSICMOD2 CLUBBER_B / 6.6
#define MUSICSUNSIZE length(vec2(CLUBBER_R, CLUBBER_G ))
#define MUSICSUNSPIN length(vec2(CLUBBER_B, CLUBBER_A ))

//#define SHOWONLYEDGES
#define WAVES

#define RAY_STEPS 150

#define BRIGHTNESS 1.2
#define GAMMA 1.4
#define SATURATION .65


#define detail .001
#define t iTime*.5


static const vec3 origin = vec3(-1., .7, 0.);
static float det = 0.0;


// 2D rotation function
mat2 rot(float a)
{
    return mat2(cos(a), sin(a), -sin(a), cos(a));
}

// "Amazing Surface" fractal
vec4 formula(vec4 p)
{
    p.xz = abs(p.xz + 1.) - abs(p.xz - 1.2 + MUSICMOD1) - p.xz;
    p.y -= .25;
    mat2 v = rot(radians(35.));
    p.xy =mul(p.xy, v);
    p = p * 2. / clamp(dot(p.xyz, p.xyz), .2, min(1.05, 0.95 + MUSICMOD2));
    return p;
}

// Distance function
float de(vec3 pos)
{
#ifdef WAVES
    pos.y += sin(pos.z - t * 6. + MUSICWAVES) * .15; //waves!
#endif
    float hid = 0.;
    vec3 tpos = pos;
    tpos.z = abs(3. - mod(tpos.z, 6.));
    vec4 p = vec4(tpos, 1.);
    for (int i = 0; i < 4; i++)
    {
        p = formula(p);
    }
    float fr = (length(max(vec2(0.), p.yz - 1.5)) - 1.) / p.w;
    float ro = max(abs(pos.x + 1.) - .3, pos.y - .35);
    ro = max(ro, -max(abs(pos.x + 1.) - .1, pos.y - .5));
    pos.z = abs(.25 - mod(pos.z - 2. * length(iMusic[2].xz), .5));
    ro = max(ro, -max(abs(pos.z) - .2, pos.y - .3));
    ro = max(ro, -max(abs(pos.z) - .01, -pos.y + .32));
    float d = min(fr, ro);
    return d;
}


// Camera path
vec3 path(float ti)
{
    ti *= 1.5;
    vec3 p = vec3(sin(ti), (1. - sin(ti * 2.)) * .5, -ti * 5.) * .33;
    return p;
}

// Calc normals, and here is edge detection, set to variable "edge"

static float edge = 0.;
vec3 normal(vec3 p)
{
    vec3 e = vec3(0.0, det * 5., 0.0);

    float d1 = de(p - e.yxx), d2 = de(p + e.yxx);
    float d3 = de(p - e.xyx), d4 = de(p + e.xyx);
    float d5 = de(p - e.xxy), d6 = de(p + e.xxy);
    float d = de(p);
    edge = abs(d - 0.5 * (d2 + d1)) + abs(d - 0.5 * (d4 + d3)) + abs(d - 0.5 * (d6 + d5)); //edge finder
    edge = min(1., pow(edge, .55) * 15.);
    return normalize(vec3(d1 - d2, d3 - d4, d5 - d6));
}
// Raymarching and 2D graphics

vec3 raymarch(in vec3 from, in vec3 dir)
{
    edge = 0.;
    vec3 p, norm;
    float d = 100.;
    float totdist = 0.;
    for (int i = 0; i < RAY_STEPS; i++)
    {
        if (d > det && totdist < 25.0)
        {
            p = from + totdist * dir;
            d = de(p);
            det = detail * exp(.13 * totdist);
            totdist += d;
        }
        else
        {
            break;
        }
    }
    vec3 col = vec3(0.);
    p -= (det - d) * dir;
    norm = normal(p);
    
#ifdef SHOWONLYEDGES
	col=1.-vec3(edge); // show wireframe version
#else
    col = mix(vec3(edge), (1. - abs(norm)) * max(0., 1. - edge * .8), iTransition); // set normal as color with dark edges
#endif		
    totdist = clamp(totdist, 0., 26.);
    dir.y -= .02;
    float sunsize = 7. - MUSICSUNSIZE; // responsive sun size
    float an = atan(dir.x, dir.y) + iTime * 1.5 + MUSICSUNSPIN; // angle for drawing and rotating sun
    float s = pow(clamp(1.0 - length(dir.xy) * sunsize - abs(.2 - mod(an, .4)), 0., 1.), .1); // sun
    float sb = pow(clamp(1.0 - length(dir.xy) * (sunsize - .2) - abs(.2 - mod(an, .4)), 0., 1.), .1); // sun border
    float sg = pow(clamp(1.0 - length(dir.xy) * (sunsize - 4.5 - MUSICRAYS) - .5 * abs(.2 - mod(an, .4)), 0., 1.), 3.); // sun rays
    float y = mix(.45, 1.2, pow(smoothstep(0., 1., .75 - dir.y), 2.)) * (1. - sb * .5); // gradient sky
	
	// set up background with sky and sun
    vec3 backg = vec3(0.5, 0., 1.) * iTransition * ((1. - s) * (1. - sg) * y + (1. - sb) * sg * vec3(1., .8, 0.15) * 3.);
    backg += min(vec3(1., .9, .1) * s, vec3(iTransition));
    backg = max(backg, sg * vec3(1., .9, .5));
	
    col = mix(vec3(1., .9, .3) * iTransition, col, exp(-.004 * totdist * totdist)); // distant fading to sun color
    if (totdist > 25.)
        col = backg; // hit background
    col = pow(col, vec3(GAMMA)) * BRIGHTNESS;
    col = mix(vec3(length(col)), col, SATURATION);
#ifdef SHOWONLYEDGES
	col=1.-vec3(length(col));
#else
    col *= mix(vec3(length(col)), vec3(1., .9, .85), iTransition);
#endif
    return col;
}

// get camera position
vec3 move(inout vec3 dir)
{
    vec3 go = path(t);
    vec3 adv = path(t + .7);
    float hd = de(adv);
    vec3 advec = normalize(adv - go);
    float an = adv.x - go.x;
    an *= min(1., abs(adv.z - go.z)) * sign(adv.z - go.z) * .7;
    dir.xy = mul(dir.xy, mat2(cos(an), sin(an), -sin(an), cos(an)));
    an = advec.y * 1.7;
    dir.yz = mul(dir.yz, mat2(cos(an), sin(an), -sin(an), cos(an)));
    an = atan(advec.x, advec.z);
    dir.xz = mul(dir.xz, mat2(cos(an), sin(an), -sin(an), cos(an)));
    return go;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy * 2. - 1.;
    vec2 oriuv = uv;
    uv.y *= iResolution.y / iResolution.x;
    vec2 mouse = (iMouse.xy / iResolution.xy - .5) * 3.;
    if (iMouse.z < 1.)
        mouse = vec2(0., -0.05);
    float fov = .9 - max(0., .7 - iTime * .3);
    vec3 dir = normalize(vec3(uv * fov, 1.));
    vec3 from = origin + move(dir);
    vec3 color = raymarch(from, dir);
#ifdef BORDER
	color=mix(vec3(0.),color,pow(max(0.,.95-length(oriuv*oriuv*oriuv*vec2(1.05,1.1))),.3));
#endif
    fragColor = vec4(color, 1.);
}
float4 main(float2 uv : TEXCOORD) : SV_TARGET0
{
    float4 color;
    mainImage(color,uv*iResolution);
    return color;

}
