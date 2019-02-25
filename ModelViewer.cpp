#include "ModelViewer.h"
#include "MultiPassPostProcess.h"
#include "SceneRendererExtend.h"
#include "SceneExtend.h"

ModelViewer::ModelViewer() : mNearZ(0.1f), mFarZ(1000000.0f)
{
}


ModelViewer::~ModelViewer()
{
}

void ModelViewer::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    mpCamera = Camera::create();
    mpProgram = GraphicsProgram::createFromFile("ModelViewer.ps.hlsl", "vert", "frag");

    // create rasterizer state
    RasterizerState::Desc wireframeDesc;
    wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    wireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpWireframeRS = RasterizerState::create(wireframeDesc);

    RasterizerState::Desc solidDesc;
    solidDesc.setCullMode(RasterizerState::CullMode::None);
    mpCullRastState[0] = RasterizerState::create(solidDesc);
    solidDesc.setCullMode(RasterizerState::CullMode::Back);
    mpCullRastState[1] = RasterizerState::create(solidDesc);
    solidDesc.setCullMode(RasterizerState::CullMode::Front);
    mpCullRastState[2] = RasterizerState::create(solidDesc);

    BlendState::Desc blendDesc;
    mpBlendState[0] = BlendState::create(blendDesc);
    blendDesc.setAlphaToCoverage(false);
    blendDesc.setRenderTargetWriteMask(0, true, true, true, true);

    blendDesc.setRtBlend(0, true).setRtParams(0, BlendState::BlendOp::Add, BlendState::BlendOp::Add,
        BlendState::BlendFunc::SrcAlpha, BlendState::BlendFunc::OneMinusSrcAlpha,
        BlendState::BlendFunc::One, BlendState::BlendFunc::Zero);
    mpBlendState[1] = BlendState::create(blendDesc);

    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthTest(false);
    mpNoDepthDS = DepthStencilState::create(dsDesc);
    dsDesc.setDepthTest(true);
    mpDepthTestDS = DepthStencilState::create(dsDesc);

    mModelViewCameraController.attachCamera(mpCamera);
    mFirstPersonCameraController.attachCamera(mpCamera);
    m6DoFCameraController.attachCamera(mpCamera);

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);
    mpPointSampler = Sampler::create(samplerDesc);
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    mpLinearSampler = Sampler::create(samplerDesc);

    mpDirLight = DirectionalLight::create();
    mpPointLight = PointLight::create();
    mpDirLight->setWorldDirection(glm::vec3(0.13f, 0.27f, -0.9f));

    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    mpGraphicsState = GraphicsState::create();
    mpGraphicsState->setProgram(mpProgram);

    loadModelResources();
}

void ModelViewer::onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    std::shared_ptr<SceneExtend> pScene = std::make_shared<SceneExtend>("");
    SceneRendererExtend::SharedPtr pSceneRenderer = SceneRendererExtend::create(pScene);

    const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpGraphicsState->setFbo(pTargetFbo);

    if (mDrawWireframe)
    {
        mpGraphicsState->setRasterizerState(mpWireframeRS);
        mpGraphicsState->setDepthStencilState(mpNoDepthDS);
    }
    else
    {
        mpGraphicsState->setRasterizerState(mpCullRastState[mCullMode]);
        mpGraphicsState->setDepthStencilState(mpDepthTestDS);
    }
    if (mBlendMode != 0) {
        mpGraphicsState->setDepthStencilState(mpNoDepthDS);
    }
    mpGraphicsState->setBlendState(mpBlendState[mBlendMode]);

    for (auto& v : models)
    {
        mpCamera->setDepthRange(mNearZ, mFarZ);
        getActiveCameraController().update();

        // Animate
        if (v.mAnimate)
        {
            PROFILE("animate");
            v.mpModel->animate(pSample->getCurrentTime());
        }
        // Set render state

        v.mpModel->bindSamplerToMaterials(v.mUseTriLinearFiltering ? mpLinearSampler : mpPointSampler);

        mpGraphicsState->setProgram(mpProgram);
        pRenderContext->setGraphicsState(mpGraphicsState);
        pRenderContext->setGraphicsVars(mpProgramVars);

        pScene->addModelResource(v, "");
    }
    pSceneRenderer->toggleMeshCulling(true);
    pSceneRenderer->renderScene(pRenderContext, mpCamera.get());
    //pSample->renderText(mModelString, glm::vec2(10, 30));
}

