#include "PostProcessMotionBlur.h"

void PostProcessMotionBlur::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "MotionBlur";
    SET_PROGRAM_VARS(MotionBlur, "MotionBlur.ps.hlsl");
    dPreFrameIndex = 1;
    dBlurFactor = 0.5f;
}

void PostProcessMotionBlur::execute()
{
    pContext->setGraphicsVars(vMotionBlur);
    vMotionBlur->setTexture("gTexture[0]", getRecentFrame(0));
    vMotionBlur->setTexture("gTexture[1]", getRecentFrame(dPreFrameIndex));
    vMotionBlur["PerImageCB"]["gBlurFactor"] = dBlurFactor;
    pMotionBlur->execute(pContext);
}

void PostProcessMotionBlur::gui()
{
    pGui->addFloatSlider("Blur Factor", dBlurFactor, 0.0f, 1.0f);
    pGui->addIntVar("Previous Image", dPreFrameIndex, 1, 4);
}
