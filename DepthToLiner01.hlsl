//_ZBufferParams	float4	Used to linearize Z buffer values.x is(1 - far / near), y is(far / near), z is(x / far) and w is(y / far).
static const float far = 1000.0f;
static const float near = 0.1f;
static const float x = (1 - far / near);
static const float y = (far / near);
static const float z = (x / far);
static const float w = (y / far);
static const float4 _ZBufferParams = float4(x, y, z, w);
// Z buffer to linear 0..1 depth
inline float Linear01Depth(float z)
{
    return 1.0 / (_ZBufferParams.x * z + _ZBufferParams.y);
}
// Z buffer to linear depth
inline float LinearEyeDepth(float z)
{
    return 1.0 / (_ZBufferParams.z * z + _ZBufferParams.w);
}
// Encoding/decoding [0..1) floats into 8 bit/channel RGBA. Note that 1.0 will not be encoded properly.
inline float4 EncodeFloatRGBA(float v)
{
    float4 kEncodeMul = float4(1.0, 255.0, 65025.0, 16581375.0);
    float kEncodeBit = 1.0 / 255.0;
    float4 enc = kEncodeMul * v;
    enc = frac(enc);
    enc -= enc.yzww * kEncodeBit;
    return enc;
}
inline float DecodeFloatRGBA(float4 enc)
{
    float4 kDecodeDot = float4(1.0, 1 / 255.0, 1 / 65025.0, 1 / 16581375.0);
    return dot(enc, kDecodeDot);
}
//D32float only has one channel
Texture2D<float> gInput;
//Output four channel
RWTexture2D<float4> gOutput;

groupshared float colors[16][16];
groupshared float4 pixelated;

[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadId)
{
    // Calculate the start position of the block    
    uint3 resDim;
    gOutput.GetDimensions(resDim.x, resDim.y);

    uint2 posStart = groupId.xy * 16;
    uint2 crd = posStart + groupThreadId.xy;

    // Fetch all of the data into the shared local memory
    colors[groupThreadId.x][groupThreadId.y] = gInput[crd];

    float c = Linear01Depth(colors[groupThreadId.x][groupThreadId.y]);
    gOutput[crd] = float4(c, c, c, 1);
}
