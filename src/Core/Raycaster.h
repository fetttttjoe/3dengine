#pragma once

#include <glm/glm.hpp>
#include <limits>

// Forward declarations
class SculptableMesh;

namespace Raycaster {

struct RaycastResult {
  bool hit = false;
  float distance = std::numeric_limits<float>::max();
  glm::vec3 hitPoint = glm::vec3(0.0f);
};

/**
 * @brief Performs a ray-mesh intersection test.
 * * @param rayOrigin The starting point of the ray in world space.
 * @param rayDirection The direction of the ray in world space.
 * @param mesh The mesh to test against.
 * @param modelMatrix The transformation matrix of the mesh.
 * @param outResult The result of the raycast if a hit occurs.
 * @return True if the ray intersects the mesh, false otherwise.
 */
bool IntersectMesh(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                   const SculptableMesh& mesh, const glm::mat4& modelMatrix,
                   RaycastResult& outResult);

}  // namespace Raycaster