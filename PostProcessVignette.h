#pragma once
#include "PostProcessBase.h"
class PostProcessVignette :
	public PostProcessBase
{
public:
	PostProcessVignette()=default;
	~PostProcessVignette()=default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pVignette;
    ShaderVar  vVignette;

    vec2 dVignette_Center;
    vec4 dVignette_Settings; //new Vector4(settings.intensity * 3f, settings.smoothness * 5f, roundness, settings.rounded ? 1f : 0f));
    vec3 dVignette_Color;
};

