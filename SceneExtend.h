#pragma once
#include "Falcor.h"
#include "ModelResource.h"
#include "rapidjson/document.h"
#include <sstream>
using namespace Falcor;
namespace SceneExtendKeys {
    static const char* kMaterialName = "MaterialName";
    static const char* kMaterials = "Materials";
    static const char* kShader = "Shader";
    static const char* kDepthTest = "DepthTest";
    static const char* kWriteDepth = "WriteDepth";
    static const char* kDepthTestFunc = "DepthTestFunc";
    static const char* kBlendMode = "BlendMode";
    static const char* kRasterizeMode = "RasterizeMode";
    static const char* kRenderQueue = "RenderQueue";
    static const char* kParameter = "Parameter";
}

namespace ParameterPrefix {
    static const char* kBool = "(b1)";
    static const char* kFloat = "(f1)";
    static const char* kFloat2 = "(f2)";
    static const char* kFloat3 = "(f3)";
    static const char* kFloat4 = "(f4)";
    static const char* kInt = "(i1)";
    static const char* kInt2 = "(i2)";
    static const char* kInt3 = "(i3)";
    static const char* kInt4 = "(i4)";
    static const char* kMat2 = "(m2)";
    static const char* kMat3 = "(m3)";
    static const char* kMat4 = "(m4)";
    static const char* kTexture2D = "(t2)";
}
namespace ParameterDefault {
    static const char* kBaseColor = "Albedo";
    static const char* kSpecular = "Specular";
    static const char* kNormalMap = "NormalMap";
    static const char* kEmissive = "Emissive";
    static const char* kOcclusionMap = "OcclusionMap";
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
            addModelInstance(v.mpModel, v.mpModel->getName());
        }
    }
    void clearModels() { mModelRes.clear(); deleteAllModels(); }
    bool empty() { return mModelRes.empty(); }
    std::vector<ModelResource>& getModels() { return mModelRes; }
private:
    void createModelValue(uint32_t modelID, rapidjson::Document::AllocatorType& allocator, rapidjson::Value& jmodel);
    void writeModelsJson();
    bool topLevelLoop();
    bool validateSceneFile();

    bool parseModels(const rapidjson::Value& jsonVal);

    using ObjectMap = std::map<std::string, IMovableObject::SharedPtr>;
    bool isNameDuplicate(const std::string& name, const ObjectMap& objectMap, const std::string& objectType) const;
    bool createModel(const rapidjson::Value& jsonModel);
    bool createModelInstances(const rapidjson::Value& jsonVal, const Model::SharedPtr& pModel);

    ObjectMap mInstanceMap;
    ObjectMap mCameraMap;
    ObjectMap mLightMap;

    struct FuncValue
    {
        const std::string token;
        decltype(&SceneExtend::parseModels) func;
    };
    static const FuncValue kFunctionTable[];

    bool error(const std::string& msg);
    template<uint32_t VecSize>
    bool getFloatVec(const rapidjson::Value& jsonVal, const std::string& desc, float vec[VecSize]);
    template<uint32_t VecSize>
    bool getIntVec(const rapidjson::Value& jsonVal, const std::string& desc, int vec[VecSize]);
    bool getFloatVecAnySize(const rapidjson::Value& jsonVal, const std::string& desc, std::vector<float>& vec);
    std::vector<ModelResource> mModelRes;
    rapidjson::Document mJDoc;
    std::string mFilename;
    std::string mDirectory;
    uint32_t mExportOptions = 0;
    Model::LoadFlags mModelLoadFlags = Model::LoadFlags::None;
};

