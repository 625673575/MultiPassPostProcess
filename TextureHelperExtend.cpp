#include "TextureHelperExtend.h"

Texture::SharedPtr TextureHelperExtend::createTextureFromFile(const std::vector<std::string>& filenames, bool generateMipLevels, bool loadAsSrgb, Texture::BindFlags bindFlags)
{
    Texture::SharedPtr pTex;
    std::vector<Bitmap::UniqueConstPtr> pBitmaps;
    for (auto &filename : filenames) {
        pBitmaps.push_back(std::move(Bitmap::createFromFile(filename, true)));
    }
    auto texCount = uint32_t(filenames.size());
    if (pBitmaps.back())
    {
        ResourceFormat texFormat = pBitmaps.back()->getFormat();
        if (loadAsSrgb)
        {
            texFormat = linearToSrgbFormat(texFormat);
        }
        size_t eachSize = pBitmaps.back()->getWidth()*pBitmaps.back()->getHeight() * 4;
        std::vector<uint8_t> d(eachSize*texCount);
        for (uint32_t i = 0; i < texCount; i++) {
            std::memcpy(&d[0 + i * eachSize], pBitmaps[i]->getData(), eachSize);
        }
        pTex = Texture::create2D(pBitmaps.back()->getWidth(), pBitmaps.back()->getHeight(), texFormat, texCount, generateMipLevels ? Texture::kMaxPossible : 1, &d[0], bindFlags);
    }
    if (pTex != nullptr)
    {
        pTex->setSourceFilename(stripDataDirectories(filenames[0]));
    }

    return pTex;
}
