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
    }
    for (auto& v : postProcessor) {
        v->onGuiRender();
    }

    static ivec4 rect(200, 400, 20, 400);
    pGui->pushWindow("ShaderToy", rect.x, rect.y, rect.z, rect.w);
    //pGui->addTooltip(help, true);

    for (auto& v : shaderToy) {
        v->onGuiRender();
    }

    pGui->popWindow();

    static ivec4 frameRect(300, 900, 1300, 40);
    pGui->pushWindow("History Frames", frameRect.x, frameRect.y, frameRect.z, frameRect.w);
    auto& depthTex = PostProcessBase::gDepthTexture;
    if (depthTex) {
        pGui->addImage("", mpDepthLiner01Texture);
    }
    for (auto& v : PostProcessBase::gRencentFrames) {
        pGui->addImage("", v);
    }
    pGui->popWindow();

    modelViewer->onGuiRender(pSample, pGui);
}
void MultiPassPostProcess::onDroppedFile(SampleCallbacks* pSample, const std::string& filename)
{
    if (hasSuffix(filename, ".fbx", false) || hasSuffix(filename, ".dae", false) || hasSuffix(filename, ".obj", false))
    {
        modelViewer->loadModel(filename, ResourceFormat::RGBA8UnormSrgb);
        return;
    }
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"},{"jpeg"},  {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    for (auto& v : filters) {
        if (hasSuffix(filename, v.ext, false)) {
            loadImageFromFile(pSample, filename);
            return;
        }
    }
    msgBox("DropFile only support 3d Model and picture format");
}
void MultiPassPostProcess::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    MaterialInstance::loadStaticData();
    mpGaussianBlur = GaussianBlur::create(5);
    mpBlit = FullScreenPass::create("Blit.ps.hlsl");
    mpProgVars = GraphicsVars::create(mpBlit->getProgram()->getReflector());

    //loadShaderToy();
    loadPostProcess();

    for (auto& v : postProcessor) {
        v->loadProgram(pSample, pRenderContext, pSample->getGui());
    }
    for (auto& v : shaderToy) {
        v->loadProgram(pSample, pRenderContext, pSample->getGui());
    }
    modelViewer = std::make_unique<ModelViewer>();
    modelViewer->onLoad(pSample, pRenderContext);

    //Compute Shader
    mpComputeProg = ComputeProgram::createFromFile("DepthToLiner01.hlsl", "main");
    mpComputeState = ComputeState::create();
    mpComputeState->setProgram(mpComputeProg);
    mpComputeProgVars = ComputeVars::create(mpComputeProg->getReflector());
    Fbo::SharedPtr pFbo = pSample->getCurrentFbo();
    mpDepthLiner01Texture = Texture::create2D(pFbo->getWidth(), pFbo->getHeight(), ResourceFormat::RGBA8Unorm, 1, 1, nullptr, Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
    PostProcessBase::gDepthTexture = Texture::create2D(pFbo->getWidth(), pFbo->getHeight(), ResourceFormat::D32Float, 1, 1, nullptr, Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
}

void MultiPassPostProcess::loadImage(SampleCallbacks * pSample)
{
    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"},{"jpeg"},  {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    if (openFileDialog(filters, filename))
    {
        loadImageFromFile(pSample, filename);
    }
}

void MultiPassPostProcess::loadImageFromFile(SampleCallbacks * pSample, std::string filename)
{
    auto fboFormat = pSample->getCurrentFbo()->getColorTexture(0)->getFormat();
    pTextureSelectedFromFile = createTextureFromFile(filename, false, isSrgbFormat(fboFormat));

    Fbo::Desc fboDesc;
    fboDesc.setColorTarget(0, pTextureSelectedFromFile->getFormat());

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

void MultiPassPostProcess::loadShaderToy()
{
    shaderToy.emplace_back(new ShaderToyImplementation("Toy"));
    shaderToy.emplace_back(new ShaderToyImplementation("Heart3D"));
    shaderToy.emplace_back(new ShaderToyImplementation("OceanStructure"));
    shaderToy.emplace_back(new ShaderToyImplementation("FractalCartoonLand"));
    shaderToy.emplace_back(new ShaderToyImplementation("Topologica"));
    auto mars = new ShaderToyImplementation("Seascape");
    mars->setTexture(0, "iChannel0", MaterialInstance::pTextureNoise);
    shaderToy.emplace_back(mars);
    std::vector<std::string> volcanic_name{ "VolcanicBuffer0", "Volcanic" };
    auto toy_volcanic = new ShaderToyImplementation(volcanic_name);
    toy_volcanic->setTexture(0, "iChannel0", MaterialInstance::pTextureSmoke);
    toy_volcanic->setTexture(0, "iChannel1", MaterialInstance::pTextureNoise);
    toy_volcanic->setTexture(0, "iChannel2", MaterialInstance::pTextureWoodFloor);
    shaderToy.emplace_back(toy_volcanic);

    auto toy_pirates = new ShaderToyImplementation("Pirates");
    toy_pirates->setTexture(0, "iChannel0", MaterialInstance::pTextureWoodFloor);
    toy_pirates->setTexture(0, "iChannel1", MaterialInstance::pTextureNoise);
    shaderToy.emplace_back(toy_pirates);
}

void MultiPassPostProcess::loadPostProcess()
{
    postProcessor.emplace_back(new PostProcessMixer());
    postProcessor.emplace_back(new PostProcessVignette());
    postProcessor.emplace_back(new PostProcessGlitch());
    postProcessor.emplace_back(new PostProcessSharp());
    postProcessor.emplace_back(new PostProcessFilmGrain());
    postProcessor.emplace_back(new PostProcessBugTV());
    postProcessor.emplace_back(new PostProcessMotionBlur());
    postProcessor.emplace_back(new PostProcessLut());
}

void MultiPassPostProcess::onFrameRender(SampleCallbacks * pSample, RenderContext * pContext, const Fbo::SharedPtr & pTargetFbo)
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
    pContext->pushGraphicsState(pContext->getGraphicsState());
    if (!hasImage && modelViewer->hasModel()) {
        modelViewer->onFrameRender(pSample, pContext, pTargetFbo);
        hasImage = true;
    }
    pContext->popGraphicsState();
    mpComputeProgVars->setTexture("gInput", PostProcessBase::gDepthTexture);
    mpComputeProgVars->setTexture("gOutput", mpDepthLiner01Texture);

    pContext->setComputeState(mpComputeState);
    pContext->setComputeVars(mpComputeProgVars);

    uint32_t w = (PostProcessBase::gDepthTexture->getWidth() / 16) + 1;
    uint32_t h = (PostProcessBase::gDepthTexture->getHeight() / 16) + 1;
    pContext->dispatch(w, h, 1);

    if (hasImage || pTextureSelectedFromFile)
    {
        Texture::SharedPtr& pSrcTex = hasImage ? pContext->getGraphicsState()->getFbo()->getColorTexture(0) : pTextureSelectedFromFile;

        pContext->setGraphicsVars(mpProgVars);

        if (mEnableGaussianBlur)
        {
            if(mpTempFB==nullptr){
                auto& currentFbo = pContext->getGraphicsState()->getFbo();
                mpTempFB = FboHelper::create2D(currentFbo->getWidth(), currentFbo->getHeight() / 2, currentFbo->getDesc());
            }
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

void MultiPassPostProcess::onShutdown(SampleCallbacks * pSample)
{

}

bool MultiPassPostProcess::onKeyEvent(SampleCallbacks * pSample, const KeyboardEvent & keyEvent)
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

void MultiPassPostProcess::onInitializeTesting(SampleCallbacks * pSample)
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
