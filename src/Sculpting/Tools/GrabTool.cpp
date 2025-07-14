#include "Sculpting/Tools/GrabTool.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "Core/MathHelpers.h"
#include "Core/UI/BrushSettings.h"
#include "Interfaces/IEditableMesh.h"

void GrabTool::Apply(IEditableMesh& mesh, const glm::vec3& hitPoint,
                     const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                     const BrushSettings& settings, const glm::mat4& viewMatrix,
                     const glm::mat4& projectionMatrix, int viewportWidth,
                     int viewportHeight) {
  if (glm::length(mouseDelta) == 0.0f) return;

  float brushRadiusSq = settings.radius * settings.radius;

  glm::mat4 viewProj = projectionMatrix * viewMatrix;
  glm::vec2 screenPos = MathHelpers::WorldToScreen(
      hitPoint, viewProj, viewportWidth, viewportHeight);

  glm::mat4 invViewProj = glm::inverse(viewProj);
  glm::vec3 worldPosStart = MathHelpers::ScreenToWorldPoint(
      screenPos, screenPos.y, invViewProj, viewportWidth, viewportHeight);
  glm::vec3 worldPosEnd = MathHelpers::ScreenToWorldPoint(
      screenPos + mouseDelta, screenPos.y, invViewProj, viewportWidth,
      viewportHeight);

  glm::vec3 worldDelta =
      (worldPosEnd - worldPosStart) * settings.strength * 0.2f;

  auto& vertices = mesh.GetVertices();
  for (size_t i = 0; i < vertices.size(); ++i) {
    glm::vec3& vertex = vertices[i];
    float distSq = glm::distance2(hitPoint, vertex);

    if (distSq < brushRadiusSq) {
      float normalizedDist = glm::sqrt(distSq) / settings.radius;
      float falloff = settings.falloff.Evaluate(normalizedDist);
      vertex += worldDelta * falloff;
    }
  }
}