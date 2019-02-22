#pragma once
#include "PostProcessBase.h"
class PostProcessMotionBlur :
	public PostProcessBase
{
public:
	PostProcessMotionBlur()=default;
	~PostProcessMotionBlur() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pMotionBlur;
    ShaderVar  vMotionBlur;
    int dPreFrameIndex;
    float dBlurFactor;
};

