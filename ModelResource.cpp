#include "ModelResource.h"
#include "ModelViewer.h"

Gui::DropdownList ModelResource::programDropDownList = {};
bool ModelResource::hasInitGui = false;
ModelResource::ModelResource() :Translation(0), Rotation(0), Scale(1)
{
}

ModelResource::ModelResource(const Model::SharedPtr& pModel) : Translation(0), Rotation(0), Scale(1)
{
    mpModel = pModel;
}

std::string ModelResource::getModelDesc(bool isAfterCull, float loadTime)
{
    std::string mModelString = isAfterCull ? "Mesh culling" : "Loading";
    mModelString += " took " + std::to_string(loadTime) + " seconds.\n";
    mModelString += "Model has " + std::to_string(mpModel->getVertexCount()) + " vertices, ";
    mModelString += std::to_string(mpModel->getIndexCount()) + " indices, ";
    mModelString += std::to_string(mpModel->getPrimitiveCount()) + " primitives, ";
    mModelString += std::to_string(mpModel->getMeshCount()) + " meshes, ";
    mModelString += std::to_string(mpModel->getInstanceCount()) + " mesh instances, ";
    mModelString += std::to_string(mpModel->getMaterialCount()) + " materials, ";
    mModelString += std::to_string(mpModel->getTextureCount()) + " textures, ";
    mModelString += std::to_string(mpModel->getBufferCount()) + " buffers.\n";
    return mModelString;
}

void ModelResource::setProgram(const std::string & materialName, const std::string & programName)
{
    auto& material = sharedMaterials[materialName];
    ModelViewer::getMaterialFuncMap().at(programName)(material);
    resetMaterialGui();
}

uint32_t ModelResource::getProgramIndex(const GraphicsProgram::SharedPtr & pProgram)
{
    uint32_t i = 0;
    for (auto & p : ModelViewer::getProgramMap()) {
        if (p.second == pProgram) {
            return i;
        }
        i++;
    }
    return 0;
}

std::vector<std::string> ModelResource::getMaterialsName()
{
    std::vector<std::string> r;
    for (auto&v : sharedMaterials)
        r.emplace_back(v.first);
    return r;
}

const std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>& ModelResource::getProgramMapFunc()
{
    return  ModelViewer::getMaterialFuncMap();
}

size_t ModelResource::getMaterialCount()
{
    return sharedMaterials.size();
}

void ModelResource::resetMaterialGui()
{
    for (auto&inst : sharedMaterials) {
        auto initProgramIndex = getProgramIndex(inst.second->get_program());
        programDropDownIndex.emplace(inst.first, initProgramIndex);
    }
}

void ModelResource::init(const std::string& default_shader)
{
    if (!hasInitGui) {
        uint32_t i = 0;
        auto& materialFuncMap = ModelViewer::getMaterialFuncMap();
        for (auto&v : materialFuncMap) {
            programDropDownList.push_back({ i++, v.first });
        }
        hasInitGui = true;
    }
    initMaterials(default_shader);
}

void ModelResource::setBuffers(RenderContext* pRenderContext, GraphicsVars* vars, uint32_t meshID)
{
    auto& pMat = mpModel->getMesh(meshID)->getMaterial();
    auto& MatInstance = sharedMaterials[pMat->getName()];
    MatInstance->onRender(pRenderContext, vars);
}

void ModelResource::onGui(Gui* p)
{
    p->addSeparator();
    auto modelName = mpModel->getName();
    if (p->beginGroup(modelName, true)) {
        p->addText("Transform ");
        p->addFloat3Var((modelName+"-Translation").c_str(), Translation);
        p->addFloat3Var((modelName + "-Rotation").c_str(), Rotation);
        p->addFloat3Var((modelName + "-Scale").c_str(), Scale);
        for (auto&v : sharedMaterials) {
            if (p->beginGroup(v.first, true)) {
                auto dropDownName = modelName + v.first + "-shader";
                if (p->addDropdown(dropDownName.c_str(), programDropDownList, programDropDownIndex[v.first])) {
                    auto i = 0;
                    for (auto & func : getProgramMapFunc()) {
                        if (i++ == programDropDownIndex[v.first]) {
                            func.second(v.second);
                        }
                    }
                }
                v.second->onMaterialGui(p);
                p->endGroup();
            }
        }
        p->endGroup();
    }
}

void ModelResource::setTRS(const glm::vec3 & translation, const glm::vec3 & rotation, const glm::vec3 & scale)
{
    Translation = translation;
    Rotation = rotation;
    Scale = scale;
}

void ModelResource::initMaterials(const std::string& default_shader)
{
    for (uint32_t i = 0; i < mpModel->getMeshCount(); i++) {
        auto& mat = mpModel->getMesh(i)->getMaterial();
        const auto& matName = mat->getName();
        if (sharedMaterials.find(matName) != sharedMaterials.end())
            continue;
        auto inst = std::make_shared<MaterialInstance>(matName, mat);
        sharedMaterials.emplace(matName, inst);
        if (default_shader.size() > 0) {
            ModelViewer::getMaterialFuncMap().at(default_shader)(inst);
            resetMaterialGui();
        }
    }
}
