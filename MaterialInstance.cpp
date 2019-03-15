#include "MaterialInstance.h"
#include "ModelViewer.h"


std::vector<DepthStencilState::SharedPtr> DepthStencilStateBundle::state = {};
bool DepthStencilStateBundle::operator==(const DepthStencilStateBundle& rhs)
{
    return bWriteDepth == rhs.bWriteDepth && eDepthTestFunc == rhs.eDepthTestFunc;
}

bool DepthStencilStateBundle::Get(const DepthStencilStateBundle& bundle, DepthStencilState::SharedPtr& ref)
{
    for (auto& v : state) {
        if (v->isDepthWriteEnabled() == bundle.bWriteDepth && v->getDepthFunc() == bundle.eDepthTestFunc && v->isDepthTestEnabled() == bundle.bDepthTest) {
            ref = v;
            return true;
        }
    }
    return false;
}

DepthStencilState::SharedPtr DepthStencilStateBundle::Get(const DepthStencilStateBundle& bundle)
{
    DepthStencilState::SharedPtr r;
    if (Get(bundle, r))return r;
    DepthStencilState::Desc desc;
    desc.setDepthTest(bundle.bDepthTest).setDepthFunc(bundle.eDepthTestFunc).setDepthWriteMask(bundle.bWriteDepth).setStencilTest(false);
    r = DepthStencilState::create(desc);
    state.push_back(r);
    return r;
}

const std::string MaterialInstance::vsEntry = "vert";
const std::string MaterialInstance::psEntry = "frag";

Texture::SharedPtr MaterialInstance::WhiteTexture = nullptr;
Texture::SharedPtr MaterialInstance::BlackTexture = nullptr;
Texture::SharedPtr MaterialInstance::RedTexture = nullptr;
Texture::SharedPtr MaterialInstance::GreenTexture = nullptr;
Texture::SharedPtr MaterialInstance::BlueTexture = nullptr;
Texture::SharedPtr MaterialInstance::pTextureNoise = nullptr;
Texture::SharedPtr MaterialInstance::pTextureNoiseRGB = nullptr;
Texture::SharedPtr MaterialInstance::pTextureSmoke = nullptr;
Texture::SharedPtr MaterialInstance::pTextureWoodFloor = nullptr;
RasterizerState::SharedPtr MaterialInstance::pRasterizerState_Wire = nullptr;
RasterizerState::SharedPtr MaterialInstance::pRasterizerState_Solid_Back = nullptr;
RasterizerState::SharedPtr MaterialInstance::pRasterizerState_Solid_Front = nullptr;
RasterizerState::SharedPtr MaterialInstance::pRasterizerState_Solid_None = nullptr;
BlendState::SharedPtr MaterialInstance::pBlenderState_Opaque = nullptr;
BlendState::SharedPtr MaterialInstance::pBlenderState_Transparent = nullptr;

const Gui::DropdownList MaterialInstance::kBlendModeDropDownList = { {uint32_t(EBlendMode::Opaque),"Opaque"}, {uint32_t(EBlendMode::Transparent),"Transparent"} };
const Gui::DropdownList MaterialInstance::kRasterizeModeDropDownList = { {uint32_t(ERasterizeMode::Wire),"Wire"},  {uint32_t(ERasterizeMode::CullNone),"CullNone"},  {uint32_t(ERasterizeMode::CullBack),"CullBack"}, {uint32_t(ERasterizeMode::Wire),"CullFront"} };
const Gui::DropdownList MaterialInstance::kDepthTestFuncDropDownList = { {uint32_t(ComparisonFunc::Disabled),"Disabled"},  {uint32_t(ComparisonFunc::Never),"Never"},  {uint32_t(ComparisonFunc::Always),"Always"}, {uint32_t(ComparisonFunc::Less),"Less"},  {uint32_t(ComparisonFunc::Equal),"Equal"},  {uint32_t(ComparisonFunc::NotEqual),"NotEqual"},  {uint32_t(ComparisonFunc::LessEqual),"LessEqual"},  {uint32_t(ComparisonFunc::Greater),"Greater"},  {uint32_t(ComparisonFunc::GreaterEqual),"GreaterEqual"} };

