#include "SceneExtend.h"


SceneExtend::SceneExtend(const std::string & filename) :Scene(filename)
{
}

void SceneExtend::addModelResource(const ModelResource & pModel, const std::string & instanceName, const glm::vec3 & translation, const glm::vec3 & yawPitchRoll, const glm::vec3 & scaling)
{
    addModelInstance(pModel.mpModel, instanceName, translation, yawPitchRoll, scaling);
    modelsRes.emplace_back(pModel);
}

SceneExtend::~SceneExtend()
{
}
