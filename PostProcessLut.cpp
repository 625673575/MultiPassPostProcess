#include "PostProcessLut.h"

void PostProcessLut::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "Lut";
    SET_PROGRAM_VARS(Lut, "Lut.ps.hlsl");
}

void PostProcessLut::execute()
{
    pContext->setGraphicsVars(vLut);
    vLut->setTexture("gTexture", getRecentFrame());
    vLut->setTexture("gLut", dLutTexture);

    vLut["PerImageCB"]["fLUT_AmountChroma"] = 1.0f;
    vLut["PerImageCB"]["fLUT_AmountLuma"]=1.0f;
    pLut->execute(pContext);
}

void PostProcessLut::gui()
{
    if (pGui->addButton("Load Lut")) {
        loadImage([this](auto& imageFile) {
            dLutTexture = createTextureFromFile(imageFile, false, true);
        });
    }
}