MaterialInstance::SharedPtr MaterialInstance::create(const std::string& shader, const Program::DefineList& programDefines, const std::string& _name)
{
    return std::make_shared<MaterialInstance>(shader, programDefines, _name);
}

bool MaterialInstance::ComaparsionQueue(const MaterialInstance* rhs)const
{
    if (this->renderQueue == rhs->renderQueue) return this->mResId < rhs->mResId;
    return this->renderQueue < rhs->renderQueue;
}

MaterialInstance::MaterialInstance(const std::string & shader, const Program::DefineList & programDefines, const std::string & _name) :mName(_name)
{
    mResId = ModelResource::getModelCount();
    loadStaticData();
    mpState = GraphicsState::create();
    mpProgram = GraphicsProgram::createFromFile(shader, vsEntry, psEntry, programDefines);
    mpState->setProgram(mpProgram);
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());

    mpDepthStencilState = DepthStencilStateBundle::Get(depthStencilBundle);

    mpState->setDepthStencilState(mpDepthStencilState);
    set_blendMode(EBlendMode::Opaque);
    set_rasterizeMode(ERasterizeMode::CullBack);

}

MaterialInstance::MaterialInstance(const std::string & _name, const  Material::SharedPtr & material) : mName(_name), mpMaterial(material)
{
    mResId = ModelResource::getModelCount();
    loadStaticData();
    mpState = GraphicsState::create();
    mpProgram = ModelViewer::getProgramMap().at("Diffuse");
    mpState->setProgram(mpProgram);
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());

    mpDepthStencilState = DepthStencilStateBundle::Get(depthStencilBundle);

    mpState->setDepthStencilState(mpDepthStencilState);
    set_blendMode(EBlendMode::Opaque);
    set_rasterizeMode(ERasterizeMode::CullBack);
}

