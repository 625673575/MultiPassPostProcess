#include "SceneRendererExtend.h"
#include "ModelResource.h"


SceneRendererExtend::SceneRendererExtend(const Scene::SharedPtr& pScene) :SceneRenderer(pScene)
{
}

void SceneRendererExtend::renderScene(RenderContext * pContext, const Camera * pCamera)
{
    updateVariableOffsets(pContext->getGraphicsVars()->getReflection().get());
    //设置默认的shader和var
    //mpDefaultProgram = pContext->getGraphicsState()->getProgram();
    //mpDefaultProgramVars = pContext->getGraphicsVars();

    CurrentWorkingData currentData;
    currentData.pContext = pContext;
    currentData.pState = pContext->getGraphicsState().get();
    currentData.pVars = pContext->getGraphicsVars().get();
    currentData.pCamera = pCamera;
    currentData.pMaterial = nullptr;
    currentData.pModel = nullptr;
    currentData.drawID = 0;
    render(currentData);
}

bool SceneRendererExtend::setPerMaterialData(const CurrentWorkingData & currentData, const Material * pMaterial)
{
    //currentData.pVars->setParameterBlock("gMaterial", pMaterial->getParameterBlock());
    return true;
}

void SceneRendererExtend::render(CurrentWorkingData & currentData)
{
    for (uint32_t modelID = 0; modelID < mpScene->getModelCount(); modelID++)
    {
        currentData.pModel = mpScene->getModel(modelID).get();

        if (setPerModelData(currentData))
        {
            for (uint32_t instanceID = 0; instanceID < mpScene->getModelInstanceCount(modelID); instanceID++)
            {
                const auto pInstance = mpScene->getModelInstance(modelID, instanceID).get();

                if (pInstance->isVisible())
                {
                    if (setPerModelInstanceData(currentData, pInstance, instanceID))
                    {
                        mpLastMaterial = nullptr;

                        auto res = getScene()->getModelResource(modelID);
                        pInstance->setTranslation(res.Translation, true);
                        pInstance->setRotation(res.Rotation);
                        pInstance->setScaling(res.Scale);

                        // Loop over the meshes
                        for (uint32_t meshID = 0; meshID < pInstance->getObject()->getMeshCount(); meshID++)
                        {
                            res.setBuffers(currentData.pContext, currentData.pVars, meshID);
                            updateVariableOffsets(currentData.pContext->getGraphicsVars()->getReflection().get());
                            setPerFrameData(currentData);
                            renderMeshInstances(currentData, pInstance, meshID);
                        }
                    }
                }
            }
        }
    }
}
