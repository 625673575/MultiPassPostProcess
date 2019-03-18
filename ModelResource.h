//单个FBX加载多个Mesh，并且每个Mesh有独立的Material
#pragma once

#include "Falcor.h"
#include "MaterialInstance.h"
using namespace Falcor;
class ModelResource {
public:
    ModelResource();
    ~ModelResource() = default;
    explicit ModelResource(const Model::SharedPtr& pModel);
public:
    std::string mModelString;
    uint64_t mResId;
    Model::SharedPtr mpModel = nullptr;
    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    bool mAnimate = false;
    bool mUseTriLinearFiltering = false;
    std::map<std::string, MaterialInstance::SharedPtr> sharedMaterials;
    
    std::string getModelDesc(bool isAfterCull, float loadTime);
    void setProgram(const std::string& materialName, const std::string& programName);
    uint32_t getProgramIndex(const GraphicsProgram::SharedPtr&);
    std::vector<std::string> getMaterialsName();
    static const std::map<std::string, std::function<void(MaterialInstance::SharedPtr&)>>& getProgramMapFunc();
    size_t getMaterialCount();
    MaterialInstance::SharedPtr& getMaterialInstance(uint32_t meshID);
    void resetMaterialGui();
    static bool hasInitGui;
    void init(const std::string& default_shader="");
    void onGui(Gui* p);
    void setTRS(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
    std::string getModelResName();
private:
    void initMaterials(const std::string& default_shader = "");
    std::map<std::string,uint32_t> programDropDownIndex;
    static Gui::DropdownList programDropDownList;
    static uint64_t count;
public:
    static uint64_t getModelCount() { return count; };
};
