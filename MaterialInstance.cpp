#include "MaterialInstance.h"
const std::string MaterialInstance::vsEntry = "vert";
const std::string MaterialInstance::psEntry = "frag";
Texture::SharedPtr MaterialInstance::pBlankTexture = nullptr;
MaterialInstance::SharedPtr MaterialInstance::create(const std::string & shader, const Program::DefineList & programDefines)
{
    return std::make_shared<MaterialInstance>(shader, programDefines);
}

MaterialInstance::MaterialInstance(const std::string & shader, const Program::DefineList& programDefines)
{
    mpProgram = GraphicsProgram::createFromFile(shader, vsEntry, psEntry, programDefines);
    mpProgramVars = GraphicsVars::create(mpProgram->getReflector());
}
MaterialInstance::MaterialInstance(const Material::SharedPtr& pMat)
{
    mpSharedMaterial = pMat;
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
    if (pBlankTexture == nullptr) {
        const uint8_t blankData[4] = { 255,255,255,255 };
        pBlankTexture = Texture::create2D(1, 1, ResourceFormat::RGBA8UnormSrgb, 1, 1, blankData);
    }
#define ADD_GUI_VEC(vec,var_type)\
    for (auto &v : param_##vec) {\
        std::string a=v.first+std::string("(")+#vec+std::string(")");\
        p->add##var_type##Var(a.c_str(), v.second);\
    }
#define ADD_GUI_MAT(mat)\
    for (auto &v : param_##mat) {\
        std::string a=v.first+std::string("(")+#mat+std::string(")");\
        p->addMatrixVar<mat>(a.c_str(), v.second);\
    }
   
    ADD_GUI_VEC(float, Float);
    ADD_GUI_VEC(vec2, Float2);
    ADD_GUI_VEC(vec3, Float3);
    ADD_GUI_VEC(vec4, Float4);
    ADD_GUI_VEC(int, Int);
    ADD_GUI_VEC(ivec2, Int2);
    ADD_GUI_VEC(ivec3, Int3);
    ADD_GUI_VEC(ivec4, Int4);
    ADD_GUI_MAT(mat2);
    ADD_GUI_MAT(mat3);
    ADD_GUI_MAT(mat4);

    for (auto &v : param_bool) {
        p->addCheckBox(v.first.c_str(), v.second);
    }

    for (auto&v : param_texture2D) {
        if (p->addImageButton(v.first.c_str(), v.second == nullptr ? pBlankTexture : v.second, glm::vec2(128, 128),true,true)) {
            std::string filename;
            FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"},{"gif"} };
            if (openFileDialog(filters, filename))
            {
                v.second = createTextureFromFile(filename, false, true);
            }
        }
        p->addText(v.first.c_str());
    }
}

void MaterialInstance::onRender(RenderContext* pRenderContext)
{
    static auto& graphicsState = pRenderContext->getGraphicsState();
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

    pRenderContext->setGraphicsVars(mpProgramVars);
    graphicsState->setProgram(mpProgram);

}
