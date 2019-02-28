#include "PostProcessMixer.h"

void PostProcessMixer::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName = "Mixer";
    SET_PROGRAM_VARS(Mixer, "Mixer.ps.hlsl");
    dImageCount = 0;

    dRatio.resize(10);
    constexpr auto sizeofBuffer = 10* sizeof(float);
    pRatioBuffer = TypedBuffer<float>::create(sizeofBuffer);
    vMixer->setTypedBuffer("gRatio", pRatioBuffer);

    //std::vector<std::string> in{ "C:\\Users\\Liu\\Pictures\\ch.jpg","C:\\Users\\Liu\\Pictures\\tou.jpg" };
    //dImageArray = createTextureFromFile(in, false, true);
}

void PostProcessMixer::execute()
{
    pContext->setGraphicsVars(vMixer);
    vMixer->setTexture("gTexture", dImageArray);//pContext->getGraphicsState()->getFbo()->getColorTexture(0)
    pRatioBuffer->uploadToGPU();
    pMixer->execute(pContext);
}

void PostProcessMixer::gui()
{
    static  std::vector<std::string> imageFiles;
    if (dImageCount > 0)
        pGui->addImage("Images", dImageArray);
    if (pGui->addButton("Add Image")) {
        loadImage([this](auto y) {
            imageFiles.push_back(y); 
        dImageCount++;});
        //dImageArray = createTextureFromFile(imageFiles, false, true);
    }
    for (int i = 0; i < dImageCount; i++) {
        pGui->addFloatSlider("ratio", dRatio[i], 0, 1);
        pRatioBuffer[i] = dRatio[i];
    }
}
