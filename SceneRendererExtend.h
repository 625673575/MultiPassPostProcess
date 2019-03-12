#pragma once
#include "Falcor.h"
#include "SceneExtend.h"

using namespace Falcor;

class SceneRendererExtend :
    public SceneRenderer
{
public:
    using SharedPtr = std::shared_ptr<SceneRendererExtend>;
    using SharedConstPtr = std::shared_ptr<const SceneRendererExtend>;
    static SceneRendererExtend::SharedPtr create(const Scene::SharedPtr& pScene)
    {
        return SharedPtr(new SceneRendererExtend(pScene));
    }
    SceneRendererExtend(const Scene::SharedPtr& pScene);
    virtual ~SceneRendererExtend() = default;

    void renderScene(RenderContext* pContext, const Fbo::SharedPtr& fbo, const Camera* pCamera, bool setShaderForEachMaterial);
    void setPerFrameData(const CurrentWorkingData& currentData)override;
    bool setPerModelData(const CurrentWorkingData& currentData)override;
    bool setPerMeshInstanceData(const CurrentWorkingData& currentData, const Scene::ModelInstance* pModelInstance, const Model::MeshInstance* pMeshInstance, uint32_t drawInstanceID)override;
    bool setPerMaterialData(const CurrentWorkingData& currentData, const Material* pMaterial)override;

protected:
    GraphicsProgram::SharedPtr mpDefaultProgram;
    GraphicsVars::SharedPtr mpDefaultProgramVars;
    Fbo::SharedPtr mpFbo;
    struct MaterialInstanceBuffer{
        MaterialInstance*  pMaterialInstance;
        ObjectInstance<Model> * pModelInstance;
        uint32_t meshId;
        CurrentWorkingData data;
        MaterialInstanceBuffer(MaterialInstance* materialInst, ObjectInstance<Model> * model, uint32_t id,const CurrentWorkingData&d) {
            pMaterialInstance = materialInst;
            pModelInstance = model;
            meshId = id;
            data = d;
        }
    };
    std::vector<MaterialInstanceBuffer> mSavedInstance;
    bool mSetShaderForEachMaterial;
    void render(CurrentWorkingData& currentData);
    SceneExtend* getScene() { return static_cast<SceneExtend*>(mpScene.get()); }
};

