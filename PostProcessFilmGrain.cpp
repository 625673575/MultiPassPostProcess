#include "PostProcessFilmGrain.h"

void PostProcessFilmGrain::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "FilmGrain";
    SET_PROGRAM_VARS(FilmGrain, "FilmGrain.ps.hlsl");
    dFilmGrainStrength = 24;
}

void PostProcessFilmGrain::execute()
{
    pContext->setGraphicsVars(vFilmGrain);
    vFilmGrain->setTexture("gTexture", getRecentFrame());
    vFilmGrain["FilmGrainCB"]["iGlobalTime"] = pSample->getCurrentTime();
    vFilmGrain["FilmGrainCB"]["strength"] = dFilmGrainStrength;
    pFilmGrain->execute(pContext);
}

void PostProcessFilmGrain::gui()
{
    pGui->addFloatSlider("Strength", dFilmGrainStrength, 1, 100);
}
