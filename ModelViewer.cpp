#include "ModelViewer.h"
#include "MultiPassPostProcess.h"

std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>> ModelViewer::mMaterialFuncMap;
std::map<std::string, GraphicsProgram::SharedPtr> ModelViewer::mProgramMap;
const Gui::DropdownList ModelViewer::kSkyBoxDropDownList = { { HdrImage::EveningSun, "Evening Sun" },
                                                    { HdrImage::AtTheWindow, "Window" },
                                                    { HdrImage::OvercastDay, "Overcast Day" },
                                                    { HdrImage::DayTime, "Day Time" },
                                                    { HdrImage::Rathaus, "Rathaus" } };

ModelViewer::ModelViewer() : mNearZ(0.1f), mFarZ(1000.0f)
{
}


ModelViewer::~ModelViewer()
{
}

void ModelViewer::onLoad(SampleCallbacks* ppSample, RenderContext* pRenderContext)
{
    pSample = ppSample;
    mpCamera = Camera::create();
    mpProgram = GraphicsProgram::createFromFile("Diffuse.hlsl", "", "frag");
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());

    mpGraphicsState = GraphicsState::create();
    mpGraphicsState->setProgram(mpProgram);

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
    {
        DepthStencilState::Desc dsDesc;
        dsDesc.setDepthTest(false);
        mpNoDepthDS = DepthStencilState::create(dsDesc);
        dsDesc.setDepthTest(true).setStencilTest(false).setDepthWriteMask(true).setDepthFunc(DepthStencilState::Func::Less);
        mpDepthTestLessDS = DepthStencilState::create(dsDesc);
        dsDesc.setDepthFunc(DepthStencilState::Func::Greater);
        mpDepthTestGreaterDS = DepthStencilState::create(dsDesc);
    }
    {
        DepthStencilState::Desc dsDesc;
        dsDesc.setDepthFunc(DepthStencilState::Func::Always);
        mpDepthTestAlways = DepthStencilState::create(dsDesc);
    }

    initDepthPass();

    uint32_t w = ppSample->getWindow()->getClientAreaWidth();
    uint32_t h = ppSample->getWindow()->getClientAreaHeight();

    // Common FBO desc (2 color outputs - color and normal)
    Fbo::Desc fboDesc;
    fboDesc.setColorTarget(0, ResourceFormat::RGBA32Float).setColorTarget(1, ResourceFormat::RGBA8Unorm).setDepthStencilTarget(ResourceFormat::D32Float);
    mpDepthPassFbo = FboHelper::create2D(w, h, fboDesc);

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

    mpScene = std::make_shared<SceneExtend>("");
    mpScene->addLight(mpDirLight);
    mpScene->addLight(mpPointLight);

    loadSkyBox();
    loadShaderProgram();
    loadMaterialFunctions();
    loadModelResources();
}

void ModelViewer::onFrameRender(SampleCallbacks * pSample, RenderContext * pRenderContext, const Fbo::SharedPtr & pTargetFbo)
{
    pSceneRenderer = SceneRendererExtend::create(mpScene);
    const glm::vec4 clearColor(0, 0, 0, 0);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    pRenderContext->clearFbo(mpDepthPassFbo.get(), glm::vec4(0), 1.0f, 0, FboAttachmentType::All);

    for (auto& v : mpScene->getModels())
    {
        // Animate
        if (v.mAnimate)
        {
            PROFILE("animate");
            v.mpModel->animate(pSample->getCurrentTime());
        }
        // Set render state
        v.mpModel->bindSamplerToMaterials(v.mUseTriLinearFiltering ? mpLinearSampler : mpPointSampler);
    }

    mpGraphicsState->setFbo(mpDepthPassFbo);
    mpGraphicsState->setProgram(mDepthPass.pProgram);
    mpGraphicsState->setRasterizerState(mpCullRastState[1]);
    mpGraphicsState->setDepthStencilState(mpDepthTestLessDS);
    pRenderContext->setGraphicsVars(mDepthPass.pVars);
    pRenderContext->setGraphicsState(mpGraphicsState);

    mpCamera->setDepthRange(mNearZ, mFarZ);
    getActiveCameraController().update();
    pSceneRenderer->update(pSample->getCurrentTime());
    pSceneRenderer->toggleMeshCulling(true);
    pSceneRenderer->renderScene(pRenderContext, mpDepthPassFbo, mpCamera.get(), false);

    pRenderContext->copyResource(PostProcessBase::gDepthTexture.get(), mpDepthPassFbo->getDepthStencilTexture().get());
    pTargetFbo->attachDepthStencilTarget(mpDepthPassFbo->getDepthStencilTexture());

    //mpDepthPassFbo->attachDepthStencilTarget(pTargetFbo->getDepthStencilTexture());
    //Sleep(300);
    //pTargetFbo->getDepthStencilTexture()->captureToFile(0, 0, "d:/cap.png");

    if (mpSkyBox) {
        mpGraphicsState->setDepthStencilState(mpDepthTestAlways);
        mpGraphicsState->setBlendState(mpBlendState[1]);
        mpSkyBox->render(pRenderContext, mpCamera.get(), pTargetFbo);
        pRenderContext->setGraphicsState(nullptr);
    }
    pSceneRenderer->renderScene(pRenderContext, pTargetFbo, mpCamera.get(), true);
}

