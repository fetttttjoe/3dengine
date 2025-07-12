#pragma once

#include "Scene/Objects/ScalableSphereObject.h"  // Inherit from the new class

class Sphere : public ScalableSphereObject {  // Changed base class
 public:
  Sphere();
  ~Sphere() override = default;

  std::string GetTypeString() const override;
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;

 protected:
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};