#pragma once

#include "Scene/Objects/BaseObject.h"

class Triangle : public BaseObject {
 public:
  Triangle();
  ~Triangle() override = default;

  // ISceneObject override
  std::string GetTypeString() const override;
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;

 protected:
  // This is a helper for the transform calculation, not part of the public API
  glm::vec3 GetLocalCenter() const override;

  // BaseObject override
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};