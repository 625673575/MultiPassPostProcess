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

Texture::SharedPtr MultiPassPostProcess::pTextureNoise = nullptr;
Texture::SharedPtr MultiPassPostProcess::pTextureNoiseRGB = nullptr;
Texture::SharedPtr MultiPassPostProcess::pTextureStar = nullptr;
Texture::SharedPtr MultiPassPostProcess::pTextureGirl = nullptr;
Texture::SharedPtr MultiPassPostProcess::pTextureWoodFloor = nullptr;
Texture::SharedPtr MultiPassPostProcess::pTextureSelectedFromFile = nullptr;

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
    }
    for (auto& v : postProcessor) {
        v->onGuiRender();
    }

    static ivec4 rect(200, 400, 20, 400);
    pGui->pushWindow("ShaderToy", rect.x, rect.y, rect.z, rect.w, false, true, true, false);
    //pGui->addTooltip(help, true);

    for (auto& v : shaderToy) {
        v->onGuiRender();
    }

    pGui->popWindow();

    static ivec4 frameRect(300, 900, 1300, 40);
    pGui->pushWindow("History Frames", frameRect.x, frameRect.y, frameRect.z, frameRect.w, false, true, true, false);
    //pGui->addText("History Frames");
    //pGui->addTooltip(help, true);

    for (auto& v : PostProcessBase::gRencentFrames) {
        pGui->addImage("", v);
    }

    pGui->popWindow();

    modelViewer->onGuiRender(pSample, pGui);
}
void MultiPassPostProcess::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    pTextureNoise = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\noise1024.png", false, true);
    pTextureNoiseRGB = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\noisergb.jpg", false, true);
    pTextureStar = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\star.jpg", false, true);
    pTextureGirl = createTextureFromFile("c:\\Users\\Liu\\Pictures\\mnv.jpg", false, true);
    pTextureWoodFloor = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\wood_floor.jpg", false, true);

    mpGaussianBlur = GaussianBlur::create(5);
    mpBlit = FullScreenPass::create("Blit.ps.hlsl");
    mpProgVars = GraphicsVars::create(mpBlit->getProgram()->getReflector());

    postProcessor.emplace_back(new PostProcessMixer());
    postProcessor.emplace_back(new PostProcessVignette());
    postProcessor.emplace_back(new PostProcessGlitch());
    postProcessor.emplace_back(new PostProcessSharp());
    postProcessor.emplace_back(new PostProcessFilmGrain());
    postProcessor.emplace_back(new PostProcessBugTV());
    postProcessor.emplace_back(new PostProcessMotionBlur());
    postProcessor.emplace_back(new PostProcessLut());

    shaderToy.emplace_back(new ShaderToyImplementation("Toy"));
    shaderToy.emplace_back(new ShaderToyImplementation("Heart3D"));
    shaderToy.emplace_back(new ShaderToyImplementation("OceanStructure"));
    shaderToy.emplace_back(new ShaderToyImplementation("FractalCartoonLand"));
    shaderToy.emplace_back(new ShaderToyImplementation("Topologica"));
    auto mars = new ShaderToyImplementation("Seascape");
    mars->setTexture(0, "iChannel0", pTextureNoise);
    shaderToy.emplace_back(mars);
    std::vector<std::string> volcanic_name{ "VolcanicBuffer0", "Volcanic" };
    auto toy_volcanic = new ShaderToyImplementation(volcanic_name);
    toy_volcanic->setTexture(0, "iChannel0", pTextureGirl);
    toy_volcanic->setTexture(0, "iChannel1", pTextureNoise);
    toy_volcanic->setTexture(0, "iChannel2", pTextureWoodFloor);
    shaderToy.emplace_back(toy_volcanic);

    auto toy_pirates = new ShaderToyImplementation("Pirates");
    toy_pirates->setTexture(0, "iChannel0", pTextureWoodFloor);
    toy_pirates->setTexture(0, "iChannel1", pTextureNoise);
    shaderToy.emplace_back(toy_pirates);

    for (auto& v : postProcessor) {
        v->loadProgram(pSample, pRenderContext, pSample->getGui());
    }
    for (auto& v : shaderToy) {
        v->loadProgram(pSample, pRenderContext, pSample->getGui());
    }
    modelViewer = std::make_unique<ModelViewer>();
    modelViewer->onLoad(pSample, pRenderContext);
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
    pTextureSelectedFromFile = createTextureFromFile(filename, false, isSrgbFormat(fboFormat));

    Fbo::Desc fboDesc;
    fboDesc.setColorTarget(0, pTextureSelectedFromFile->getFormat());
    mpTempFB = FboHelper::create2D(pTextureSelectedFromFile->getWidth(), pTextureSelectedFromFile->getHeight() / 2, fboDesc);

    pSample->resizeSwapChain(pTextureSelectedFromFile->getWidth(), pTextureSelectedFromFile->getHeight());
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

    bool hasImage = false;

    for (auto& v : shaderToy) {
        if (v->enable()) {
            v->onFrameRender();
            hasImage = true;
        }
    }
    if (!hasImage && modelViewer->hasModel()) {
        modelViewer->onFrameRender(pSample, pContext, pTargetFbo);
        hasImage = true;
    }

    if (hasImage || pTextureSelectedFromFile)
    {
        Texture::SharedPtr& pSrcTex = hasImage ? pContext->getGraphicsState()->getFbo()->getColorTexture(0) : pTextureSelectedFromFile;

        pContext->setGraphicsVars(mpProgVars);

        if (mEnableGaussianBlur)
        {
            mpGaussianBlur->execute(pContext, pSrcTex, mpTempFB);
            mpProgVars->setTexture("gTexture", mpTempFB->getColorTexture(0));
            const FullScreenPass* pFinalPass = mpBlit.get();
            pFinalPass->execute(pContext);
        }
        else {
            mpProgVars->setTexture("gTexture", pSrcTex);
            mpBlit->execute(pContext);
        }

        for (auto& v : postProcessor) {
            v->onFrameRender();
        }
    }

    if (!pSample->isTimeFrozen()) {
        Texture::SharedPtr& a = pContext->getGraphicsState()->getFbo()->getColorTexture(0);
        auto newFrame = Texture::create2D(a->getWidth(), a->getHeight(), a->getFormat(), a->getArraySize(), 1);
        gpDevice->getRenderContext()->copyResource(newFrame.get(), a.get());

        PostProcessBase::gRencentFrames.push_back(newFrame);
        if (PostProcessBase::gRencentFrames.size() > PostProcessBase::HISTORY_FRAME_COUNT) {
            PostProcessBase::gRencentFrames.erase(PostProcessBase::gRencentFrames.begin());
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
        case KeyboardEvent::Key::B:
            mEnableGaussianBlur = true;
            return true;
        }
    }
    modelViewer->onKeyEvent(pSample, keyEvent);
    return false;
}

bool MultiPassPostProcess::onMouseEvent(SampleCallbacks * pSample, const MouseEvent & mouseEvent)
{
    PostProcessBase::SetMouseState(glm::vec3(mouseEvent.pos, mouseEvent.type));
    modelViewer->onMouseEvent(pSample, mouseEvent);
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
