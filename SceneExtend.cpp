#include "SceneExtend.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "Framework.h"
#include <fstream>
#include "Utils/Platform/OS.h"
#include "Graphics/Scene/Editor/SceneEditor.h"
#include "rapidjson/error/en.h"
#include "TextureHelperExtend.h"
#define SCENE_EXPORTER
#define SCENE_IMPORTER
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

template<uint32_t VecSize>
bool SceneExtend::getFloatVec(const rapidjson::Value& jsonVal, const std::string& desc, float vec[VecSize])
{
    if (jsonVal.IsArray() == false)
    {
        error("Trying to load a vector for " + desc + ", but JValue is not an array");
        return false;
    }

    if (jsonVal.Size() != VecSize)
    {
        return error("Trying to load a vector for " + desc + ", but vector size mismatches. Required size is " + std::to_string(VecSize) + ", array size is " + std::to_string(jsonVal.Size()));
    }

    for (uint32_t i = 0; i < jsonVal.Size(); i++)
    {
        if (jsonVal[i].IsNumber() == false)
        {
            return error("Trying to load a vector for " + desc + ", but one the elements is not a number.");
        }

        vec[i] = (float)(jsonVal[i].GetDouble());
    }
    return true;
}

template<uint32_t VecSize>
bool SceneExtend::getIntVec(const rapidjson::Value & jsonVal, const std::string & desc, int vec[VecSize])
{
    if (jsonVal.IsArray() == false)
    {
        error("Trying to load a vector for " + desc + ", but JValue is not an array");
        return false;
    }

    if (jsonVal.Size() != VecSize)
    {
        return error("Trying to load a vector for " + desc + ", but vector size mismatches. Required size is " + std::to_string(VecSize) + ", array size is " + std::to_string(jsonVal.Size()));
    }

    for (uint32_t i = 0; i < jsonVal.Size(); i++)
    {
        if (jsonVal[i].IsNumber() == false)
        {
            return error("Trying to load a vector for " + desc + ", but one the elements is not a number.");
        }

        vec[i] = (jsonVal[i].GetInt());
    }
    return true;
}

bool SceneExtend::getFloatVecAnySize(const rapidjson::Value & jsonVal, const std::string & desc, std::vector<float> & vec)
{
    if (jsonVal.IsArray() == false)
    {
        return error("Trying to load a vector for " + desc + ", but JValue is not an array");
    }

    vec.resize(jsonVal.Size());
    for (uint32_t i = 0; i < jsonVal.Size(); i++)
    {
        if (jsonVal[i].IsNumber() == false)
        {
            return error("Trying to load a vector for " + desc + ", but one the elements is not a number.");
        }

        vec[i] = (float)(jsonVal[i].GetDouble());
    }
    return true;
}

void SceneExtend::addModelResource(const ModelResource & pModel, const std::string & instanceName, const glm::vec3 & translation, const glm::vec3 & yawPitchRoll, const glm::vec3 & scaling)
{
    std::string name = instanceName;
    if (instanceName.empty()) {
        name = pModel.mpModel->getName();
    }
    mModelRes.emplace_back(pModel);
    addModelInstance(pModel.mpModel, name, translation, yawPitchRoll, scaling);
}

