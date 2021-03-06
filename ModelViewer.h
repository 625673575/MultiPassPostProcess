#pragma once
#include "Falcor.h"
#include "ModelResource.h"
#include "SceneRendererExtend.h"
#include "SceneExtend.h"
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
    bool hasModel() { return !mpScene->empty(); }
    void loadModel(const std::string& Filename, ResourceFormat fboFormat, bool animation = false, bool useLinearFilter = true);
    static ModelViewer* getCurrentViewer() { return mCurrentViewer; }
private:
    void openDialogLoadModel(ResourceFormat fboFormat);
    void deleteCulledMeshes(ModelResource& mesh);

    void initDepthPass();
    bool loadModelFromFile(ModelResource& r, const std::string& Filename, ResourceFormat fboFormat, bool animation = false, bool useLinearFilter = true);
    void resetCamera(const Model::SharedPtr& model = nullptr, float distance = 5.0f);
    void renderModelUI(Gui* pGui);
    void loadSkyBox();
    void loadShaderProgram();
    void loadMaterialFunctions();
    void loadModelResources();
    SampleCallbacks* pSample;
    static ModelViewer* mCurrentViewer;
    std::shared_ptr<SceneExtend> mpScene;
    SceneRendererExtend::SharedPtr pSceneRenderer;
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

    enum HdrImage
    {
        EveningSun,
        OvercastDay,
        AtTheWindow,
        DayTime,
        Rathaus
    };
    static const Gui::DropdownList kSkyBoxDropDownList;
    HdrImage mHdrImageIndex = HdrImage::EveningSun;
    Texture::SharedPtr mHdrImage;
    SkyBox::SharedPtr mpSkyBox;

    Picking::UniquePtr mpScenePicker;
    std::set<const Scene::ModelInstance*> mSelectedInstances;
    CpuTimer mMouseHoldTimer;

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
    DepthStencilState::SharedPtr mpDepthTestLessDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestGreaterDS = nullptr;
    DepthStencilState::SharedPtr mpDepthTestAlways = nullptr;

    std::vector<DirectionalLight::SharedPtr> mpDirLight;
    PointLight::SharedPtr mpPointLight;

    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    GraphicsState::SharedPtr mpGraphicsState = nullptr;

    struct
    {
        GraphicsVars::SharedPtr pVars;
        GraphicsProgram::SharedPtr pProgram;
    } mDepthPass;
    Fbo::SharedPtr mpDepthPassFbo;

    static std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>mMaterialFuncMap;
    static std::map<std::string, GraphicsProgram::SharedPtr>mProgramMap;
public:
    static std::function<void(MaterialInstance::SharedPtr&)>& getMaterialFunc(const std::string& name) { return mMaterialFuncMap[name]; };
    static GraphicsProgram::SharedPtr& getProgram(const std::string& name) { return mProgramMap[name]; }
    static const std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>& getMaterialFuncMap() { return mMaterialFuncMap; };
    static const std::map<std::string, GraphicsProgram::SharedPtr>& getProgramMap() { return mProgramMap; }

};

