//单个FBX加载多个Mesh，并且每个Mesh有独立的Material
#pragma once

#include "Falcor.h"
#include "MaterialInstance.h"
using namespace Falcor;
class ModelResource {
public:
    ModelResource() = default;
    ~ModelResource() = default;
    ModelResource(const Model::SharedPtr& pModel);
public:
    std::string mModelString;
    Model::SharedPtr mpModel = nullptr;
    bool mAnimate = false;
    bool mUseTriLinearFiltering = false;
    std::string getModelDesc(bool isAfterCull, float loadTime);
    std::map<std::string, MaterialInstance::SharedPtr> sharedMaterials;
    std::vector<std::string> getMaterialsName();
    size_t getMaterialCount();
    void init();
    void renderModel(RenderContext* pRenderContext);
    void renderMaterialGui(Gui* p);
private:
    void initMaterials();
    void renderMeshInstances(RenderContext* pRenderContext);
 
};
