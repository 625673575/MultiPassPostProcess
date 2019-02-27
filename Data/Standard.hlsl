__import Shading;
__import DefaultVS;
cbuffer DCB : register(b0)
{
    bool gConstColor; //to supresss warning 
    SamplerState gSampler;
    Texture2D gAlbedoTexture;
    Texture2D gNormalTexture;
    Texture2D gMentalnessTexture;
    Texture2D gRoughnessTexture;
    TextureCube gSpecularTexture;
    TextureCube gIrradianceTexture;
    Texture2D gSpecularBRDF_LUT;
};

static const float PI = 3.14159265;
static const float Epsilon = 0.00001;

static const uint NumLights = 3;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

// GGX/Towbridge-
//Reitz normal
//distribution function.
//Uses Disney's
//reparametrization ofalpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
    uint width, height, levels;
    gSpecularTexture.GetDimensions(0, width, height, levels);
    return levels;
}

//Pixel shader

float4 frag(VertexOut vOut) : SV_Target
{
	// Sample input textures to get shading model params.
    float3 albedo = gAlbedoTexture.Sample(gSampler, vOut.texC).rgb;
    float metalness = gMentalnessTexture.Sample(gSampler, vOut.texC).r;
    float roughness = gRoughnessTexture.Sample(gSampler, vOut.texC).r;

	// Outgoing light direction (vector from world-space fragment position to the "eye").
    float3 Lo = normalize(gCamera.posW - vOut.posW);

	// Get current fragment's normal and transform to world space.
    float3 N = normalize(2.0 * gNormalTexture.Sample(gSampler, vOut.texC).rgb - 1.0);
    N = normalize(vOut.bitangentW * N);
	
	// Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
    float3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
    float3 F0 = lerp(Fdielectric, albedo, metalness);

	// Direct lighting calculation for analytical lights.
    float3 directLighting = 0.0;
    uint gLightsCount = 1;
    //gLights[0].dirW = float3(0.5, 0.14, 0.123);
    //gLightsCount[0].intensity = float3(3.0, 4.0, 5.0);
    for (uint i = 0; i < gLightsCount; ++i)
    {
        float3 Li = -float3(0.5, 0.14, 0.123); //gLights[i].dirW;
        float3 Lradiance = float3(3.0, 4.0, 5.0); //gLights[i].intensity;

		// Half-vector between Li and Lo.
        float3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(N, Li));
        float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
        float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
        float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
        float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
        float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
        float3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
        float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
        directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

	// Ambient lighting (IBL).
    float3 ambientLighting;
	{
		// Sample diffuse irradiance at normal direction.
        float3 irradiance = gIrradianceTexture.Sample(gSampler, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
        float3 F = fresnelSchlick(F0, cosLo);

		// Get diffuse contribution factor (as with direct lighting).
        float3 kd = lerp(1.0 - F, 0.0, metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
        float3 diffuseIBL = kd * albedo * irradiance;

		// Sample pre-filtered specular reflection environment at correct mipmap level.
        uint specularTextureLevels = querySpecularTextureLevels();
        float3 specularIrradiance = gSpecularTexture.SampleLevel(gSampler, Lr, roughness * specularTextureLevels).rgb;

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
        float2 specularBRDF = gSpecularBRDF_LUT.Sample(gSampler, float2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
        ambientLighting = diffuseIBL + specularIBL;
    }

	// Final fragment color.
    return float4(directLighting + ambientLighting, 1.0);
    //return gAlbedoTexture.Sample(gSampler, vOut.texC);
}