void ModelViewer::onGuiRender(SampleCallbacks * pSample, Gui * pGui)
{
    static ivec4 frameRect(300, 600, 500, 100);
    pGui->pushWindow("Model Viewer", frameRect.x, frameRect.y, frameRect.z, frameRect.w, true, true, true, false);
    // Load SkyBox
    uint32_t uHdrIndex = static_cast<uint32_t>(mHdrImageIndex);
    if (pGui->addDropdown("HdrImage", kSkyBoxDropDownList, uHdrIndex))
    {
        mHdrImageIndex = static_cast<HdrImage>(uHdrIndex);
        loadSkyBox();
    }
    if (pGui->addButton("Save Scene"))
    {
        std::string filename;
        if (saveFileDialog(Scene::kFileExtensionFilters, filename))
        {
            mpScene->save(filename);
        }
    }
    if (pGui->addButton("Load Scene"))
    {
        std::string Filename;
        if (openFileDialog(Scene::kFileExtensionFilters, Filename))
        {
            mpScene->load(Filename); //Scene::loadFromFile(Filename, Model::LoadFlags::None, Scene::LoadFlags::None);
        }
    }
    // Load Model
    if (pGui->addButton("Load Model"))
    {
        openDialogLoadModel(pSample->getCurrentFbo()->getColorTexture(0)->getFormat());
    }

    if (pGui->beginGroup("Load Options"))
    {
        if (pGui->addButton("Delete Culled Meshes"))
        {
            for (auto& v : mpScene->getModels())
                deleteCulledMeshes(v);
        }
        pGui->endGroup();
    }

    pGui->addSeparator();
    // Set Render Model
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
    auto& mModels = mpScene->getModels();
    bool dirty = false;
    for (auto& v = mModels.begin(); v != mModels.end(); v++) {
        if (pGui->addButton(("Remove " + v->getModelResName()).c_str())) {
            mModels.erase(v);
            dirty = true;
            mpScene->reset();
            break;
        }
        v->onGui(pGui);
    }
    //if(dirty)
    pGui->popWindow();
}

void ModelViewer::loadModel(const std::string & Filename, ResourceFormat fboFormat, bool animation, bool useLinearFilter)
{
    ModelResource res;
    if (loadModelFromFile(res, Filename, fboFormat)) {
        mpScene->addModelResource(res);
        mpScene->reset();
        resetCamera();
    }
}

bool ModelViewer::onKeyEvent(SampleCallbacks * pSample, const KeyboardEvent & keyEvent)
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
            case KeyboardEvent::Key::C:
                pSample->getCurrentFbo()->getDepthStencilTexture()->captureToFile(0, 0, "d:/cap.png");
                return true;
            }
        }
    }
    return bHandled;
}

bool ModelViewer::onMouseEvent(SampleCallbacks * pSample, const MouseEvent & mouseEvent)
{
    return getActiveCameraController().onMouseEvent(mouseEvent);
}

void ModelViewer::onResizeSwapChain(SampleCallbacks * pSample, uint32_t width, uint32_t height)
{
    float h = (float)height;
    float w = (float)width;

    mpCamera->setFocalLength(21.0f);
    float aspectRatio = (w / h);
    mpCamera->setAspectRatio(aspectRatio);
}

void ModelViewer::initDepthPass()
{
    mDepthPass.pProgram = GraphicsProgram::createFromFile("DepthPass.hlsl", "", "main");
    mDepthPass.pVars = GraphicsVars::create(mDepthPass.pProgram->getReflector());
}

bool ModelViewer::loadModelFromFile(ModelResource & r, const std::string & filename, ResourceFormat fboFormat, bool animation, bool useLinearFilter)
{
    Model::LoadFlags flags = Model::LoadFlags::None;

    flags |= isSrgbFormat(fboFormat) ? Model::LoadFlags::None : Model::LoadFlags::AssumeLinearSpaceTextures;
    r.mModelString = filename;
    r.mpModel = Model::createFromFile(filename.c_str(), flags);
    if (r.mpModel == nullptr)
    {
        return false;
    }
    r.mUseTriLinearFiltering = useLinearFilter;
    r.mAnimate = animation;

    r.init("Diffuse");
    return true;
}