bool SceneExtend::save(const std::string & filename)
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
    writeModelsJson();
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
    std::string fullpath;
    mFilename = filename;

    if (findFileInDataDirectories(filename, fullpath))
    {
        // Load the file
        std::string jsonData = readFile(fullpath);
        rapidjson::StringStream JStream(jsonData.c_str());

        // Get the file directory
        auto last = fullpath.find_last_of("/\\");
        mDirectory = fullpath.substr(0, last);

        // create the DOM
        mJDoc.ParseStream(JStream);

        if (mJDoc.HasParseError())
        {
            size_t line;
            line = std::count(jsonData.begin(), jsonData.begin() + mJDoc.GetErrorOffset(), '\n');
            return error(std::string("JSON Parse error in line ") + std::to_string(line) + ". " + rapidjson::GetParseError_En(mJDoc.GetParseError()));
        }

        clearModels();
        if (topLevelLoop() == false)
        {
            return false;
        }
        return true;
    }
    else
    {
        return error("File not found.");
    }
    return false;
}
const SceneExtend::FuncValue SceneExtend::kFunctionTable[] =
{
    //// The order matters here.
    //{SceneKeys::kVersion, &SceneExtend::parseVersion},
    //{SceneKeys::kSceneUnit, &SceneExtend::parseSceneUnit},
    //{SceneKeys::kEnvMap, &SceneExtend::parseEnvMap},
    //{SceneKeys::kAmbientIntensity, &SceneExtend::parseAmbientIntensity},
    //{SceneKeys::kLightingScale, &SceneExtend::parseLightingScale},
    //{SceneKeys::kCameraSpeed, &SceneExtend::parseCameraSpeed},

    {SceneKeys::kModels, &SceneExtend::parseModels},
    //{SceneKeys::kLights, &SceneExtend::parseLights},
    //{SceneKeys::kLightProbes, &SceneExtend::parseLightProbes},
    //{SceneKeys::kCameras, &SceneExtend::parseCameras},
    //{SceneKeys::kActiveCamera, &SceneExtend::parseActiveCamera},  // Should come after ParseCameras
    //{SceneKeys::kUserDefined, &SceneExtend::parseUserDefinedSection},

    //{SceneKeys::kPaths, &SceneExtend::parsePaths},
    //{SceneKeys::kActivePath, &SceneExtend::parseActivePath}, // Should come after ParsePaths
    //{SceneKeys::kInclude, &SceneExtend::parseIncludes}
};
bool SceneExtend::validateSceneFile()
{
    // Make sure the top-level is valid
    for (auto it = mJDoc.MemberBegin(); it != mJDoc.MemberEnd(); it++)
    {
        bool found = false;
        const std::string name(it->name.GetString());

        for (uint32_t i = 0; i < arraysize(kFunctionTable); i++)
        {
            // Check that we support this value
            if (kFunctionTable[i].token == name)
            {
                found = true;
                break;
            }
        }

        if (found == false)
        {
            return error("Invalid key found in top-level object. Key == " + std::string(it->name.GetString()) + ".");
        }
    }
    return true;
}
bool SceneExtend::parseModels(const rapidjson::Value & jsonVal)
{
    if (jsonVal.IsArray() == false)
    {
        return error("models section should be an array of objects.");
    }

    // Loop over the array
    for (uint32_t i = 0; i < jsonVal.Size(); i++)
    {
        if (createModel(jsonVal[i]) == false)
        {
            return false;
        }
    }
    return true;
}
bool SceneExtend::createModel(const rapidjson::Value & jsonModel)
{
    // Model must have at least a filename
    if (jsonModel.HasMember(SceneKeys::kFilename) == false)
    {
        return error("Model must have a filename");
    }

    // Get Model name
    const auto& modelFile = jsonModel[SceneKeys::kFilename];
    if (modelFile.IsString() == false)
    {
        return error("Model filename must be a string");
    }

    std::string file = mDirectory + '/' + modelFile.GetString();
    if (doesFileExist(file) == false)
    {
        file = modelFile.GetString();
    }

    // Parse additional properties that affect loading
    Model::LoadFlags modelFlags = mModelLoadFlags;
    if (jsonModel.HasMember(SceneKeys::kMaterial))
    {
        const auto& materialSettings = jsonModel[SceneKeys::kMaterial];
        if (materialSettings.IsObject() == false)
        {
            return error("Material properties for \"" + file + "\" must be a JSON object");
        }

        for (auto m = materialSettings.MemberBegin(); m != materialSettings.MemberEnd(); m++)
        {
            if (m->name == SceneKeys::kShadingModel)
            {
                if (m->value == SceneKeys::kShadingSpecGloss)
                {
                    modelFlags |= Model::LoadFlags::UseSpecGlossMaterials;
                }
                else
                {
                    modelFlags |= Model::LoadFlags::UseMetalRoughMaterials;
                }
            }
        }
    }
    else {
        modelFlags |= Model::LoadFlags::UseSpecGlossMaterials;
    }

    // Load the model
    auto pModel = Model::createFromFile(file.c_str(), modelFlags);
    if (pModel == nullptr)
    {
        return error("Could not load model: " + file);
    }

    bool instanceAdded = false;

    // Loop over the other members
    for (auto jval = jsonModel.MemberBegin(); jval != jsonModel.MemberEnd(); jval++)
    {
        std::string keyName(jval->name.GetString());
        if (keyName == SceneKeys::kFilename)
        {
            // Already handled
        }
        else if (keyName == SceneKeys::kName)
        {
            if (jval->value.IsString() == false)
            {
                return error("Model name should be a string value.");
            }
            pModel->setName(std::string(jval->value.GetString()));
        }
        else if (keyName == SceneKeys::kModelInstances)
        {
            if (createModelInstances(jval->value, pModel) == false)
            {
                return false;
            }

            instanceAdded = true;
        }
        else if (keyName == SceneKeys::kActiveAnimation)
        {
            if (jval->value.IsUint() == false)
            {
                return error("Model active animation should be an unsigned integer");
            }
            uint32_t activeAnimation = jval->value.GetUint();
            if (activeAnimation >= pModel->getAnimationsCount())
            {
                std::string msg = "Warning when parsing scene file \"" + mFilename + "\".\nModel " + pModel->getName() + " was specified with active animation " + std::to_string(activeAnimation);
                msg += ", but model only has " + std::to_string(pModel->getAnimationsCount()) + " animations. Ignoring field";
                logWarning(msg);
            }
            else
            {
                pModel->setActiveAnimation(activeAnimation);
            }
        }
        else if (keyName == SceneKeys::kMaterial)
        {
            // Existing parameters already handled
        }
        else
        {
            return error("Invalid key found in models array. Key == " + keyName + ".");
        }
    }

    // If no instances for the model were loaded from the scene file
    if (instanceAdded == false)
    {
        addModelInstance(pModel, "Instance 0");
    }

    return true;
}