void ModelViewer::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    static ivec4 frameRect(300, 600, 500, 100);
    pGui->pushWindow("Model Viewer", frameRect.x, frameRect.y, frameRect.z, frameRect.w, false, true, true, false);
    // Load model group
    if (pGui->addButton("Load Model"))
    {
        openDialogLoadModel(pSample->getCurrentFbo()->getColorTexture(0)->getFormat());
    }
    if (pGui->beginGroup("Load Options"))
    {
        if (pGui->addButton("Delete Culled Meshes"))
        {
            for (auto& v : models)
                deleteCulledMeshes(v);
        }
        pGui->endGroup();
    }

    pGui->addSeparator();
    pGui->addCheckBox("Wireframe", mDrawWireframe);

    Gui::DropdownList cullList;
    cullList.push_back({ 0, "No Culling" });
    cullList.push_back({ 1, "Backface Culling" });
    cullList.push_back({ 2, "Frontface Culling" });
    pGui->addDropdown("Cull Mode", cullList, mCullMode);

    Gui::DropdownList blendList;
    blendList.push_back({ 0, "Normal" });
    blendList.push_back({ 1, "Transparent" });
    pGui->addDropdown("Blend Mode", blendList, mBlendMode);

    if (pGui->beginGroup("Lights"))
    {
        if (pGui->beginGroup("Directional Light"))
        {
            mpDirLight->renderUI(pGui);
            pGui->endGroup();
        }
        if (pGui->beginGroup("Point Light"))
        {
            mpPointLight->renderUI(pGui);
            pGui->endGroup();
        }
        pGui->endGroup();
    }

    Gui::DropdownList cameraDropdown;
    cameraDropdown.push_back({ ModelViewCamera, "Model-View" });
    cameraDropdown.push_back({ FirstPersonCamera, "First-Person" });
    cameraDropdown.push_back({ SixDoFCamera, "6 DoF" });

    pGui->addDropdown("Camera Type", cameraDropdown, (uint32_t&)mCameraType);

    //if (mpModel)
    //{
    //    renderModelUI(pGui);
    //}
    for (auto&v : models) {
        v.renderMaterialGui(pGui);
    }
    pGui->popWindow();

}
bool ModelViewer::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
    bool bHandled = getActiveCameraController().onKeyEvent(keyEvent);
    if (bHandled == false)
    {
        if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
        {
            switch (keyEvent.key)
            {
            case KeyboardEvent::Key::R:
                resetCamera();
                bHandled = true;
                break;
            }
        }
    }
    return bHandled;
}

bool ModelViewer::onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent)
{
    return getActiveCameraController().onMouseEvent(mouseEvent);
}

void ModelViewer::onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height)
{
    float h = (float)height;
    float w = (float)width;

    mpCamera->setFocalLength(21.0f);
    float aspectRatio = (w / h);
    mpCamera->setAspectRatio(aspectRatio);
}

