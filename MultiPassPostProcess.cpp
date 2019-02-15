/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "MultiPassPostProcess.h"
void MultiPassPostProcess::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    if (pGui->addButton("Load Image"))
    {
        loadImage(pSample);
    }
    if (pGui->addButton("Load Image", true))
    {
        loadVideoFromFile(pSample);
    }
    pGui->addCheckBox("Gaussian Blur", mEnableGaussianBlur);
    if (mEnableGaussianBlur)
    {
        mpGaussianBlur->renderUI(pGui, "Blur Settings");
        pGui->addCheckBox("Grayscale", mEnableGrayscale);
    }
    pGui->addCheckBox("Sharp", mEnableSharp);
    if (mEnableSharp)
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
            pGui->addFloatSlider(std::to_string(i).data(), mdSharpLight[i].intensity,1.0f,10.0f);
        }
        pGui->addCheckBox("Count FS invocations", mdCountPixelShaderInvocations);

    }

    pGui->addCheckBox("FilmGrain", mEnableFilmGrain);
    if (mEnableFilmGrain) {
        pGui->addFloatSlider("Strength", mdFilmGrainStrength, 1, 100);
    }
    pGui->addCheckBox("EnableGlitch", mEnableGlitch);
    if (mEnableGlitch) {
        pGui->addFloatSlider("Strength", mdGlitchStrength, 0.01f,1.0f);
    }
}

void MultiPassPostProcess::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    mpLuminance = FullScreenPass::create("Luminance.ps.hlsl");
    mpSharp = FullScreenPass::create("Sharp.ps.hlsl");
    mpFilmGrain = FullScreenPass::create("FilmGrain.ps.hlsl");
    mpGlitch = FullScreenPass::create("Glitch.ps.hlsl");
    mpGaussianBlur = GaussianBlur::create(5);
    mpBlit = FullScreenPass::create("Blit.ps.hlsl");
    mpProgVars = GraphicsVars::create(mpBlit->getProgram()->getReflector());
    mpSharpVars = GraphicsVars::create(mpSharp->getProgram()->getReflector());
    mpFilmGrainVars = GraphicsVars::create(mpFilmGrain->getProgram()->getReflector());
    mpGlitchVars= GraphicsVars::create(mpGlitch->getProgram()->getReflector());
    for (uint32_t i = 0; i < mpFilmGrainVars->getParameterBlockCount(); i++) {

        auto block = mpFilmGrainVars->getParameterBlock(i);
        auto layout = block->getReflection()->getDescriptorSetLayouts();
    }

    //Buffer<float>定义
    constexpr auto sizeofBuffer = sizeof(mdSharpSaturation) / sizeof(glm::vec1);
    mpSharpSaturationBuffer = TypedBuffer<float>::create(sizeofBuffer);
    mpSharpVars->setTypedBuffer("gSaturation", mpSharpSaturationBuffer);

    //StructureBuffer定义
    mpSharpLightBuffer = StructuredBuffer::create(mpSharp->getProgram(), "gLight", 10);
    mpSharpVars->setStructuredBuffer("gLight", mpSharpLightBuffer);

    uint32_t z = 0;
    mpInvocationsBuffer = Buffer::create(sizeof(uint32_t), Buffer::BindFlags::UnorderedAccess, Buffer::CpuAccess::Read, &z);
    mpSharpVars->setRawBuffer("gInvocationBuffer", mpInvocationsBuffer);

    mdSharpContrast = 1.0f;
    mdSharpSaturation = glm::vec2(0.0f, 1.0f);
    mSharpLightNum = 1;
    mdSharpLight.resize(mSharpLightNum);
    for (auto &v : mdSharpLight) {
        v.intensity = float(LIGHT_COUNT) / mSharpLightNum;
    }
    mdFilmGrainStrength = 24;
}

void MultiPassPostProcess::loadImage(SampleCallbacks* pSample)
{
    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    if (openFileDialog(filters, filename))
    {
        loadImageFromFile(pSample, filename);
    }
}

void MultiPassPostProcess::loadImageFromFile(SampleCallbacks* pSample, std::string filename)
{
    auto fboFormat = pSample->getCurrentFbo()->getColorTexture(0)->getFormat();
    mpImage = createTextureFromFile(filename, false, isSrgbFormat(fboFormat));

    Fbo::Desc fboDesc;
    fboDesc.setColorTarget(0, mpImage->getFormat());
    mpTempFB = FboHelper::create2D(mpImage->getWidth(), mpImage->getHeight(), fboDesc);

    pSample->resizeSwapChain(mpImage->getWidth(), mpImage->getHeight());
}

void MultiPassPostProcess::loadVideoFromFile(SampleCallbacks * pSample)
{
    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    if (openFileDialog(filters, filename))
    {
        //mpVideoDecoder =  VideoDecoder::create(filename);
        //auto frame= mpVideoDecoder->getTextureForNextFrame(1.0f);

    }
}

