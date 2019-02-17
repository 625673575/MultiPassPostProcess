#include "PostProcessFilmGrain.h"



PostProcessFilmGrain::PostProcessFilmGrain():dFilmGrainStrength(24)
{
}


PostProcessFilmGrain::~PostProcessFilmGrain()
{
}

void PostProcessFilmGrain::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    name = "FilmGrain";
    SET_PROGRAM_VARS(FilmGrain, "FilmGrain.ps.hlsl");
}

void PostProcessFilmGrain::execute()
{
    pContext->setGraphicsVars(vFilmGrain);
    vFilmGrain->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
    vFilmGrain["FilmGrainCB"]["iGlobalTime"] = pSample->getCurrentTime();
    vFilmGrain["FilmGrainCB"]["strength"] = dFilmGrainStrength;
    pFilmGrain->execute(pContext);
}

void PostProcessFilmGrain::gui()
{
    pGui->addFloatSlider("Strength", dFilmGrainStrength, 1, 100);
}
