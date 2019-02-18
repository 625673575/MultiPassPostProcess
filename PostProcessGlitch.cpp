#include "PostProcessGlitch.h"

void PostProcessGlitch::execute()
{
    pContext->setGraphicsVars(vGlitch);
    vGlitch->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
    vGlitch["GlitchCB"]["iGlobalTime"] = pSample->getCurrentTime();
    vGlitch["GlitchCB"]["strength"] = dGlitchStrength;
    pGlitch->execute(pContext);
}

void PostProcessGlitch::loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    name = "Glitch";
    SET_PROGRAM_VARS(Glitch, "Glitch.ps.hlsl");
    dGlitchStrength=0.05f;
}

void PostProcessGlitch::gui()
{
    pGui->addFloatSlider("Strength", dGlitchStrength, 0.01f, 1.0f);
}