void MultiPassPostProcess::onFrameRender(SampleCallbacks* pSample, RenderContext* pContext, const Fbo::SharedPtr& pTargetFbo)
{
    const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pContext->clearFbo(pTargetFbo.get(), clearColor, 0, 0, FboAttachmentType::Color);

    if (mpImage)
    {
        // Grayscale is only with radial blur
        mEnableGrayscale = mEnableGaussianBlur && mEnableGrayscale;

        pContext->setGraphicsVars(mpProgVars);

        if (mEnableGaussianBlur)
        {
            mpGaussianBlur->execute(pContext, mpImage, mpTempFB);
            mpProgVars->setTexture("gTexture", mpTempFB->getColorTexture(0));
            const FullScreenPass* pFinalPass = mEnableGrayscale ? mpLuminance.get() : mpBlit.get();
            pFinalPass->execute(pContext);
        }
        else
        {
            mpProgVars->setTexture("gTexture", mpImage);
            mpBlit->execute(pContext);
        }

        if (mEnableSharp) {

            pContext->setGraphicsVars(mpSharpVars);
            mpSharpVars->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));

            mpSharpSaturationBuffer[0] = mdSharpSaturation.x;
            mpSharpSaturationBuffer[1] = mdSharpSaturation.y;
            mpSharpSaturationBuffer->uploadToGPU();

            mpSharpVars["SharpParamCB"]["contrast"] = mdSharpContrast;
            mpSharpVars["SharpParamCB"]["invocation"] = mdCountPixelShaderInvocations;
            for (int32_t i = 0; i < mSharpLightNum; ++i) {
                mpSharpLightBuffer[i]["color"] = mdSharpLight[i].color;
                mpSharpLightBuffer[i]["intensity"] = mdSharpLight[i].intensity;
            }
            mpSharp->execute(pContext);

            if (mdCountPixelShaderInvocations)
            {
                // RWByteAddressBuffer
                uint32_t* pData = (uint32_t*)mpInvocationsBuffer->map(Buffer::MapType::Read);
                std::string msg = "PS was invoked " + std::to_string(*pData) + " times";
                pSample->renderText(msg, vec2(300, 100));
                mpInvocationsBuffer->unmap();

                //// RWStructuredBuffer UAV Counter
                //pData = (uint32_t*)mpRWBuffer->getUAVCounter()->map(Buffer::MapType::Read);
                //msg = "UAV Counter counted " + std::to_string(*pData) + " times";
                //pSample->renderText(msg, vec2(600, 120));
                //mpRWBuffer->getUAVCounter()->unmap();

                pContext->clearUAV(mpInvocationsBuffer->getUAV().get(), uvec4(0));
                //pContext->clearUAVCounter(mpRWBuffer, 0);
            }
        }
        if (mEnableFilmGrain) {
            pContext->setGraphicsVars(mpFilmGrainVars);
            mpFilmGrainVars->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
            mpFilmGrainVars["FilmGrainCB"]["iGlobalTime"] = pSample->getCurrentTime();
            mpFilmGrainVars["FilmGrainCB"]["strength"] = mdFilmGrainStrength;
            mpFilmGrain->execute(pContext);
        }
        if (mEnableGlitch) {
            pContext->setGraphicsVars(mpGlitchVars);
            mpGlitchVars->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
            mpGlitchVars["GlitchCB"]["iGlobalTime"] = pSample->getCurrentTime();
            mpGlitchVars["GlitchCB"]["strength"] = mdGlitchStrength;
            mpGlitch->execute(pContext);
        }
    }
}

void MultiPassPostProcess::onShutdown(SampleCallbacks* pSample)
{
}

bool MultiPassPostProcess::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        switch (keyEvent.key)
        {
        case KeyboardEvent::Key::L:
            loadImage(pSample);
            return true;
        case KeyboardEvent::Key::G:
            mEnableGrayscale = true;
            return true;
        case KeyboardEvent::Key::B:
            mEnableGaussianBlur = true;
            return true;
        }
    }
    return false;
}

void MultiPassPostProcess::onInitializeTesting(SampleCallbacks* pSample)
{
    auto argList = pSample->getArgList();
    std::vector<ArgList::Arg> filenames = argList.getValues("loadimage");
    if (!filenames.empty())
    {
        loadImageFromFile(pSample, filenames[0].asString());
    }

    if (argList.argExists("gaussianblur"))
    {
        mEnableGaussianBlur = true;
        if (argList.argExists("grayscale"))
        {
            mEnableGrayscale = true;
        }
    }
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
    MultiPassPostProcess::UniquePtr pRenderer = std::make_unique<MultiPassPostProcess>();

    SampleConfig config;
    config.windowDesc.title = "Multi-pass post-processing";
#ifdef _WIN32
    Sample::run(config, pRenderer);
#else
    config.argc = (uint32_t)argc;
    config.argv = argv;
    Sample::run(config, pRenderer);
#endif
    return 0;
}
