#pragma once

#include "Scene/Objects/BaseObject.h"

class Pyramid : public BaseObject {
 public:
  Pyramid();
  ~Pyramid() override = default;

  // ISceneObject overrides
  std::string GetTypeString() const override;
  glm::vec3 GetLocalCenter() const override;

 protected:
  // BaseObject override
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};
