#include "SceneRendererExtend.h"
#include "ModelResource.h"


SceneRendererExtend::SceneRendererExtend(const Scene::SharedPtr& pScene) :SceneRenderer(pScene)
{
}

void SceneRendererExtend::renderScene(RenderContext * pContext, const Fbo::SharedPtr& fbo, const Camera * pCamera, bool renderDepth)
{
    mpFbo = fbo;
    mSetShaderForEachMaterial = renderDepth;
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

void SceneRendererExtend::setPerFrameData(const CurrentWorkingData & currentData)
{
    ConstantBuffer* pCB = currentData.pContext->getGraphicsVars()->getConstantBuffer(kPerFrameCbName).get();
    if (pCB)
    {
        // Set camera
        if (currentData.pCamera)
        {
            currentData.pCamera->setIntoConstantBuffer(pCB, sCameraDataOffset);
        }

        // Set lights
        if (sLightArrayOffset != ConstantBuffer::kInvalidOffset)
        {
            assert(mpScene->getLightCount() <= MAX_LIGHT_SOURCES);  // Max array size in the shader
            for (uint32_t i = 0; i < mpScene->getLightCount(); i++)
            {
                mpScene->getLight(i)->setIntoProgramVars(currentData.pVars, pCB, sLightArrayOffset + (i * Light::getShaderStructSize()));
            }
        }
        if (sLightCountOffset != ConstantBuffer::kInvalidOffset)
        {
            pCB->setVariable(sLightCountOffset, mpScene->getLightCount());
        }
        if (mpScene->getLightProbeCount() > 0)
        {
            // #TODO Support multiple light probes
            LightProbe::setCommonIntoProgramVars(currentData.pVars, kProbeSharedVarName);
            mpScene->getLightProbe(0)->setIntoProgramVars(currentData.pVars, pCB, kProbeVarName);
        }
    }

    if (mpScene->getAreaLightCount() > 0)
    {
        const ParameterBlockReflection* pBlock = currentData.pVars->getReflection()->getDefaultParameterBlock().get();

        // If area lights have been declared
        const ReflectionVar* pVar = pBlock->getResource(kAreaLightCbName).get();
        if (pVar != nullptr)
        {
            const ReflectionVar* pAreaLightVar = pVar->getType()->findMember("gAreaLights").get();
            assert(pAreaLightVar != nullptr);

            uint32_t areaLightArraySize = pAreaLightVar->getType()->asArrayType()->getArraySize();
            for (uint32_t i = 0; i < min(areaLightArraySize, mpScene->getAreaLightCount()); i++)
            {
                std::string varName = "gAreaLights[" + std::to_string(i) + "]";
                mpScene->getAreaLight(i)->setIntoProgramVars(currentData.pVars, currentData.pVars->getConstantBuffer(kAreaLightCbName).get(), varName.c_str());
            }
        }
    }
}

bool SceneRendererExtend::setPerModelData(const CurrentWorkingData & currentData)
{
    const Model* pModel = currentData.pModel;

    // Set bones
    if (pModel->hasBones())
    {
        ConstantBuffer* pCB = currentData.pContext->getGraphicsVars()->getConstantBuffer(kBoneCbName).get();
        if (pCB != nullptr)
        {
            if (sBonesOffset == ConstantBuffer::kInvalidOffset || sBonesInvTransposeOffset == ConstantBuffer::kInvalidOffset)
            {
                sBonesOffset = pCB->getVariableOffset("gBoneMat[0]");
                sBonesInvTransposeOffset = pCB->getVariableOffset("gInvTransposeBoneMat[0]");
            }

            assert(pModel->getBoneCount() <= MAX_BONES);
            pCB->setVariableArray(sBonesOffset, pModel->getBoneMatrices(), pModel->getBoneCount());
            pCB->setVariableArray(sBonesInvTransposeOffset, pModel->getBoneInvTransposeMatrices(), pModel->getBoneCount());
        }
    }
    return true;
}

bool SceneRendererExtend::setPerMeshInstanceData(const CurrentWorkingData & currentData, const Scene::ModelInstance * pModelInstance, const Model::MeshInstance * pMeshInstance, uint32_t drawInstanceID)
{
    ConstantBuffer* pCB = currentData.pContext->getGraphicsVars()->getConstantBuffer(kPerMeshCbName).get();
    if (pCB)
    {
        const Mesh* pMesh = pMeshInstance->getObject().get();

        glm::mat4 worldMat = pModelInstance->getTransformMatrix();
        glm::mat4 prevWorldMat = pModelInstance->getPrevTransformMatrix();

        if (pMesh->hasBones() == false)
        {
            worldMat = worldMat * pMeshInstance->getTransformMatrix();
            prevWorldMat = prevWorldMat * pMeshInstance->getPrevTransformMatrix();
        }

        glm::mat3x4 worldInvTransposeMat = transpose(inverse(glm::mat3(worldMat)));

        assert(drawInstanceID < sWorldMatArraySize);
        pCB->setBlob(&worldMat, sWorldMatOffset + drawInstanceID * sizeof(glm::mat4), sizeof(glm::mat4));
        pCB->setBlob(&worldInvTransposeMat, sWorldInvTransposeMatOffset + drawInstanceID * sizeof(glm::mat3x4), sizeof(glm::mat3x4)); // HLSL uses column-major and packing rules require 16B alignment, hence use glm:mat3x4
        pCB->setBlob(&prevWorldMat, sPrevWorldMatOffset + drawInstanceID * sizeof(glm::mat4), sizeof(glm::mat4));

        // Set mesh id
        pCB->setVariable(sMeshIdOffset, pMesh->getId());
    }

    return true;
}

bool SceneRendererExtend::setPerMaterialData(const CurrentWorkingData & currentData, const Material * pMaterial)
{
    currentData.pContext->getGraphicsVars()->setParameterBlock("gMaterial", pMaterial->getParameterBlock());
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

                        ModelResource& res = getScene()->getModelResource(modelID);
                        pInstance->setTranslation(res.Translation, true);
                        pInstance->setRotation(res.Rotation);
                        pInstance->setScaling(res.Scale);

                        // Loop over the meshes
                        for (uint32_t meshID = 0; meshID < pInstance->getObject()->getMeshCount(); meshID++)
                        {
                            if (mSetShaderForEachMaterial) {
                                MaterialInstance::SharedPtr& mat = res.getMaterialInstance(meshID);
                                currentData.pState = mat->get_state().get();
                                currentData.pVars = mat->get_programVars().get();
                                currentData.pState->setFbo(mpFbo);
                                mat->onRender(currentData.pContext);
                            }
                            updateVariableOffsets(currentData.pVars->getReflection().get());
                            setPerFrameData(currentData);
                            renderMeshInstances(currentData, pInstance, meshID);
                        }
                    }
                }
            }
        }
    }
}
