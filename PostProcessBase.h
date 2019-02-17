#pragma once
#include "Falcor.h"
#define SET_PROGRAM_VARS(NAME,SHADER)    p##NAME = FullScreenPass::create(SHADER);\
v##NAME = GraphicsVars::create(p##NAME->getProgram()->getReflector());

using namespace Falcor;
class PostProcessBase
{
protected:
    std::string name;
    bool bEnable;
    SampleCallbacks* pSample;
    RenderContext* pContext;
    Gui* pGui;
    Texture::SharedPtr pTexture;
public:
using UniquePtr = std::unique_ptr<PostProcessBase>;
    using ShaderPass = FullScreenPass::UniquePtr;
    using ShaderVar = GraphicsVars::SharedPtr;
    using ShaderTex = Texture::SharedPtr;
    PostProcessBase() = default;
    virtual ~PostProcessBase() = default;
protected:
    virtual void execute() {}
    virtual void gui() {}
public:
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui);
    void onGuiRender() {
        pGui->addCheckBox(name.c_str(), bEnable);
        if (bEnable) { gui(); }
    }
    void onFrameRender() {
        if (bEnable)execute();
    }

    void SetTexture(const Texture::SharedPtr& t) {
        pTexture = t;
    }
    void loadImage(std::function<void(const std::string& filename)> f)
    {
        std::string filename;
        FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
        if (openFileDialog(filters, filename))
        {
            f(filename);
        }
    }

    bool enable() { return bEnable; }
    vec2 getResolution() { return { pSample->getWindow()->getClientAreaWidth(),pSample->getWindow()->getClientAreaWidth() }; }
};

