#pragma once
#include "PostProcessBase.h"
class PostProcessFilmGrain :
    public PostProcessBase
{
public:
    PostProcessFilmGrain() = default;
    ~PostProcessFilmGrain() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pFilmGrain;
    ShaderVar  vFilmGrain;
    float      dFilmGrainStrength;

};
