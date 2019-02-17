#include "PostProcessBase.h"

void PostProcessBase::loadProgram(SampleCallbacks * Sample, RenderContext * Context, Gui * Gui)
{
    bEnable = false;
    pSample = Sample;
    pContext = Context;
    pGui = Gui;
}
