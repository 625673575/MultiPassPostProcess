#include "PostProcessBase.h"

vec3 PostProcessBase::vMouseState = vec3();
std::vector<Texture::SharedPtr> PostProcessBase::gRencentFrames = std::vector<Texture::SharedPtr>();


void PostProcessBase::loadProgram(SampleCallbacks * Sample, RenderContext * Context, Gui * Gui)
{
    bEnable = false;
    pSample = Sample;
    pContext = Context;
    pGui = Gui;
    Sampler::Desc samDesc;
    samDesc.setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Wrap, Sampler::AddressMode::Wrap);
    samDesc.setComparisonMode(Sampler::ComparisonMode::Never);
    samDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    pLineWarpSampler = Sampler::create(samDesc);

    samDesc.setAddressingMode(Sampler::AddressMode::Border, Sampler::AddressMode::Border, Sampler::AddressMode::Border);
    pLineBoardSampler = Sampler::create(samDesc);
}

void PostProcessBase::onGuiRender()
{
    pGui->addCheckBox(sName.c_str(), bEnable);
    if (bEnable) { gui(); }
}

void PostProcessBase::onFrameRender()
{
    if (bEnable)execute();
}

Texture::SharedPtr PostProcessBase::getRecentFrame(uint index)
{
    if (index == 0 || index >= gRencentFrames.size()) {
        return pContext->getGraphicsState()->getFbo()->getColorTexture(0);
    }
    return gRencentFrames[HISTORY_FRAME_COUNT - 1 - index];
}

void PostProcessBase::loadImage(std::function<void(const std::string&filename)> f)
{
    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    if (openFileDialog(filters, filename))
    {
        f(filename);
    }
}
