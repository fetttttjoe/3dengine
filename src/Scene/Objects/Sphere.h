#pragma once

#include "Scene/Objects/BaseObject.h"

class Sphere : public BaseObject {
 public:
  Sphere();
  ~Sphere() override = default;

  std::string GetTypeString() const override;

  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override;

 protected:
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};