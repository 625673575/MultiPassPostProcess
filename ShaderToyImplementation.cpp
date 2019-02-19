#include "ShaderToyImplementation.h"

ShaderToyImplementation::ShaderToyImplementation(const std::string & hlsl)
{
    sName = hlsl;
    passes.emplace_back(hlsl, PassVar());
}

ShaderToyImplementation::ShaderToyImplementation(const std::vector<std::string>& hlsl)
{
    if (hlsl.empty())throw;
    for (auto &v : hlsl) {
        passes.emplace_back(v, PassVar());
    }
    sName = hlsl.back();
}

void ShaderToyImplementation::loadProgram(SampleCallbacks * pSample, RenderContext * pContext, Gui * pGui)
{
    PostProcessBase::loadProgram(pSample, pContext, pGui);
    for (auto&v : passes) {
        v.second.pToy = FullScreenPass::create(v.first + ".ps.hlsl");
        v.second.vToy = GraphicsVars::create(v.second.pToy->getProgram()->getReflector());
        v.second.mToyCBBinding = v.second.pToy->getProgram()->getReflector()->getDefaultParameterBlock()->getResourceBinding("ToyCB");
    }
}

void ShaderToyImplementation::setTexture(int passIndex, const std::string & bufferName, const std::string & imageFile)
{
    auto& pass = passes[passIndex].second;
    Texture::SharedPtr tPtr = createTextureFromFile(imageFile, false, true);
    pass.tParams.emplace_back(imageFile.c_str(), bufferName.c_str(), tPtr);
}

void ShaderToyImplementation::setTexture(int passIndex, const std::string & bufferName, Texture::SharedPtr & imageFile)
{
    auto& pass = passes[passIndex].second;
    pass.tParams.emplace_back(imageFile->getName().c_str(), bufferName.c_str(), imageFile);
}

void ShaderToyImplementation::execute()
{
    int i = 0;
    for (auto &v : passes) {
        pContext->setGraphicsVars(v.second.vToy);
        //vToy->setTexture("gTexture", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
        auto b0 = v.second.vToy->getDefaultBlock()->getConstantBuffer(v.second.mToyCBBinding, 0);
        b0["iTime"] = pSample->getCurrentTime();
        b0["iResolution"] = getResolution();
        b0["iMouse"] = getMousePos();
        if (i > 0) {
            v.second.vToy->setTexture("iChannel0", pContext->getGraphicsState()->getFbo()->getColorTexture(0));
        }
        for (auto&vTextureParam : v.second.tParams) {
            v.second.vToy->setTexture(vTextureParam.bufferName, vTextureParam.pTexture);
        }
        v.second.pToy->execute(pContext);
        i++;
    }
}

void ShaderToyImplementation::gui()
{
}
