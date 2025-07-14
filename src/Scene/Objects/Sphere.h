#pragma once

#include "Scene/Objects/ScalableSphereObject.h"

class Sphere : public ScalableSphereObject {
 public:
  Sphere();
  ~Sphere() override = default;

  std::string GetTypeString() const override;
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;

 protected:
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};