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
    ModelResource& getModelResource(uint32_t id) { return mModelRes[id]; }
    bool save(const std::string& filename);
    bool load(const std::string& filename);
    //reset after load from file or delete a model
    void reset() {
        deleteAllModels();
        for (auto& v : mModelRes) {
            addModelInstance(v.mpModel,v.mpModel->getName());
        }
    }
    void clearModels() { mModelRes.clear(); deleteAllModels(); }
    bool empty() { return mModelRes.empty(); }
    std::vector<ModelResource>& getModels() { return mModelRes; }
private:
    void createModelValue(uint32_t modelID, rapidjson::Document::AllocatorType & allocator, rapidjson::Value & jmodel);
    void writeModelsJson();
    bool topLevelLoop();
    bool validateSceneFile();

    bool parseModels(const rapidjson::Value & jsonVal);

    using ObjectMap = std::map<std::string, IMovableObject::SharedPtr>;
    bool isNameDuplicate(const std::string & name, const ObjectMap & objectMap, const std::string & objectType) const;
    bool createModel(const rapidjson::Value & jsonModel);
    bool createModelInstances(const rapidjson::Value & jsonVal, const Model::SharedPtr & pModel);

    ObjectMap mInstanceMap;
    ObjectMap mCameraMap;
    ObjectMap mLightMap;

    struct FuncValue
    {
        const std::string token;
        decltype(&SceneExtend::parseModels) func;
    };
    static const FuncValue kFunctionTable[];

    bool error(const std::string & msg);
    template<uint32_t VecSize>
    bool getFloatVec(const rapidjson::Value & jsonVal, const std::string & desc, float vec[VecSize]);
    bool getFloatVecAnySize(const rapidjson::Value & jsonVal, const std::string & desc, std::vector<float> & vec);
    std::vector<ModelResource> mModelRes;
    rapidjson::Document mJDoc;
    std::string mFilename;
    std::string mDirectory;
    uint32_t mExportOptions = 0;
    Model::LoadFlags mModelLoadFlags;
};