void MaterialInstance::clear()
{
#define CLEAR_MAP(type) param_##type##.clear()
    CLEAR_MAP(bool);
    CLEAR_MAP(int);
    CLEAR_MAP(ivec2);
    CLEAR_MAP(ivec3);
    CLEAR_MAP(ivec4);
    CLEAR_MAP(float);
    CLEAR_MAP(vec2);
    CLEAR_MAP(vec3);
    CLEAR_MAP(vec4);
    CLEAR_MAP(mat2);
    CLEAR_MAP(mat3);
    CLEAR_MAP(mat4);
    CLEAR_MAP(texture2D);

}
inline ConstantBuffer::SharedPtr MaterialInstance::get_constantbuffer(ECBType t)
{
    switch (t) {
    case ECBType::PerFrame:return mpProgramVars["DCB"];
    case ECBType::Static:return mpProgramVars["SCB"];
    }
    return mpProgramVars["DCB"];
};
void MaterialInstance::onMaterialGui(Gui * p)
{
    int i = 0;
#define PRE_INDEX_NAME  (v.first + ":" + get_resName())
#define ADD_GUI_VEC(vec,var_type)\
    for (auto &v : param_##vec) {\
        std::string a=PRE_INDEX_NAME+std::string("(")+#vec+std::string(")");\
        p->add##var_type##Var(a.c_str(), v.second);\
    }
#define ADD_GUI_MAT(mat)\
    for (auto &v : param_##mat) {\
        std::string a=PRE_INDEX_NAME+std::string("(")+#mat+std::string(")");\
        p->addMatrixVar<mat>(a.c_str(), v.second);\
    }
    //Render Queue 
    std::string renderQueueS = get_resName() + std::string("-Render Queue");
    p->addIntSlider(renderQueueS.c_str(), renderQueue, 0, 5000);
    //Blend Mode
    std::string blendS = get_resName() + std::string("-Blend Mode");
    uint32_t blendModeUint = uint32_t(blendMode);
    if (p->addDropdown(blendS.c_str(), kBlendModeDropDownList, blendModeUint)) {
        blendMode = EBlendMode(blendModeUint);
        set_blendMode(blendMode);
    }
    //Rasterize Mode
    std::string rasterizeS = get_resName() + std::string("-Rasterize Mode");
    uint32_t rastModeUint = uint32_t(rasterizeMode);
    if (p->addDropdown(rasterizeS.c_str(), kRasterizeModeDropDownList, rastModeUint)) {
        rasterizeMode = ERasterizeMode(rastModeUint);
        set_rasterizeMode(rasterizeMode);
    }
    bool dirty = false;
    std::string depthTestS = get_resName() + std::string("-Depth Test");
    if (p->addCheckBox(depthTestS.c_str(), depthStencilBundle.bDepthTest)) dirty = true;
    std::string writeDepthS = get_resName() + std::string("-Write Depth");
    if (p->addCheckBox(writeDepthS.c_str(), depthStencilBundle.bWriteDepth)) dirty = true;
    std::string depthTestFuncS = get_resName() + std::string("-DepthTest Func");
    uint32_t depthTestFuncUint = uint32_t(depthStencilBundle.eDepthTestFunc);
    if (p->addDropdown(depthTestFuncS.c_str(), kDepthTestFuncDropDownList, depthTestFuncUint)) {
        depthStencilBundle.eDepthTestFunc = ComparisonFunc(depthTestFuncUint);
        dirty = true;
    }
    if (dirty) {
        set_depthStencilTest(depthStencilBundle);
    }
    ADD_GUI_VEC(float, Float);
    ADD_GUI_VEC(vec2, Float2);
    //ADD_GUI_VEC(vec3, Float3);
    //ADD_GUI_VEC(vec4, Float4);
    for (auto& v : param_vec3) {
        std::string a = PRE_INDEX_NAME + std::string("(vec3)");
        p->addFloat3Var(a.c_str(), v.second);
        p->addRgbColor((a + ":color").c_str(), v.second);
    }
    for (auto& v : param_vec4) {
        std::string a = PRE_INDEX_NAME + std::string("(vec4)");
        p->addFloat4Var(a.c_str(), v.second);
        p->addRgbaColor((a + ":color").c_str(), v.second);
    }
    ADD_GUI_VEC(int, Int);
    ADD_GUI_VEC(ivec2, Int2);
    ADD_GUI_VEC(ivec3, Int3);
    ADD_GUI_VEC(ivec4, Int4);
    ADD_GUI_MAT(mat2);
    ADD_GUI_MAT(mat3);
    ADD_GUI_MAT(mat4);

    for (auto& v : param_bool) {
        p->addCheckBox(PRE_INDEX_NAME.c_str(), v.second);
    }

    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"jpeg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    for (auto& v : param_texture2D) {
        if (p->addImageButton("", v.second == nullptr ? WhiteTexture : v.second, glm::vec2(32, 32), false)) {
            if (openFileDialog(filters, filename))
            {
                v.second = createTextureFromFile(filename, false, true);
            }
        }

        //if (p->addButton("White"))  v.second = WhiteTexture;
        //if (p->addButton("Black", true))  v.second = BlackTexture;
        //if (p->addButton("Red"),true)  v.second = RedTexture;
        //if (p->addButton("Green", true))  v.second = GreenTexture;
        //if (p->addButton("Blue", true))  v.second = BlueTexture;
        p->addText(v.first.c_str(), true);
    }
    if (bUseMaterial) {
#define ADD_MATERIAL_TEXTURE(Text,FunctionName,DefaultTexture)auto& FunctionName##ref=mpMaterial->get##FunctionName();\
        if(FunctionName##ref==nullptr){mpMaterial->set##FunctionName(DefaultTexture); return;}\
        if (p->addImageButton("",FunctionName##ref , glm::vec2(32, 32), false)) {\
            if (openFileDialog(filters, filename))mpMaterial->set##FunctionName(createTextureFromFile(filename, false, true));\
        }\
        p->addText(#Text,true);
        /*if (p->addButton("White")) { mpMaterial->set##FunctionName(WhiteTexture);return;}\
        if (p->addButton("Black",true)) { mpMaterial->set##FunctionName(BlackTexture); return;}\
        if (p->addButton("Red",true)) { mpMaterial->set##FunctionName(RedTexture); return;}\
        if (p->addButton("Green",true)) { mpMaterial->set##FunctionName(GreenTexture); return;}\
        if (p->addButton("Blue",true)) { mpMaterial->set##FunctionName(BlueTexture);  return;}*/
        std::string a = get_resName() + std::string("Shader Model Spec");
        bool isSpecGloss = mpMaterial->getShadingModel() == ShadingModelSpecGloss;
        if (p->addCheckBox(a.c_str(), isSpecGloss)) {
            mpMaterial->setShadingModel(isSpecGloss ? ShadingModelSpecGloss : ShadingModelMetalRough);
        }
        ADD_MATERIAL_TEXTURE(Albedo, BaseColorTexture, WhiteTexture);
        ADD_MATERIAL_TEXTURE(Specular, SpecularTexture, BlackTexture);
        ADD_MATERIAL_TEXTURE(NormalMap, NormalMap, WhiteTexture);
        ADD_MATERIAL_TEXTURE(Emissive, EmissiveTexture, BlackTexture);
        ADD_MATERIAL_TEXTURE(OcclusionMap, OcclusionMap, BlackTexture);
        //ADD_MATERIAL_TEXTURE(LightMap, LightMap);
        //ADD_MATERIAL_TEXTURE(HeightMap, HeightMap);
    }
}

void MaterialInstance::onRender(RenderContext * pRenderContext)
{
    //mpState->setBlendState(pRenderContext->getGraphicsState()->getBlendState());
    //mpState->setRasterizerState(pRenderContext->getGraphicsState()->getRasterizerState());
    //mpState->setDepthStencilState(pRenderContext->getGraphicsState()->getDepthStencilState());

    for (auto& v : param_texture2D) {
        mpProgramVars->setTexture(v.first, v.second);
    }
    auto& cb = get_constantbuffer(ECBType::PerFrame);
#define SET_CONSTANT_BUFFER(type)\
    for (auto&v : bind_##type) {v.execute();}\
    for (auto&v : param_##type) { cb[v.first] = v.second;}

    SET_CONSTANT_BUFFER(bool);
    SET_CONSTANT_BUFFER(int);
    SET_CONSTANT_BUFFER(ivec2);
    SET_CONSTANT_BUFFER(ivec3);
    SET_CONSTANT_BUFFER(ivec4);
    SET_CONSTANT_BUFFER(float);
    SET_CONSTANT_BUFFER(vec2);
    SET_CONSTANT_BUFFER(vec3);
    SET_CONSTANT_BUFFER(vec4);
    SET_CONSTANT_BUFFER(mat2);
    SET_CONSTANT_BUFFER(mat3);
    SET_CONSTANT_BUFFER(mat4);

    mpState->setProgram(mpProgram);
    pRenderContext->setGraphicsState(mpState);
    pRenderContext->setGraphicsVars(mpProgramVars);
}

void MaterialInstance::loadStaticData()
{
    if (WhiteTexture == nullptr) {
        std::vector<uint8_t> blankData = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
        WhiteTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData.data());
        blankData = { 0,0,0,255, 0,0,0,255, 0,0,0,255, 0,0,0,255 };
        BlackTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData.data());
        blankData = { 255,0,0,255, 255,0,0,255, 255,0,0,255, 255,0,0,255 };
        RedTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData.data());
        blankData = { 0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255 };
        GreenTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData.data());
        blankData = { 0,0,255,255, 0,0,255,255, 0,0,255,255, 0,0,255,255 };
        BlueTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData.data());

        pTextureSmoke = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\smoke.jpg", false, true);
        pTextureNoise = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\noise1024.png", false, true);
        pTextureNoiseRGB = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\noisergb.jpg", false, true);
        pTextureWoodFloor = createTextureFromFile("d:\\Falcor\\Samples\\Core\\MultiPassPostProcess\\Media\\wood_floor.jpg", false, true);

        // create rasterizer state
        RasterizerState::Desc wireframeDesc;
        wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
        wireframeDesc.setCullMode(RasterizerState::CullMode::None);
        pRasterizerState_Wire = RasterizerState::create(wireframeDesc);

        RasterizerState::Desc solidDesc;
        solidDesc.setCullMode(RasterizerState::CullMode::None);
        pRasterizerState_Solid_None = RasterizerState::create(solidDesc);
        solidDesc.setCullMode(RasterizerState::CullMode::Back);
        pRasterizerState_Solid_Back = RasterizerState::create(solidDesc);
        solidDesc.setCullMode(RasterizerState::CullMode::Front);
        pRasterizerState_Solid_Front = RasterizerState::create(solidDesc);

        BlendState::Desc blendDesc;
        pBlenderState_Opaque = BlendState::create(blendDesc);
        blendDesc.setAlphaToCoverage(false);
        blendDesc.setRenderTargetWriteMask(0, true, true, true, true);

        blendDesc.setRtBlend(0, true).setRtParams(0, BlendState::BlendOp::Add, BlendState::BlendOp::Add,
            BlendState::BlendFunc::SrcAlpha, BlendState::BlendFunc::OneMinusSrcAlpha,
            BlendState::BlendFunc::One, BlendState::BlendFunc::Zero);
        pBlenderState_Transparent = BlendState::create(blendDesc);
    }
}

