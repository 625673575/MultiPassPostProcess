#include "SceneExtend.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "Framework.h"
#include <fstream>
#include "Utils/Platform/OS.h"
#include "Graphics/Scene/Editor/SceneEditor.h"

#define SCENE_EXPORTER
#include "Graphics/Scene/SceneExportImportCommon.h"
SceneExtend::SceneExtend(const std::string& filename) :Scene(filename)
{
}
template<typename T>
void addLiteral(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
{
    rapidjson::Value jkey;
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
    jval.AddMember(jkey, value, jallocator);
}

void addJsonValue(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, rapidjson::Value& value)
{
    rapidjson::Value jkey;
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
    jval.AddMember(jkey, value, jallocator);
}

void addString(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const std::string& value)
{
    rapidjson::Value jstring, jkey;
    jstring.SetString(value.c_str(), (uint32_t)value.size(), jallocator);
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

    jval.AddMember(jkey, jstring, jallocator);
}

void addBool(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, bool isValue)
{
    rapidjson::Value jbool, jkey;
    jbool.SetBool(isValue);
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

    jval.AddMember(jkey, jbool, jallocator);
}

void addInt(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, INT64 intValue)
{
    rapidjson::Value jint64, jkey;
    jint64.SetInt64(intValue);
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

    jval.AddMember(jkey, jint64, jallocator);
}
void addFloat(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, float fValue)
{
    rapidjson::Value jfloat, jkey;
    jfloat.SetFloat(fValue);
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

    jval.AddMember(jkey, jfloat, jallocator);
}
template<typename T>
void addVector(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
{
    rapidjson::Value jkey;
    jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
    rapidjson::Value jvec(rapidjson::kArrayType);
    for (int32_t i = 0; i < value.length(); i++)
    {
        jvec.PushBack(value[i], jallocator);
    }

    jval.AddMember(jkey, jvec, jallocator);
}
void SceneExtend::addModelResource(const ModelResource& pModel, const std::string& instanceName, const glm::vec3& translation, const glm::vec3& yawPitchRoll, const glm::vec3& scaling)
{
    std::string name = instanceName;
    if (instanceName.empty()) {
        name = pModel.mpModel->getName();
    }
    addModelInstance(pModel.mpModel, name, translation, yawPitchRoll, scaling);
    modelsRes.emplace_back(pModel);
}

bool SceneExtend::save(const std::string& filename)
{
    //mExportOptions = exportOptions;
    mFilename = filename;
    // create the file
    mJDoc.SetObject();

    // Write the version
    rapidjson::Value& JVal = mJDoc;
    auto& allocator = mJDoc.GetAllocator();
    //addLiteral(JVal, allocator, SceneKeys::kVersion, kVersion);

    //// Write everything else
    //bool exportPaths = (exportOptions & ExportPaths) != 0;
    //if (exportOptions & ExportGlobalSettings)    writeGlobalSettings(exportPaths);
    //if (exportOptions & ExportModels)            writeModels();
    //if (exportOptions & ExportLights)            writeLights();
    //if (exportOptions & ExportCameras)           writeCameras();
    //if (exportOptions & ExportUserDefined)       writeUserDefinedSection();
    //if (exportOptions & ExportPaths)             writePaths();
    writeModels();
    // Get the output string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    writer.SetIndent(' ', 4);
    mJDoc.Accept(writer);
    std::string str(buffer.GetString(), buffer.GetSize());

    // Output the file
    std::ofstream outputStream(mFilename.c_str());
    if (outputStream.fail())
    {
        logError("Can't open output scene file " + mFilename + ".\nExporting failed.");
        return false;
    }
    outputStream << str;
    outputStream.close();

    return true;
}

bool SceneExtend::load(const std::string & filename)
{
    return false;
}

void SceneExtend::createModelValue(uint32_t modelID, rapidjson::Document::AllocatorType & allocator, rapidjson::Value & jmodel)
{
    jmodel.SetObject();

    auto& modelRes = modelsRes.at(modelID);
    const Model* pModel = modelRes.mpModel.get();

    // Export model properties
    addString(jmodel, allocator, SceneKeys::kFilename, stripDataDirectories(pModel->getFilename()));
    addString(jmodel, allocator, SceneKeys::kName, pModel->getName());

    if (getModel(modelID)->hasAnimations())
    {
        addLiteral(jmodel, allocator, SceneKeys::kActiveAnimation, pModel->getActiveAnimation());
    }

    // Export model material properties
    rapidjson::Value materialValue;
    materialValue.SetObject();
    switch (pModel->getMesh(0)->getMaterial()->getShadingModel())
    {
    case ShadingModelMetalRough:
        addString(materialValue, allocator, SceneKeys::kShadingModel, SceneKeys::kShadingMetalRough);
        break;
    case ShadingModelSpecGloss:
        addString(materialValue, allocator, SceneKeys::kShadingModel, SceneKeys::kShadingSpecGloss);
        break;
    default:
        logWarning("SceneExporter: Unknown shading model found on model " + pModel->getName() + ", ignoring value");
    }
    addJsonValue(jmodel, allocator, SceneKeys::kMaterial, materialValue);

    // Export model instances
    rapidjson::Value jsonInstanceArray;
    jsonInstanceArray.SetArray();
    for (uint32_t i = 0; i < getModelInstanceCount(modelID); i++)
    {
        rapidjson::Value jsonInstance;
        jsonInstance.SetObject();
        auto& pInstance = getModelInstance(modelID, i);

        addString(jsonInstance, allocator, SceneKeys::kName, pInstance->getName());
        addVector(jsonInstance, allocator, SceneKeys::kTranslationVec, pInstance->getTranslation());
        addVector(jsonInstance, allocator, SceneKeys::kScalingVec, pInstance->getScaling());

        // Translate rotation to degrees
        glm::vec3 rotation = glm::degrees(pInstance->getRotation());
        addVector(jsonInstance, allocator, SceneKeys::kRotationVec, rotation);

        rapidjson::Value jsonMaterialArray;
        jsonMaterialArray.SetArray();
        for (uint32_t meshID = 0; meshID < pInstance->getObject()->getMeshCount(); meshID++)
        {
            rapidjson::Value jsonMaterial;
            jsonMaterial.SetObject();
            auto& mat = modelRes.getMaterialInstance(meshID);
            addString(jsonMaterial, allocator, SceneExtendKeys::kMaterialName, mat->mName);
            addString(jsonMaterial, allocator, SceneExtendKeys::kShader, mat->mShaderName);
            addInt(jsonMaterial, allocator, SceneExtendKeys::kRenderQueue, mat->renderQueue);
            addInt(jsonMaterial, allocator, SceneExtendKeys::kBlendMode, UINT64(mat->blendMode));
            addInt(jsonMaterial, allocator, SceneExtendKeys::kRasterizeMode, UINT64(mat->rasterizeMode));
            addBool(jsonMaterial, allocator, SceneExtendKeys::kDepthTest, mat->depthStencilBundle.bDepthTest);
            addBool(jsonMaterial, allocator, SceneExtendKeys::kWriteDepth, mat->depthStencilBundle.bWriteDepth);
            addInt(jsonMaterial, allocator, SceneExtendKeys::kDepthTestFunc, UINT64(mat->depthStencilBundle.eDepthTestFunc));

            for (auto& v : mat->param_bool) {
                addFloat(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_float) {
                addFloat(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_vec2) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_vec3) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_vec4) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_int) {
                addInt(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_ivec2) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_ivec3) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }
            for (auto& v : mat->param_ivec4) {
                addVector(jsonMaterial, allocator, v.first, v.second);
            }

            for (auto& v : mat->param_texture2D) {
                addString(jsonMaterial, allocator, v.first, v.second->getSourceFilename());
            }
            //todo : mat 写入
            jsonMaterialArray.PushBack(jsonMaterial, allocator);
            
            //const auto& shaderName = ;
        }

        jsonInstanceArray.PushBack(jsonInstance, allocator);
        jsonInstanceArray.PushBack(jsonMaterialArray, allocator);

    }

    addJsonValue(jmodel, allocator, SceneKeys::kModelInstances, jsonInstanceArray);
}

void SceneExtend::writeModels()
{
    if (getModelCount() == 0)
    {
        return;
    }

    rapidjson::Value jsonModelArray;
    jsonModelArray.SetArray();

    for (uint32_t i = 0; i < getModelCount(); i++)
    {
        rapidjson::Value jsonModel;
        createModelValue(i, mJDoc.GetAllocator(), jsonModel);
        jsonModelArray.PushBack(jsonModel, mJDoc.GetAllocator());
    }
    addJsonValue(mJDoc, mJDoc.GetAllocator(), SceneKeys::kModels, jsonModelArray);
}

SceneExtend::~SceneExtend()
{
}
