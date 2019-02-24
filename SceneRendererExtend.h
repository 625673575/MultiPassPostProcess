#pragma once
#include "Falcor.h"

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

    void renderScene(RenderContext* pContext, const Camera* pCamera)override;
protected:
    void render(CurrentWorkingData& currentData);
    void renderMeshInstancesExtend(CurrentWorkingData& currentData, const Scene::ModelInstance* pModelInstance, uint32_t meshID);
};

