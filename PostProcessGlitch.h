#pragma once
#include "PostProcessBase.h"
class PostProcessGlitch :
    public PostProcessBase
{
public:
    PostProcessGlitch();
    ~PostProcessGlitch();
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
   ShaderPass pGlitch;
   ShaderVar vGlitch;
   float dGlitchStrength;

};

