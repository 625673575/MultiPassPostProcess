#include "SceneRendererExtend.h"



SceneRendererExtend::SceneRendererExtend(const Scene::SharedPtr& pScene) :SceneRenderer(pScene)
{
}

void SceneRendererExtend::renderScene(RenderContext * pContext, const Camera * pCamera)
{
    updateVariableOffsets(pContext->getGraphicsVars()->getReflection().get());

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

void SceneRendererExtend::render(CurrentWorkingData & currentData)
{
    setPerFrameData(currentData);

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

                        // Loop over the meshes
                        for (uint32_t meshID = 0; meshID < pInstance->getObject()->getMeshCount(); meshID++)
                        {
                            renderMeshInstancesExtend(currentData, pInstance, meshID);
                        }
                    }
                }
            }
        }
    }
}

void SceneRendererExtend::renderMeshInstancesExtend(CurrentWorkingData & currentData, const Scene::ModelInstance * pModelInstance, uint32_t meshID)
{
    const Model* pModel = currentData.pModel;
    const Mesh* pMesh = pModel->getMesh(meshID).get();

    if (setPerMeshData(currentData, pMesh))
    {
        Program* pProgram = currentData.pState->getProgram().get();
        bool useVsSkinning = pMesh->hasBones() && !pModel->getSkinningCache();
        if (useVsSkinning)
        {
            pProgram->addDefine("_VERTEX_BLENDING");
        }

        // Bind VAO and set topology            
        currentData.pState->setVao(useVsSkinning ? pMesh->getVao() : pModel->getMeshVao(pMesh));

        uint32_t activeInstances = 0;

        const uint32_t instanceCount = pModel->getMeshInstanceCount(meshID);
        for (uint32_t instanceID = 0; instanceID < instanceCount; instanceID++)
        {
            const Model::MeshInstance* pMeshInstance = pModel->getMeshInstance(meshID, instanceID).get();

            if (pMeshInstance->isVisible())
            {
                if ((mCullEnabled == false) || (cullMeshInstance(currentData, pModelInstance, pMeshInstance) == false))
                {
                    if (setPerMeshInstanceData(currentData, pModelInstance, pMeshInstance, activeInstances))
                    {
                        currentData.drawID++;
                        activeInstances++;

                        if (activeInstances == mMaxInstanceCount)
                        {
                            // DISABLED_FOR_D3D12
                            //pContext->setProgram(currentData.pProgram->getActiveProgramVersion());
                            draw(currentData, pMesh, activeInstances);
                            activeInstances = 0;
                        }
                    }
                }
            }
        }
        if (activeInstances != 0)
        {
            draw(currentData, pMesh, activeInstances);
        }

        // Restore the program state
        if (useVsSkinning)
        {
            pProgram->removeDefine("_VERTEX_BLENDING");
        }
    }
}
