#include "PostProcessSharp.h"

void PostProcessSharp::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    sName="Sharp";
    SET_PROGRAM_VARS(Sharp, "Sharp.ps.hlsl");

    //Buffer<float>定义
    constexpr auto sizeofBuffer = sizeof(mdSharpSaturation) / sizeof(glm::vec1);
    mpSharpSaturationBuffer = TypedBuffer<float>::create(sizeofBuffer);
    vSharp->setTypedBuffer("gSaturation", mpSharpSaturationBuffer);

    //StructureBuffer定义
    mpSharpLightBuffer = StructuredBuffer::create(pSharp->getProgram(), "gLight", 10);
    vSharp->setStructuredBuffer("gLight", mpSharpLightBuffer);

    //RWRawBuffer
    uint32_t z = 0;
    mpInvocationsBuffer = Buffer::create(sizeof(uint32_t), Buffer::BindFlags::UnorderedAccess, Buffer::CpuAccess::Read, &z);
    vSharp->setRawBuffer("gInvocationBuffer", mpInvocationsBuffer);

    mdSharpContrast = 2.0f;
    mdSharpSaturation = glm::vec2(0.0f, 1.0f);
    mSharpLightNum = 1;
    mdSharpLight.resize(mSharpLightNum);
    for (auto &v : mdSharpLight) {
        v.intensity = float(LIGHT_COUNT) / mSharpLightNum;
    }
}

void PostProcessSharp::execute()
{
    pContext->setGraphicsVars(vSharp);
    vSharp->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));

    mpSharpSaturationBuffer[0] = mdSharpSaturation.x;
    mpSharpSaturationBuffer[1] = mdSharpSaturation.y;
    mpSharpSaturationBuffer->uploadToGPU();

    vSharp["SharpParamCB"]["contrast"] = mdSharpContrast;
    vSharp["SharpParamCB"]["invocation"] = mdCountPixelShaderInvocations;
    for (int32_t i = 0; i < mSharpLightNum; ++i) {
        mpSharpLightBuffer[i]["color"] = mdSharpLight[i].color;
        mpSharpLightBuffer[i]["intensity"] = mdSharpLight[i].intensity;
    }
    pSharp->execute(pContext);

    if (mdCountPixelShaderInvocations)
    {
        uint32_t* pData = (uint32_t*)mpInvocationsBuffer->map(Buffer::MapType::Read);
        std::string msg = "PS was invoked " + std::to_string(*pData) + " times";
        pSample->renderText(msg, vec2(300, 100));
        mpInvocationsBuffer->unmap();
        pContext->clearUAV(mpInvocationsBuffer->getUAV().get(), uvec4(0));
    }
}

void PostProcessSharp::gui()
{
    pGui->addFloatVar("Contrast", mdSharpContrast, 0.1f, 10.0f);
    pGui->addFloat2Var("Saturation", mdSharpSaturation, 0.0f, 1.0f);
    pGui->addIntVar("Light Num", mSharpLightNum, 0, LIGHT_COUNT);
    if (mdSharpLight.size() != mSharpLightNum) {
        mdSharpLight.resize(mSharpLightNum);
        if (mSharpLightNum == 1) {
            mdSharpLight[0].intensity = LIGHT_COUNT;
        }
        else {
            std::for_each(mdSharpLight.begin(), mdSharpLight.end(), [this](SLight& x) -> void {if (x.intensity == 10.0f / (mSharpLightNum - 1) || x.intensity == 1.0f || x.intensity == 0.0f) x.intensity = 10.0f / mSharpLightNum; });
        }
    }
    for (int32_t i = 0; i < mSharpLightNum; ++i) {
        pGui->addRgbColor(std::to_string(i).data(), mdSharpLight[i].color);
        pGui->addFloatSlider(std::to_string(i).data(), mdSharpLight[i].intensity, 1.0f, 10.0f);
    }
    pGui->addCheckBox("Count FS invocations", mdCountPixelShaderInvocations);
}
