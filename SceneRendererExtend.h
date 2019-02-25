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

    void renderScene(RenderContext* pContext, const Camera* pCamera)override;
    bool setPerMaterialData(const CurrentWorkingData& currentData, const Material* pMaterial)override;

protected:
    GraphicsProgram::SharedPtr mpDefaultProgram;
    GraphicsVars::SharedPtr mpDefaultProgramVars;
    void render(CurrentWorkingData& currentData);
     SceneExtend* getScene() { return static_cast<SceneExtend*>( mpScene.get()); }
};

