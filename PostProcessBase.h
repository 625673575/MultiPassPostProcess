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
    Sampler::SharedPtr pLineWarpSampler;
    Sampler::SharedPtr pLineBoardSampler;
    static vec3 vMouseState;
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
    void onGuiRender();
    void onFrameRender();

    void SetTexture(const Texture::SharedPtr& t) { pTexture = t; }
    static void SetMouseState(vec3 t) { vMouseState = t; }
    void loadImage(std::function<void(const std::string& filename)> f);

    bool enable() { return bEnable; }
    vec2 getResolution() { return { pSample->getWindow()->getClientAreaWidth(),pSample->getWindow()->getClientAreaWidth() }; }
    vec3 getMousePos() { return { pSample->getWindow()->getClientAreaWidth()*vMouseState.x,pSample->getWindow()->getClientAreaWidth()*vMouseState.y,vMouseState.z }; }
};

