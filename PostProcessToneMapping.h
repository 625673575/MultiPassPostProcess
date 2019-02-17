#pragma once
#include "PostProcessBase.h"
class PostProcessToneMapping :
	public PostProcessBase
{
public:
	PostProcessToneMapping()=default;
	~PostProcessToneMapping()=default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pTone;
    ShaderVar  vTone;
    float      dTone;;
};

