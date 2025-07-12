#pragma once

#include <map>

#include "Scene/Objects/ScalableSphereObject.h"  // Inherit from the new class

class Icosphere : public ScalableSphereObject {  // Changed base class
 public:
  Icosphere();
  ~Icosphere() override = default;

  std::string GetTypeString() const override;
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;

 protected:
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;

 private:
  // The declaration for the essential helper function remains.
  int getMiddlePoint(int p1, int p2, std::vector<glm::vec3>& vertices,
                     std::map<int64_t, int>& middlePointIndexCache);
  int m_RecursionLevel = 4;
};