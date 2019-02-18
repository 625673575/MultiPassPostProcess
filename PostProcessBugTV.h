#pragma once
#include "PostProcessBase.h"
class PostProcessBugTV :
	public PostProcessBase
{
public:
	PostProcessBugTV()=default;
	~PostProcessBugTV()=default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pBugTV;
    ShaderVar  vBugTV;
    float dFrequency;
    std::map<std::string,bool> dShaderDefines;
    void updateDefines();
};

