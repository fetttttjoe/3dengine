#include "Sculpting/Tools/PushPullTool.h"

#include <glm/gtx/norm.hpp>

#include "Sculpting/SculptableMesh.h"

void PushPullTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                         float brushRadius, float brushStrength,
                         SculptMode::Mode mode) {
  float brushRadiusSq = brushRadius * brushRadius;
  float direction = (mode == SculptMode::Pull) ? 1.0f : -1.0f;

  for (size_t i = 0; i < mesh.m_Vertices.size(); ++i) {
    glm::vec3& vertex = mesh.m_Vertices[i];
    float distSq = glm::distance2(hitPoint, vertex);

    if (distSq < brushRadiusSq) {
      // Calculate falloff (e.g., smoothstep)
      float falloff = 1.0f - (distSq / brushRadiusSq);
      falloff = falloff * falloff * (3.0f - 2.0f * falloff);  // Smoothstep

      // Apply displacement
      const glm::vec3& normal = mesh.m_Normals[i];
      vertex += normal * direction * brushStrength * falloff * 0.1f;
    }
  }
}
