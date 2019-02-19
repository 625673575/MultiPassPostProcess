#include "PostProcessVignette.h"

void PostProcessVignette::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "Vignette";
    SET_PROGRAM_VARS(Vignette, "Vignette.ps.hlsl");

    dVignette_Center = vec2(0.5f, 0.5f);
    dVignette_Settings = vec4(0.8f);
    dVignette_Color = vec3();
}

void PostProcessVignette::execute()
{
    pContext->setGraphicsVars(vVignette);
    vVignette->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));

    vVignette["VignetteCB"]["_ScreenParams"] = getResolution();
    vVignette["VignetteCB"]["_Vignette_Center"] = dVignette_Center;
    vVignette["VignetteCB"]["_Vignette_Settings"] = dVignette_Settings;
    vVignette["VignetteCB"]["_Vignette_Color"] = dVignette_Color;
    pVignette->execute(pContext);
}

void PostProcessVignette::gui()
{
    pGui->addFloat2Slider("Center", dVignette_Center, 0, 1);
    pGui->addFloatSlider("Intensity", dVignette_Settings.x, 0, 1);
    pGui->addFloatSlider("Smoothness", dVignette_Settings.y, 0, 1);
    pGui->addFloatSlider("Roundness", dVignette_Settings.z, 0, 1);
    static bool rounded = false;
    pGui->addCheckBox("Rounded", rounded);
    dVignette_Settings.w = rounded? 1.0f : 0.0f;
    pGui->addRgbColor("Color", dVignette_Color);
}