MaterialInstance& MaterialInstance::set_program(const GraphicsProgram::SharedPtr & prog, const std::string & shaderName, bool use_default_material, uint32_t shaderModel, bool resetState)
{
    if (!use_default_material) { bUseMaterial = false; }
    clear(); mpProgram = prog;
    mShaderName = shaderName;
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    if (use_default_material && mpMaterial) {
        bUseMaterial = use_default_material;
        mpMaterial->setShadingModel(shaderModel);
    }
    if (resetState) {
        //在添加了DepthStencil的面板之后就不需要这些了
        //set_blendMode(EBlendMode::Opaque);
        //set_rasterizeMode(ERasterizeMode::CullBack);
        const static DepthStencilStateBundle defaultBundle;
        set_depthStencilTest(defaultBundle);
    }
    return *this;
}

MaterialInstance& MaterialInstance::set_blendMode(EBlendMode mode)
{
    blendMode = mode;
    switch (mode)
    {
    case MaterialInstance::EBlendMode::Opaque:
        mpState->setBlendState(pBlenderState_Opaque);
        break;
    case MaterialInstance::EBlendMode::Transparent:
        mpState->setBlendState(pBlenderState_Transparent);
        break;
    default:
        break;
    }
    return *this;
}

MaterialInstance& MaterialInstance::set_rasterizeMode(ERasterizeMode mode)
{
    rasterizeMode = mode;
    switch (mode)
    {
    case MaterialInstance::ERasterizeMode::Wire:
        mpState->setRasterizerState(pRasterizerState_Wire);
        break;
    case MaterialInstance::ERasterizeMode::CullNone:
        mpState->setRasterizerState(pRasterizerState_Solid_None);
        break;
    case MaterialInstance::ERasterizeMode::CullBack:
        mpState->setRasterizerState(pRasterizerState_Solid_Back);
        break;
    case MaterialInstance::ERasterizeMode::CullFront:
        mpState->setRasterizerState(pRasterizerState_Solid_Front);
        break;
    default:
        break;
    }
    return *this;
}

MaterialInstance& MaterialInstance::set_depthStencilTest(const DepthStencilStateBundle & bundle)
{
    mpState->setDepthStencilState(DepthStencilStateBundle::Get(bundle));
    return *this;
}
