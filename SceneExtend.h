#pragma once
#include "Falcor.h"
#include "ModelResource.h"
#include "rapidjson/document.h"
#include <sstream>
using namespace Falcor;
namespace SceneExtendKeys {
    static const char* kMaterialName = "MaterialName";
    static const char* kShader = "Shader";
    static const char* kDepthTest = "DepthTest";
    static const char* kWriteDepth = "WriteDepth";
    static const char* kDepthTestFunc = "DepthTestFunc";
    static const char* kBlendMode = "BlendMode";
    static const char* kRasterizeMode = "RasterizeMode";
    static const char* kRenderQueue = "RenderQueue";
}
class SceneExtend :
    public Scene
{
public:
    SceneExtend(const std::string& filename = "");
    ~SceneExtend();
    void addModelResource(const ModelResource& pModel, const std::string& instanceName = "", const glm::vec3& translation = glm::vec3(), const glm::vec3& yawPitchRoll = glm::vec3(), const glm::vec3& scaling = glm::vec3(1));
    ModelResource& getModelResource(uint32_t id) { return modelsRes[id]; }
    bool save(const std::string& filename);
    bool load(const std::string& filename);
    void reset() {
        this->deleteAllModels();
        for (auto& v : modelsRes) {
            this->addModelResource(v);
    }
    };
    bool empty() { return modelsRes.empty(); }
    std::vector<ModelResource>& getModels() { return modelsRes; }
private:
    void createModelValue(uint32_t modelID, rapidjson::Document::AllocatorType& allocator, rapidjson::Value& jmodel);
    void writeModels();
    std::vector<ModelResource> modelsRes;
    rapidjson::Document mJDoc;
    std::string mFilename;
    uint32_t mExportOptions = 0;
};