void ModelViewer::openDialogLoadModel(ResourceFormat fboFormat)
{
    std::string Filename;
    if (openFileDialog(Model::kFileExtensionFilters, Filename))
    {
        ModelResource res;
        if (loadModelFromFile(res, Filename, fboFormat)) {
            mpScene->addModelResource(res);
            mpScene->reset();
            resetCamera();
        }
    }
}

void ModelViewer::deleteCulledMeshes(ModelResource & mesh)
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
    if (!mpScene->empty())
    {
        // update the camera position
        float Radius = mpScene->getModel(0)->getRadius();
        const glm::vec3& ModelCenter = mpScene->getModel(0)->getCenter();
        glm::vec3 CamPos = ModelCenter;
        CamPos.z += Radius * 5;

        mpCamera->setPosition(CamPos);
        mpCamera->setTarget(ModelCenter);
        mpCamera->setUpVector(glm::vec3(0, 1, 0));

        mpCamera->setDepthRange(0.1f, Radius * 10);
        // Update the controllers
        mModelViewCameraController.setModelParams(ModelCenter, Radius, 3.5f);
        mFirstPersonCameraController.setCameraSpeed(Radius * 0.25f);
        m6DoFCameraController.setCameraSpeed(Radius * 0.25f);

        //mNearZ = std::max(0.1f, models.begin()->mpModel->getRadius() / 750.0f);
        //mFarZ = Radius * 10;
    }
}

void ModelViewer::loadSkyBox() {

    std::string filename;
    switch (mHdrImageIndex)
    {
    case HdrImage::AtTheWindow:
        filename = "LightProbes/20060807_wells6_hd.hdr";
        break;
    case HdrImage::EveningSun:
        filename = "LightProbes/hallstatt4_hd.hdr";
        break;
    case HdrImage::OvercastDay:
        filename = "LightProbes/20050806-03_hd.hdr";
        break;
    case HdrImage::DayTime:
        filename = "LightProbes/daytime.hdr";
        break;
    case HdrImage::Rathaus:
        filename = "LightProbes/rathaus.hdr";
        break;
    }

    mHdrImage = createTextureFromFile(filename, false, false, Resource::BindFlags::ShaderResource);
    mpSkyBox = SkyBox::create(mHdrImage, mpLinearSampler);
}

void ModelViewer::loadShaderProgram()
{
    const char* psEntry = "frag";
    const char* vsEntry = "vert";
    mProgramMap.emplace("Standard", GraphicsProgram::createFromFile("Standard.hlsl", "", psEntry));
    mProgramMap.emplace("Diffuse", GraphicsProgram::createFromFile("Diffuse.hlsl", "", psEntry));
    mProgramMap.emplace("ConstColor", GraphicsProgram::createFromFile("ConstColor.hlsl", "", psEntry));
    mProgramMap.emplace("UnLit", GraphicsProgram::createFromFile("UnLit.hlsl", "", psEntry));
    mProgramMap.emplace("VertDistortion", GraphicsProgram::createFromFile("VertDistortion.hlsl", vsEntry, psEntry));
    mProgramMap.emplace("Dissolve", GraphicsProgram::createFromFile("Dissolve.hlsl", "", psEntry));
    mProgramMap.emplace("FlashEffect", GraphicsProgram::createFromFile("FlashEffect.hlsl", "", psEntry));
}

