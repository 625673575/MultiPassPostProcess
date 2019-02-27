#pragma once
#include "Falcor.h"
#include "ModelResource.h"
using namespace Falcor;

class ModelViewer
{
public:
    using UniquePtr = std::unique_ptr<ModelViewer>;
    ModelViewer();
    ~ModelViewer();
    void onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext);
    void onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo);
    void onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height);
    bool onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent);
    bool onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent);
    void onGuiRender(SampleCallbacks* pSample, Gui* pGui);
    bool hasModel() { return !models.empty(); }
private:
    void openDialogLoadModel(ResourceFormat fboFormat);
    void deleteCulledMeshes(ModelResource& mesh);

    ModelResource loadModelFromFile(const std::string& Filename, ResourceFormat fboFormat, bool animation = false, bool useLinearFilter = true);
    void resetCamera();
    void renderModelUI(Gui* pGui);
    void loadShaderProgram();
    void loadMaterialFunctions();
    void loadModelResources();

    ModelViewCameraController mModelViewCameraController;
    FirstPersonCameraController mFirstPersonCameraController;
    SixDoFCameraController m6DoFCameraController;
    Sampler::SharedPtr mpPointSampler = nullptr;
    Sampler::SharedPtr mpLinearSampler = nullptr;
    enum
    {
        ModelViewCamera,
        FirstPersonCamera,
        SixDoFCamera
    } mCameraType = ModelViewCamera;

    Camera::SharedPtr mpCamera;
    CameraController& getActiveCameraController();
    float mNearZ;
    float mFarZ;

    bool mDrawWireframe = false;
    uint32_t mActiveAnimationID = kBindPoseAnimationID;
    static const uint32_t kBindPoseAnimationID = AnimationController::kBindPoseAnimationId;

    RasterizerState::SharedPtr mpWireframeRS = nullptr;
    RasterizerState::SharedPtr mpCullRastState[3]; // 0 = no culling, 1 = backface culling, 2 = frontface culling
    uint32_t mCullMode = 1;

    BlendState::SharedPtr mpBlendState[2];
    uint32_t mBlendMode = 0;

    DepthStencilState::SharedPtr mpNoDepthDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestDS = nullptr;

    DirectionalLight::SharedPtr mpDirLight;
    PointLight::SharedPtr mpPointLight;

    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;
    std::vector<ModelResource> models;
    static std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>mMaterialFuncMap;
    static std::map<std::string, GraphicsProgram::SharedPtr>mProgramMap;
public:
    static std::function<void(MaterialInstance::SharedPtr&)>& getMaterialFunc(const std::string& name) { return mMaterialFuncMap[name]; };
    static GraphicsProgram::SharedPtr& getProgram(const std::string& name) { return mProgramMap[name]; }
    static const std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>& getMaterialFuncMap() { return mMaterialFuncMap; };
    static const std::map<std::string, GraphicsProgram::SharedPtr>& getProgramMap() { return mProgramMap; }

};

