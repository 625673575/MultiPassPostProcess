#include "MaterialInstance.h"
#include "ModelViewer.h"
const std::string MaterialInstance::vsEntry = "vert";
const std::string MaterialInstance::psEntry = "frag";

Texture::SharedPtr MaterialInstance::WhiteTexture = nullptr;
Texture::SharedPtr MaterialInstance::BlackTexture = nullptr;
Texture::SharedPtr MaterialInstance::RedTexture = nullptr;
Texture::SharedPtr MaterialInstance::GreenTexture = nullptr;
Texture::SharedPtr MaterialInstance::BlueTexture = nullptr;
MaterialInstance::SharedPtr MaterialInstance::create(const std::string & shader, const Program::DefineList & programDefines, const std::string& _name)
{
    return std::make_shared<MaterialInstance>(shader, programDefines, _name);
}

MaterialInstance::MaterialInstance(const std::string & shader, const Program::DefineList& programDefines, const std::string& _name) :mName(_name)
{
    mpProgram = GraphicsProgram::createFromFile(shader, vsEntry, psEntry, programDefines);
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
}
MaterialInstance::MaterialInstance(const std::string& _name, const  Material::SharedPtr& material) : mName(_name), mpMaterial(material)
{

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
void MaterialInstance::onMaterialGui(Gui *p)
{
    int i = 0;
    if (WhiteTexture == nullptr) {
        const uint8_t blankData[16] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
        WhiteTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
    if (BlackTexture == nullptr) {
        const uint8_t blankData[16] = { 0,0,0,255, 0,0,0,255, 0,0,0,255, 0,0,0,255 };
        BlackTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
    if (RedTexture == nullptr) {
        const uint8_t blankData[16] = { 255,0,0,255, 255,0,0,255, 255,0,0,255, 255,0,0,255 };
        RedTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
    if (GreenTexture == nullptr) {
        const uint8_t blankData[16] = { 0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255 };
        GreenTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
    if (BlueTexture == nullptr) {
        const uint8_t blankData[16] = { 0,0,255,255, 0,0,255,255, 0,0,255,255, 0,0,255,255 };
        BlueTexture = Texture::create2D(2, 2, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
#define PRE_INDEX_NAME  (v.first + ":" + mName)
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
    ADD_GUI_VEC(float, Float);
    ADD_GUI_VEC(vec2, Float2);
    //ADD_GUI_VEC(vec3, Float3);
    //ADD_GUI_VEC(vec4, Float4);
    for (auto&v : param_vec3) {
        std::string a = PRE_INDEX_NAME + std::string("(vec3)");
        p->addFloat3Var(a.c_str(), v.second);
        p->addRgbColor((a + ":color").c_str(), v.second);
    }
    for (auto&v : param_vec4) {
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

    for (auto &v : param_bool) {
        p->addCheckBox(PRE_INDEX_NAME.c_str(), v.second);
    }

    std::string filename;
    FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"jpeg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
    for (auto&v : param_texture2D) {
        if (p->addImageButton(PRE_INDEX_NAME.c_str(), v.second == nullptr ? WhiteTexture : v.second, glm::vec2(128, 128), true)) {
            if (openFileDialog(filters, filename))
            {
                v.second = createTextureFromFile(filename, false, true);
            }
        }
        if (p->addButton("White"))  v.second = WhiteTexture;
        if (p->addButton("Black", true))  v.second = BlackTexture;
        if (p->addButton("Red"),true)  v.second = RedTexture;
        if (p->addButton("Green", true))  v.second = GreenTexture;
        if (p->addButton("Blue", true))  v.second = BlueTexture;
        p->addText(v.first.c_str(), true);
    }
    if (bUseMaterial) {
#define ADD_MATERIAL_TEXTURE(Text,FunctionName,DefaultTexture)auto& FunctionName##ref=mpMaterial->get##FunctionName();\
        if(FunctionName##ref==nullptr){mpMaterial->set##FunctionName(DefaultTexture); return;}\
        p->addText(#Text);\
        if (p->addImageButton("",FunctionName##ref , glm::vec2(128, 128), true)) {\
            if (openFileDialog(filters, filename))mpMaterial->set##FunctionName(createTextureFromFile(filename, false, true));\
        }
        /*if (p->addButton("White")) { mpMaterial->set##FunctionName(WhiteTexture);return;}\
        if (p->addButton("Black",true)) { mpMaterial->set##FunctionName(BlackTexture); return;}\
        if (p->addButton("Red",true)) { mpMaterial->set##FunctionName(RedTexture); return;}\
        if (p->addButton("Green",true)) { mpMaterial->set##FunctionName(GreenTexture); return;}\
        if (p->addButton("Blue",true)) { mpMaterial->set##FunctionName(BlueTexture);  return;}*/
        
        ADD_MATERIAL_TEXTURE(Albedo, BaseColorTexture, WhiteTexture);
        ADD_MATERIAL_TEXTURE(Specular, SpecularTexture, BlackTexture);
        ADD_MATERIAL_TEXTURE(NormalMap, NormalMap, WhiteTexture);
        ADD_MATERIAL_TEXTURE(HeightMap, HeightMap, BlackTexture);
        ADD_MATERIAL_TEXTURE(OcclusionMap, OcclusionMap, BlackTexture);
        //ADD_MATERIAL_TEXTURE(LightMap, LightMap);
    }
}

void MaterialInstance::onRender(RenderContext* pRenderContext, GraphicsVars* vars)
{
    auto& graphicsState = pRenderContext->getGraphicsState();
    if (mpProgramVars == nullptr) {
        mpProgramVars = pRenderContext->getGraphicsVars();
    }
    for (auto&v : param_texture2D) {
        mpProgramVars->setTexture(v.first, v.second);
    }
    auto& cb = get_constantbuffer(ECBType::PerFrame);
#define SET_CONSTANT_BUFFER(type)\
    for (auto&v : param_##type) {\
        cb[v.first] = v.second;\
    }

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

    if (mpProgram) {
        graphicsState->setProgram(mpProgram);
        pRenderContext->setGraphicsVars(mpProgramVars);
        vars = mpProgramVars.get();
    }

}

void MaterialInstance::set_program(const GraphicsProgram::SharedPtr & prog, bool use_default_material)
{
    if (!use_default_material) { bUseMaterial = false; }
    clear(); mpProgram = prog;
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
    if (use_default_material && mpMaterial) {
        bUseMaterial = use_default_material;
    }
}
