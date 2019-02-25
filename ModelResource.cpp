#include "ModelResource.h"

ModelResource::ModelResource(const Model::SharedPtr& pModel)
{
    mpModel = pModel;
}

std::string ModelResource::getModelDesc(bool isAfterCull, float loadTime)
{
    std::string mModelString = isAfterCull ? "Mesh culling" : "Loading";
    mModelString += " took " + std::to_string(loadTime) + " seconds.\n";
    mModelString += "Model has " + std::to_string(mpModel->getVertexCount()) + " vertices, ";
    mModelString += std::to_string(mpModel->getIndexCount()) + " indices, ";
    mModelString += std::to_string(mpModel->getPrimitiveCount()) + " primitives, ";
    mModelString += std::to_string(mpModel->getMeshCount()) + " meshes, ";
    mModelString += std::to_string(mpModel->getInstanceCount()) + " mesh instances, ";
    mModelString += std::to_string(mpModel->getMaterialCount()) + " materials, ";
    mModelString += std::to_string(mpModel->getTextureCount()) + " textures, ";
    mModelString += std::to_string(mpModel->getBufferCount()) + " buffers.\n";
    return mModelString;
}

std::vector<std::string> ModelResource::getMaterialsName()
{
    std::vector<std::string> r;
    for (auto&v : sharedMaterials)
        r.emplace_back(v.first);
    return r;
}

size_t ModelResource::getMaterialCount()
{
    return sharedMaterials.size();
}

void ModelResource::init()
{
    initMaterials();
}

void ModelResource::setBuffers(RenderContext* pRenderContext, uint32_t meshID)
{
    auto& pMat = mpModel->getMesh(meshID)->getMaterial();
    auto& MatInstance = sharedMaterials[pMat->getName()];
    MatInstance->onRender(pRenderContext);
}

void ModelResource::renderMaterialGui(Gui* p)
{
    for (auto&v : sharedMaterials) {
        if (v.second) {
            if (p->beginGroup(v.first, true)) {
                v.second->onMaterialGui(p);
                p->endGroup();
            }
        }
    }
}

void ModelResource::initMaterials()
{
    for (uint32_t i = 0; i < mpModel->getMeshCount(); i++) {
        auto& mat = mpModel->getMesh(i)->getMaterial();
        const auto& matName = mat->getName();
        if (sharedMaterials.find(matName) != sharedMaterials.end())
            continue;
        auto inst = std::make_shared<MaterialInstance>(matName);
        sharedMaterials.emplace(matName, inst);
    }
}
