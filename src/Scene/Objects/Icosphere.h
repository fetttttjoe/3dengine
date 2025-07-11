#pragma once

#include <map>

#include "Scene/Objects/BaseObject.h"

/**
 * @class Icosphere
 * @brief A sphere with evenly distributed vertices, ideal for sculpting.
 *
 * This class generates a sphere by recursively subdividing an icosahedron,
 * resulting in a mesh with much more uniform topology than a traditional UV
 * sphere.
 */
class Icosphere : public BaseObject {
 public:
  Icosphere();
  ~Icosphere() override = default;

  std::string GetTypeString() const override;

  // --- IGizmoClient Overrides ---
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override;

 protected:
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;

 private:
  // Helper function for recursive subdivision
  int getMiddlePoint(int p1, int p2, std::vector<glm::vec3>& vertices,
                     std::map<int64_t, int>& middlePointIndexCache);

  int m_RecursionLevel = 4;
};