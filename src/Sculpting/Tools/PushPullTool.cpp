#include "Sculpting/Tools/PushPullTool.h"

#include <glm/gtx/norm.hpp>

#include "Core/UI/BrushSettings.h"
#include "Sculpting/SculptableMesh.h"

void PushPullTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                         const glm::vec3& rayDirection,
                         const glm::vec2& mouseDelta,
                         const BrushSettings& settings,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix, int viewportWidth,
                         int viewportHeight) {
  float brushRadiusSq = settings.radius * settings.radius;
  float direction = (settings.mode == SculptMode::Pull) ? 1.0f : -1.0f;

  for (size_t i = 0; i < mesh.m_Vertices.size(); ++i) {
    glm::vec3& vertex = mesh.m_Vertices[i];
    float distSq = glm::distance2(hitPoint, vertex);

    if (distSq < brushRadiusSq) {
      float normalizedDist = glm::sqrt(distSq) / settings.radius;
      float falloff = settings.falloff.Evaluate(normalizedDist);
      const glm::vec3& normal = mesh.m_Normals[i];
      vertex += normal * direction * settings.strength * falloff * 0.1f;
    }
  }
}