void ModelViewer::loadMaterialFunctions()
{
#define INPUT_SHADER(x) mProgramMap[#x],#x
    mMaterialFuncMap.emplace("Standard", [this](MaterialInstance::SharedPtr & m) {
        m->set_program(INPUT_SHADER(Standard));
        m->set_texture2D("gAlbedoTexture", MaterialInstance::WhiteTexture);
        m->set_texture2D("gNormalTexture", MaterialInstance::BlackTexture);
        m->set_texture2D("gMentalnessTexture", MaterialInstance::BlackTexture);
        m->set_texture2D("gRoughnessTexture", MaterialInstance::BlackTexture);
        m->set_textureCube("gSpecularTexture", MaterialInstance::BlackTexture);
        m->set_textureCube("gIrradianceTexture", MaterialInstance::BlackTexture);
        m->set_texture2D("gSpecularBRDF_LUT", MaterialInstance::BlackTexture);
    });
    mMaterialFuncMap.emplace("Diffuse", [this](MaterialInstance::SharedPtr & m) {
        m->set_program(INPUT_SHADER(Diffuse), true);
        m->insert_bool("gConstColor", false);
        m->insert_vec4("gBaseColor", glm::vec4(1.0f)); });

    mMaterialFuncMap.emplace("ConstColor", [this](MaterialInstance::SharedPtr & m) {
        DepthStencilStateBundle bundle;
        bundle.eDepthTestFunc = DepthStencilState::Func::LessEqual;
        bundle.bWriteDepth = true;
        m->set_program(INPUT_SHADER(ConstColor)).set_depthStencilTest(bundle);//.set_rasterizeMode(MaterialInstance::ERasterizeMode::Wire);
        m->insert_vec4("gBaseColor", glm::vec4(0.6f, 0.8f, 1.0f, 1.0f)); });

    mMaterialFuncMap.emplace("UnLit", [this](MaterialInstance::SharedPtr & m) {
        DepthStencilStateBundle bundle;
        bundle.eDepthTestFunc = DepthStencilState::Func::Greater;
        bundle.bWriteDepth = false;
        m->set_program(INPUT_SHADER(UnLit)).set_depthStencilTest(bundle).set_blendMode(MaterialInstance::EBlendMode::Transparent);
        m->insert_vec4("gBaseColor", glm::vec4(1.f, 1.f, 1.0f, 1.0f));
        m->set_texture2D("gAlbedoTexture", MaterialInstance::BlackTexture); });

    mMaterialFuncMap.emplace("VertDistortion", [this](MaterialInstance::SharedPtr & m) {
        m->set_program(INPUT_SHADER(VertDistortion), true);
        m->insert_vec3("gDistortionDir", glm::vec3(1.f, 10.f, 1.0f));
        m->bind(m->insert_float("gTime", 0.0f), std::bind(&SampleCallbacks::getCurrentTime, pSample));
    });

    mMaterialFuncMap.emplace("Dissolve", [this](MaterialInstance::SharedPtr & m) {
        m->set_program(INPUT_SHADER(Dissolve), true);
        m->insert_float("gDissolve", .0f);
        m->set_texture2D("gDissolveTexture", MaterialInstance::pTextureSmoke); });

    mMaterialFuncMap.emplace("FlashEffect", [this](MaterialInstance::SharedPtr & m) {
        m->set_program(INPUT_SHADER(FlashEffect));
        m->set_texture2D("gAlbedo", MaterialInstance::pTextureWoodFloor);
        m->set_texture2D("gFlashTexture", MaterialInstance::pTextureNoiseRGB);
        m->insert_vec4("gFlashColor", glm::vec4(1));
        m->insert_vec4("gFlashFactor", glm::vec4(1));
        m->insert_float("gFlashStrength", 1);
        m->bind(m->insert_float("gTime", 0), std::bind(&SampleCallbacks::getCurrentTime, pSample));
        m->insert_vec3("gLightDir", glm::vec3(1));
    });
}
void ModelViewer::loadModelResources()
{
    //必须要将quad放在人物前面渲染，后面的人物才可以将被遮挡的部分进行显示出来,原因在于必须先渲染这个才会写入深度导致后续的Greater测试成功
    {
        ModelResource cynthia;
        loadModelFromFile(cynthia, "d:\\Falcor\\Media\\Cynthia\\Cynthia.obj", ResourceFormat::RGBA8UnormSrgb);
        for (auto& v : cynthia.sharedMaterials) {
            mMaterialFuncMap["UnLit"](v.second);
        }
        cynthia.Translation = glm::vec3(-100, 0, 0);
        cynthia.Rotation = glm::vec3(0, 1, 0);
        cynthia.Scale = glm::vec3(10);
        auto albedo = createTextureFromFile("d:\\Falcor\\Media\\Cynthia\\textures\\texture_1.jpg", false, true);
        cynthia.sharedMaterials["material_0"]->set_texture2D("gAlbedoTexture", albedo);
        cynthia.resetMaterialGui();
        mpScene->addModelResource(cynthia);
    }
    {
        ModelResource quad;
        loadModelFromFile(quad, "d:\\Falcor\\Media\\quad1x1.obj", ResourceFormat::RGBA8UnormSrgb);
        for (auto& v : quad.sharedMaterials) {
            mMaterialFuncMap["ConstColor"](v.second);
        }
        quad.Rotation = glm::vec3(0.258f, -0.243f, -0.127f);
        quad.Scale = glm::vec3(10);
        quad.resetMaterialGui();
        mpScene->addModelResource(quad);
    }
    {
        ModelResource miniature;
        loadModelFromFile(miniature, "d:\\Falcor\\Media\\modelo\\SeeU Miniature.dae", ResourceFormat::RGBA8UnormSrgb);
        for (auto& v : miniature.sharedMaterials) {
            auto& m_wall = v.second;
            mMaterialFuncMap["UnLit"](m_wall);
        }
        miniature.resetMaterialGui();
        mpScene->addModelResource(miniature);
    }
    //mpScene->reset();
    resetCamera();
}