ModelResource ModelViewer::loadModelFromFile(const std::string& filename, ResourceFormat fboFormat, bool animation, bool useLinearFilter)
{
    ModelResource r;
    CpuTimer timer;
    timer.update();

    Model::LoadFlags flags = Model::LoadFlags::None;

    flags |= isSrgbFormat(fboFormat) ? Model::LoadFlags::None : Model::LoadFlags::AssumeLinearSpaceTextures;
    r.mModelString = filename;
    r.mpModel = Model::createFromFile(filename.c_str(), flags);
    if (r.mpModel == nullptr)
    {
        msgBox("Could not load model");
        return r;
    }
    r.mUseTriLinearFiltering = useLinearFilter;
    r.mAnimate = animation;

    float radius = r.mpModel->getRadius();
    float lightHeight = max(1.0f + radius, radius*1.25f);
    mpPointLight->setWorldPosition(glm::vec3(0, lightHeight, 0));
    timer.update();

    mActiveAnimationID = kBindPoseAnimationID;

    r.mpModel->bindSamplerToMaterials(r.mUseTriLinearFiltering ? mpLinearSampler : mpPointSampler);
    r.init();
    return r;
}

void ModelViewer::openDialogLoadModel(ResourceFormat fboFormat)
{
    std::string Filename;
    if (openFileDialog(Model::kFileExtensionFilters, Filename))
    {
        models.emplace_back(loadModelFromFile(Filename, fboFormat));
        resetCamera();
    }
}

void ModelViewer::deleteCulledMeshes(ModelResource& mesh)
{
    if (mesh.mpModel)
    {
        CpuTimer timer;
        timer.update();
        mesh.mpModel->deleteCulledMeshes(mpCamera.get());
        timer.update();
    }
}

CameraController& ModelViewer::getActiveCameraController()
{
    switch (mCameraType)
    {
    case ModelViewer::ModelViewCamera:
        return mModelViewCameraController;
    case ModelViewer::FirstPersonCamera:
        return mFirstPersonCameraController;
    case ModelViewer::SixDoFCamera:
        return m6DoFCameraController;
    default:
        should_not_get_here();
        return m6DoFCameraController;
    }
}


void ModelViewer::resetCamera()
{
    if (!models.empty())
    {
        // update the camera position
        float Radius = models.begin()->mpModel->getRadius();
        const glm::vec3& ModelCenter = models.begin()->mpModel->getCenter();
        glm::vec3 CamPos = ModelCenter;
        CamPos.z += Radius * 5;

        mpCamera->setPosition(CamPos);
        mpCamera->setTarget(ModelCenter);
        mpCamera->setUpVector(glm::vec3(0, 1, 0));

        // Update the controllers
        mModelViewCameraController.setModelParams(ModelCenter, Radius, 3.5f);
        mFirstPersonCameraController.setCameraSpeed(Radius*0.25f);
        m6DoFCameraController.setCameraSpeed(Radius*0.25f);

        mNearZ = std::max(0.1f, models.begin()->mpModel->getRadius() / 750.0f);
        //mFarZ = Radius * 10;
    }
}

void ModelViewer::loadModelResources()
{
    auto arcade = loadModelFromFile("d:\\Falcor\\Media\\Arcade\\Arcade.fbx", ResourceFormat::RGBA8UnormSrgb);
    for (auto&v : arcade.sharedMaterials) {
        auto& m_wall = v.second;
        m_wall->insert_bool("gConstColor", false);
        m_wall->insert_vec4("gAmbient", glm::vec4(1.0f));
        m_wall->set_texture2D("gAlbedo", MaterialInstance::BlankTexture);
        //m_wall->getDefaultMaterial()->setBaseColorTexture(m_wall->get_texture2D("gAlbedo"));
    }
    auto teapot = loadModelFromFile("d:\\Falcor\\Media\\modelo\\SeeU Miniature.dae", ResourceFormat::RGBA8UnormSrgb);
    for (auto&v : teapot.sharedMaterials) {
        auto& m_wall = v.second;
        m_wall->insert_bool("gConstColor", false);
        m_wall->insert_vec4("gAmbient", glm::vec4(1.0f));
        m_wall->set_texture2D("gAlbedo", MaterialInstance::BlankTexture);
        //m_wall->getDefaultMaterial()->setBaseColorTexture(m_wall->get_texture2D("gAlbedo"));
    }
    models.emplace_back(arcade);
    models.emplace_back(teapot);
    resetCamera();
}
