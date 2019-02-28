#pragma once
#include "Falcor.h"
using namespace Falcor;
namespace TextureHelperExtend
{
    Texture::SharedPtr createTextureFromFile(const std::vector<std::string>& filename, bool generateMipLevels, bool loadAsSrgb, Texture::BindFlags bindFlags = Texture::BindFlags::ShaderResource);

};

