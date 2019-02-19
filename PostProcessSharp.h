#pragma once
#include "PostProcessBase.h"
class PostProcessSharp :
    public PostProcessBase
{
    struct SLight {
        glm::vec3 color;
        float intensity;
        SLight() {
            color = glm::vec3(1.0f);
            intensity = 1.0f;
        }
    };
    static const int LIGHT_COUNT = 10;
public:
    PostProcessSharp() = default;
    ~PostProcessSharp() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pSharp;
    ShaderVar  vSharp;

    float mdSharpContrast;
    vec2 mdSharpSaturation;
    bool mdCountPixelShaderInvocations = false;
    int32_t mSharpLightNum;
    std::vector<SLight> mdSharpLight;

    TypedBuffer<float>::SharedPtr mpSharpSaturationBuffer;
    StructuredBuffer::SharedPtr mpSharpLightBuffer;
    Buffer::SharedPtr mpInvocationsBuffer;
};