bool SceneExtend::isNameDuplicate(const std::string & name, const ObjectMap & objectMap, const std::string & objectType) const
{
    if (objectMap.find(name) != objectMap.end())
    {
        const std::string msg = "Multiple " + objectType + " found the same name: " + name + ".\nObjects may not attach to paths correctly.\n\nContinue anyway?";

        // If user pressed ok, return false to ignore duplicate
        return msgBox(msg, MsgBoxType::OkCancel) == MsgBoxButton::Ok ? false : true;
    }

    return false;
}
bool SceneExtend::createModelInstances(const rapidjson::Value & jsonVal, const Model::SharedPtr & pModel)
{
    if (jsonVal.IsArray() == false)
    {
        return error("Model instances should be an array of objects");
    }
    ModelResource modelRes(pModel);
    modelRes.init("Diffuse");
    for (uint32_t i = 0; i < jsonVal.Size(); i++)
    {
        const auto& instance = jsonVal[i];
        glm::vec3 scaling(1, 1, 1);
        glm::vec3 translation(0, 0, 0);
        glm::vec3 rotation(0, 0, 0);
        std::string name = "Instance " + std::to_string(i);
        auto shadingModel = ShadingModelMetalRough;
        for (auto m = instance.MemberBegin(); m < instance.MemberEnd(); m++)
        {
            std::string key(m->name.GetString());
            if (key == SceneKeys::kName)
            {
                if (m->value.IsString() == false)
                {
                    return error("Model instance name should be a string value.");
                }
                name = std::string(m->value.GetString());
            }
            else if (key == SceneKeys::kTranslationVec)
            {
                if (getFloatVec<3>(m->value, "Model instance translation vector", &translation[0]) == false)
                {
                    return false;
                }
            }
            else if (key == SceneKeys::kScalingVec)
            {
                if (getFloatVec<3>(m->value, "Model instance scale vector", &scaling[0]) == false)
                {
                    return false;
                }
            }
            else if (key == SceneKeys::kRotationVec)
            {
                if (getFloatVec<3>(m->value, "Model instance rotation vector", &rotation[0]) == false)
                {
                    return false;
                }

                rotation = glm::radians(rotation);
            }
            else if (key == SceneKeys::kShadingModel)
            {
                std::string sVal = m->value.GetString();
                if (sVal == SceneKeys::kShadingSpecGloss)
                {
                    shadingModel = ShadingModelSpecGloss;
                }

                rotation = glm::radians(rotation);
            }
            else if (key == SceneExtendKeys::kMaterials) {
                auto& mats = m->value.GetArray();
                for (auto mm = mats.Begin(); mm < mats.End(); mm++)
                {
                    std::string MaterialName = (*mm)[SceneExtendKeys::kMaterialName].GetString();
                    DepthStencilStateBundle bundle;
                    for (auto mmx = mm->MemberBegin(); mmx < mm->MemberEnd(); mmx++) {
                        std::string mkey(mmx->name.GetString());
                        modelRes.sharedMaterials[MaterialName]->mpMaterial->setShadingModel(shadingModel);

                        if (mkey == SceneExtendKeys::kRenderQueue) {
                            uint32_t RenderQueue(mmx->value.GetInt());
                            modelRes.sharedMaterials[MaterialName]->setRenderQueue(RenderQueue);
                        }
                        if (mkey == SceneExtendKeys::kBlendMode) {
                            uint32_t BlendMode(mmx->value.GetInt());
                            modelRes.sharedMaterials[MaterialName]->set_blendMode(MaterialInstance::EBlendMode(BlendMode));
                        }
                        if (mkey == SceneExtendKeys::kRasterizeMode) {
                            uint32_t RasterizeMode(mmx->value.GetInt());
                            modelRes.sharedMaterials[MaterialName]->set_rasterizeMode(MaterialInstance::ERasterizeMode(RasterizeMode));
                        }
                        {
                            if (mkey == SceneExtendKeys::kDepthTest) {
                                bundle.bDepthTest = mmx->value.GetBool();
                            }
                            if (mkey == SceneExtendKeys::kWriteDepth) {
                                bundle.bWriteDepth = mmx->value.GetBool();
                            }
                            if (mkey == SceneExtendKeys::kDepthTestFunc) {
                                auto DepthTestFunc = mmx->value.GetInt();
                                bundle.eDepthTestFunc = Falcor::ComparisonFunc(DepthTestFunc);
                            }
                        }
                        if (mkey == SceneExtendKeys::kParameter) {
                            auto& jmp = mmx->value;
                            for (auto mmxp = jmp.MemberBegin(); mmxp < jmp.MemberEnd(); mmxp++) {
                                std::string pkey = mmxp->name.GetString();
                                std::string prefix(pkey.substr(0, 4));
                                std::string parameter(pkey.substr(4));
                                if (prefix == ParameterPrefix::kBool) {
                                    auto val = mmxp->value.GetBool();
                                    modelRes.sharedMaterials[MaterialName]->insert_bool(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kFloat) {
                                    auto val = mmxp->value.GetFloat();
                                    modelRes.sharedMaterials[MaterialName]->insert_float(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kFloat2) {
                                    glm::vec2 val;
                                    getFloatVec<2>(mmxp->value, ParameterPrefix::kFloat2, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_vec2(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kFloat3) {
                                    glm::vec3 val;
                                    getFloatVec<3>(mmxp->value, ParameterPrefix::kFloat3, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_vec3(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kFloat4) {
                                    glm::vec4 val;
                                    getFloatVec<4>(mmxp->value, ParameterPrefix::kFloat4, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_vec4(parameter, val);
                                }

                                if (prefix == ParameterPrefix::kInt) {
                                    auto val = mmxp->value.GetInt();
                                    modelRes.sharedMaterials[MaterialName]->insert_int(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kInt2) {
                                    glm::ivec2 val;
                                    getIntVec<2>(mmxp->value, ParameterPrefix::kInt2, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_ivec2(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kInt3) {
                                    glm::ivec3 val;
                                    getIntVec<3>(mmxp->value, ParameterPrefix::kInt3, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_ivec3(parameter, val);
                                }
                                if (prefix == ParameterPrefix::kInt4) {
                                    glm::ivec4 val;
                                    getIntVec<4>(mmxp->value, ParameterPrefix::kInt4, &val[0]);
                                    modelRes.sharedMaterials[MaterialName]->insert_ivec4(parameter, val);
                                }

                                if (prefix == ParameterPrefix::kTexture2D) {
                                    std::string texName = mmxp->value.GetString();
                                    if (!texName.empty()) {
                                        auto pTexture = createTextureFromFile(texName.c_str(), false, true);
                                        if (pTexture) {
                                            if (parameter == ParameterDefault::kBaseColor) {
                                                modelRes.sharedMaterials[MaterialName]->mpMaterial->setBaseColorTexture(pTexture);
                                                modelRes.sharedMaterials[MaterialName]->bUseMaterial = true;
                                            }
                                            else if (parameter == ParameterDefault::kSpecular) {
                                                modelRes.sharedMaterials[MaterialName]->mpMaterial->setSpecularTexture(pTexture);
                                                modelRes.sharedMaterials[MaterialName]->bUseMaterial = true;
                                            }
                                            else if (parameter == ParameterDefault::kNormalMap) {
                                                modelRes.sharedMaterials[MaterialName]->mpMaterial->setNormalMap(pTexture);
                                                modelRes.sharedMaterials[MaterialName]->bUseMaterial = true;
                                            }
                                            else if (parameter == ParameterDefault::kEmissive) {
                                                modelRes.sharedMaterials[MaterialName]->mpMaterial->setEmissiveTexture(pTexture);
                                                modelRes.sharedMaterials[MaterialName]->bUseMaterial = true;
                                            }
                                            else if (parameter == ParameterDefault::kOcclusionMap) {
                                                modelRes.sharedMaterials[MaterialName]->mpMaterial->setOcclusionMap(pTexture);
                                                modelRes.sharedMaterials[MaterialName]->bUseMaterial = true;
                                            }
                                            else {
                                                modelRes.sharedMaterials[MaterialName]->set_texture2D(parameter, pTexture);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    modelRes.sharedMaterials[MaterialName]->set_depthStencilTest(bundle);
                }
            }
            else
            {
                return error("Unknown key \"" + key + "\" when parsing model instance");
            }
            modelRes.setTRS(translation, rotation, scaling);
        }

        if (isNameDuplicate(name, mInstanceMap, "model instances"))
        {
            return false;
        }
        else
        {

            //auto pInstance = Scene::ModelInstance::create(pModel, translation, rotation, scaling, name);
            //mInstanceMap[pInstance->getName()] = pInstance;
            //addModelInstance(pInstance);
            addModelResource(modelRes, "", translation, rotation, scaling);
        }
    }

    return true;
}
bool SceneExtend::topLevelLoop()
{
    for (uint32_t i = 0; i < arraysize(kFunctionTable); i++)
    {
        const auto& jsonMember = mJDoc.FindMember(kFunctionTable[i].token.c_str());
        if (jsonMember != mJDoc.MemberEnd())
        {
            auto a = kFunctionTable[i].func;
            if ((this->*a)(jsonMember->value) == false)
            {
                return false;
            }
        }
    }

    return true;
}
bool SceneExtend::error(const std::string & msg)
{
    std::string err = "Error when parsing scene file \"" + mFilename + "\".\n" + msg;
#if _LOG_ENABLED
    logError(err);
#else
    msgBox(err);
#endif
    return false;
}

void SceneExtend::createModelValue(uint32_t modelID, rapidjson::Document::AllocatorType & allocator, rapidjson::Value & jmodel)
{
    jmodel.SetObject();

    auto& modelRes = mModelRes.at(modelID);
    const Model* pModel = modelRes.mpModel.get();

    // Export model properties
    addString(jmodel, allocator, SceneKeys::kFilename, stripDataDirectories(pModel->getFilename()));
    addString(jmodel, allocator, SceneKeys::kName, pModel->getName());

    if (getModel(modelID)->hasAnimations())
    {
        addLiteral(jmodel, allocator, SceneKeys::kActiveAnimation, pModel->getActiveAnimation());
    }


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
        // Translate rotation to degrees
        glm::vec3 rotation = glm::degrees(pInstance->getRotation());
        addVector(jsonInstance, allocator, SceneKeys::kRotationVec, rotation);
        addVector(jsonInstance, allocator, SceneKeys::kScalingVec, pInstance->getScaling());

        // Export model material properties

        switch (pModel->getMesh(0)->getMaterial()->getShadingModel())
        {
        case ShadingModelMetalRough:
            addString(jsonInstance, allocator, SceneKeys::kShadingModel, SceneKeys::kShadingMetalRough);
            break;
        case ShadingModelSpecGloss:
            addString(jsonInstance, allocator, SceneKeys::kShadingModel, SceneKeys::kShadingSpecGloss);
            break;
        default:
            logWarning("SceneExporter: Unknown shading model found on model " + pModel->getName() + ", ignoring value");
        }

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

            rapidjson::Value jsonMaterialParams;
            jsonMaterialParams.SetObject();

            if (mat->bUseMaterial) {
                auto& pMat = mat->mpMaterial;
                const std::string sTex2d(ParameterPrefix::kTexture2D);
                addString(jsonMaterialParams, allocator, sTex2d + ParameterDefault::kBaseColor, pMat->getBaseColorTexture()->getSourceFilename());
                addString(jsonMaterialParams, allocator, sTex2d + ParameterDefault::kSpecular, pMat->getSpecularTexture()->getSourceFilename());
                addString(jsonMaterialParams, allocator, sTex2d + ParameterDefault::kNormalMap, pMat->getNormalMap()->getSourceFilename());
                addString(jsonMaterialParams, allocator, sTex2d + ParameterDefault::kEmissive, pMat->getEmissiveTexture()->getSourceFilename());
                addString(jsonMaterialParams, allocator, sTex2d + ParameterDefault::kOcclusionMap, pMat->getOcclusionMap()->getSourceFilename());
            }

            for (auto& v : mat->param_bool) {
                addFloat(jsonMaterialParams, allocator, ParameterPrefix::kBool + v.first, v.second);
            }
            for (auto& v : mat->param_float) {
                addFloat(jsonMaterialParams, allocator, ParameterPrefix::kFloat + v.first, v.second);
            }
            for (auto& v : mat->param_vec2) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kFloat2 + v.first, v.second);
            }
            for (auto& v : mat->param_vec3) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kFloat3 + v.first, v.second);
            }
            for (auto& v : mat->param_vec4) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kFloat4 + v.first, v.second);
            }
            for (auto& v : mat->param_int) {
                addInt(jsonMaterialParams, allocator, ParameterPrefix::kInt + v.first, v.second);
            }
            for (auto& v : mat->param_ivec2) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kInt2 + v.first, v.second);
            }
            for (auto& v : mat->param_ivec3) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kInt3 + v.first, v.second);
            }
            for (auto& v : mat->param_ivec4) {
                addVector(jsonMaterialParams, allocator, ParameterPrefix::kInt4 + v.first, v.second);
            }
            for (auto& v : mat->param_texture2D) {
                addString(jsonMaterialParams, allocator, ParameterPrefix::kTexture2D + v.first, v.second->getSourceFilename());
            }
            addJsonValue(jsonMaterial, allocator, SceneExtendKeys::kParameter, jsonMaterialParams);
            //todo : mat 写入
            jsonMaterialArray.PushBack(jsonMaterial, allocator);

            //const auto& shaderName = ;
        }
        rapidjson::Value jkey;
        std::string k(SceneExtendKeys::kMaterials);
        jkey.SetString(k.c_str(), rapidjson::SizeType(k.size()), allocator);
        jsonInstance.AddMember(jkey, jsonMaterialArray, allocator);
        jsonInstanceArray.PushBack(jsonInstance, allocator);

    }

    addJsonValue(jmodel, allocator, SceneKeys::kModelInstances, jsonInstanceArray);
}

void SceneExtend::writeModelsJson()
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
