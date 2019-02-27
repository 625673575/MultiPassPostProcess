//单个FBX加载多个Mesh，并且每个Mesh有独立的Material
#pragma once

#include "Falcor.h"
#include "MaterialInstance.h"
using namespace Falcor;
class ModelResource {
public:
    ModelResource();
    ~ModelResource() = default;
    ModelResource(const Model::SharedPtr& pModel);
public:
    std::string mModelString;
    Model::SharedPtr mpModel = nullptr;
    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    bool mAnimate = false;
    bool mUseTriLinearFiltering = false;
    std::string getModelDesc(bool isAfterCull, float loadTime);
    std::map<std::string, MaterialInstance::SharedPtr> sharedMaterials;
    void setProgram(const std::string& materialName, const std::string& programName);
    uint32_t getProgramIndex(const GraphicsProgram::SharedPtr&);
    std::vector<std::string> getMaterialsName();
    static const std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>& getProgramMapFunc();
    size_t getMaterialCount();
    void resetMaterialGui();
    static bool hasInitGui;
    void init();
    void setBuffers(RenderContext* pRenderContext, GraphicsVars* vars, uint32_t meshID);
    void onGui(Gui* p);
    void setTRS(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
private:
    void initMaterials();
    std::map<std::string,uint32_t> programDropDownIndex;
    static Gui::DropdownList programDropDownList;

};
