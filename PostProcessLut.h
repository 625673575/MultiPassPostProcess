#pragma once
#include "PostProcessBase.h"
class PostProcessLut :
	public PostProcessBase
{
public:
	PostProcessLut()=default;
	~PostProcessLut()=default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pLut;
    ShaderVar  vLut;
    Texture::SharedPtr dLutTexture;
};

