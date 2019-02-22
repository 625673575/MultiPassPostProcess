#include "PostProcessBugTV.h"

void PostProcessBugTV::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "BugTV";
    SET_PROGRAM_VARS(BugTV, "BugTV.ps.hlsl");
    dFrequency = 11.0f;
    dShaderDefines = { {"CURVE",true},{"SCANS",true},{"FLICKS",true},{"GRAINS",true},{"YBUG",true},{"DIRTY",true},{"STRIP",true},{"COLOR",true},{"BLINK",true},{"VIG",true}, };
    updateDefines();
}

void PostProcessBugTV::execute()
{
    pContext->getGraphicsState()->setScissors(0, GraphicsState::Scissor(250, 100, 600, 500));
    pContext->setGraphicsVars(vBugTV);
    vBugTV->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
    vBugTV->setSampler("gSampler", pLineBoardSampler);
    vBugTV["BugTVCB"]["iGlobalTime"] = pSample->getCurrentTime();
    vBugTV["BugTVCB"]["Frequency"] = dFrequency;
    vBugTV["BugTVCB"]["iResolution"] = getResolution();
    pBugTV->execute(pContext);
    auto viewportDesc = pContext->getGraphicsState()->getViewport(0);
    pContext->getGraphicsState()->setScissors(0, GraphicsState::Scissor(uint32_t(viewportDesc.originX), uint32_t(viewportDesc.originY), uint32_t(viewportDesc.originX + viewportDesc.width), uint32_t(viewportDesc.height + viewportDesc.originY)));
}

void PostProcessBugTV::gui()
{
    static auto previousDefines = dShaderDefines;
    pGui->addFloatSlider("Frequency", dFrequency, 5, 30);
    for (auto& v : dShaderDefines) {
        pGui->addCheckBox(v.first.c_str(), v.second);
    }
    if (previousDefines != dShaderDefines) {
        updateDefines();
        previousDefines = dShaderDefines;
    }
}

void PostProcessBugTV::updateDefines()
{
    SET_PROGRAM_VARS(BugTV, "BugTV.ps.hlsl");
    for (auto& v : dShaderDefines) {
        if (v.second)
            pBugTV->getProgram()->addDefine(v.first);
    }
}
