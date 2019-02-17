#pragma once
#include "PostProcessBase.h"
class PostProcessMixer :
    public PostProcessBase
{
public:
    PostProcessMixer() = default;
    ~PostProcessMixer() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pMixer;
    ShaderVar  vMixer;
    int dImageCount;
    std::vector<float> dRatio;
    Texture::SharedPtr dImageArray;
    TypedBuffer<float>::SharedPtr pRatioBuffer;
};

