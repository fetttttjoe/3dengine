#include "Sculpting/Tools/PushPullTool.h"

#include <glm/gtx/norm.hpp>

#include "Core/UI/BrushSettings.h"
#include "Interfaces/IEditableMesh.h"

void PushPullTool::Apply(IEditableMesh& mesh, const glm::vec3& hitPoint,
                         const glm::vec3& rayDirection,
                         const glm::vec2& mouseDelta,
                         const BrushSettings& settings,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix, int viewportWidth,
                         int viewportHeight) {
  float brushRadiusSq = settings.radius * settings.radius;
  float direction = (settings.mode == SculptMode::Pull) ? 1.0f : -1.0f;

  auto& vertices = mesh.GetVertices();
  const auto& normals = mesh.GetNormals();

  for (size_t i = 0; i < vertices.size(); ++i) {
    glm::vec3& vertex = vertices[i];
    float distSq = glm::distance2(hitPoint, vertex);

    if (distSq < brushRadiusSq) {
      float normalizedDist = glm::sqrt(distSq) / settings.radius;
      float falloff = settings.falloff.Evaluate(normalizedDist);
      const glm::vec3& normal = normals[i];
      vertex += normal * direction * settings.strength * falloff * 0.1f;
    }
  }
}