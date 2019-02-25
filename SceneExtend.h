#pragma once
#include "Falcor.h"
#include "ModelResource.h"
using namespace Falcor;
class SceneExtend :
	public Scene
{
public:
    SceneExtend(const std::string& filename = "");
  ~SceneExtend();
  void addModelResource(const ModelResource& pModel, const std::string& instanceName, const glm::vec3& translation = glm::vec3(), const glm::vec3& yawPitchRoll = glm::vec3(), const glm::vec3& scaling = glm::vec3(1));
  ModelResource& getModelResource(uint32_t id) { return modelsRes[id]; }
private:
    std::vector<ModelResource> modelsRes;
};

