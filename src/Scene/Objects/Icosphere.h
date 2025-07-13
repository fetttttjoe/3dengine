#pragma once

#include <map>

#include "Scene/Objects/ScalableSphereObject.h"

class Icosphere : public ScalableSphereObject {
 public:
  Icosphere();
  ~Icosphere() override = default;

  // ISceneObject overrides
  std::string GetTypeString() const override;
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;

 protected:
  // BaseObject override to define the object's geometry
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;

 private:
  // Helper function for recursively subdividing the mesh
  int getMiddlePoint(int p1, int p2, std::vector<glm::vec3>& vertices,
                     std::map<int64_t, int>& middlePointIndexCache);
  int m_RecursionLevel = 4;
};