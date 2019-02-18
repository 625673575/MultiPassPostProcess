#pragma once
#include "PostProcessBase.h"
class ShaderToyImplementation :
    public PostProcessBase
{
public:
    ShaderToyImplementation(const std::string& hlsl);
    ~ShaderToyImplementation() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    ShaderPass pToy;
    ShaderVar vToy;
};

