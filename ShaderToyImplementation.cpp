#include "ShaderToyImplementation.h"

ShaderToyImplementation::ShaderToyImplementation(const std::string & hlsl)
{
    name = hlsl;
}

void ShaderToyImplementation::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    SET_PROGRAM_VARS(Toy, name + ".ps.hlsl");
}

void ShaderToyImplementation::execute()
{
    pContext->setGraphicsVars(vToy);
    //vToy->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
    vToy["ToyCB"]["iTime"] = pSample->getCurrentTime();
    vToy["ToyCB"]["iResolution"] = getResolution();
    vToy["ToyCB"]["iMouse"] = getMousePos();
    pToy->execute(pContext);
}

void ShaderToyImplementation::gui()
{
}
