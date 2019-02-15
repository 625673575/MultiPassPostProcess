cbuffer PerImageCB : register(b0)
{
    Texture2D gTexture;
    SamplerState gSampler;
};
//假定mpSharpVars是ShaderVariable
//cbuffer 直接通过 mpSharpVars["SharpParamCB"]["contrast"] = 1.0f;进行赋值
cbuffer SharpParamCB : register(b1)
{
    float contrast;
    bool invocation;
};
struct LightCB
{
    float3 color;
    float intensity;
};

StructuredBuffer<LightCB> gLight;
//Buffer是连续的内存空间段,hlsl中需要指定下标
//mpSharpPowerBuffer = TypedBuffer<float>::create(2);创建TypedBuffer
//mpSharpVars->setTypedBuffer("gSaturation", mpSharpPowerBuffer);指定Buffer注册的名称
//渲染时       mpSharpPowerBuffer[0]= 2.0f;
//上传到GPU    mpSharpPowerBuffer->uploadToGPU();//不调用好像也行
Buffer<float> gSaturation;
RWByteAddressBuffer gInvocationBuffer; //计算单次ps被执行了多少次,等于像素点的个数

static const float3 gLuminance = float3(0.2126, 0.7152, 0.0722);
float4 calcColor(float2 texC)
{
    float4 fragColor = gTexture.Sample(gSampler, texC);
    fragColor.rgb = pow(fragColor.rgb, contrast);
    fragColor.rgb = gSaturation[0] * dot(fragColor.rgb, gLuminance).xxx + gSaturation[1] * fragColor.rgb;

    uint numLights = 0;
    uint stride;
    gLight.GetDimensions(numLights, stride);
    if (numLights > 0)
    {
        float each_piece = 1.0 / numLights;
        float3 lightMixColor = float3(0, 0, 0);
        for (uint i = 0; i < numLights; i++)
        {
            lightMixColor += each_piece * gLight[i].color * gLight[i].intensity;
        }
        fragColor.rgb *= lightMixColor;
    }
    if (invocation)
    {
        gInvocationBuffer.InterlockedAdd(0, 1, stride);
        fragColor *= saturate(100000.0 / float(stride));
    }
    return fragColor;
}

float4 main(in float2 texC : TEXCOORD) : SV_TARGET
{
    return calcColor(texC);
}
