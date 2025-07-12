#include "Sculpting/Tools/GrabTool.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "Core/Camera.h"
#include "Core/UI/BrushSettings.h"
#include "Sculpting/SculptableMesh.h"

void GrabTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                     const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                     const BrushSettings& settings, const glm::mat4& viewMatrix,
                     const glm::mat4& projectionMatrix, int viewportWidth,
                     int viewportHeight) {
  if (glm::length(mouseDelta) == 0.0f) return;

  float brushRadiusSq = settings.radius * settings.radius;

  // Project the hit point to screen space to find its depth
  glm::mat4 viewProj = projectionMatrix * viewMatrix;
  glm::vec2 screenPos =
      Camera::WorldToScreen(hitPoint, viewProj, viewportWidth, viewportHeight);

  // Unproject the start and end points of the mouse drag to find the world
  // space delta
  glm::mat4 invViewProj = glm::inverse(viewProj);
  glm::vec3 worldPosStart = Camera::ScreenToWorldPoint(
      screenPos, screenPos.y, invViewProj, viewportWidth, viewportHeight);
  glm::vec3 worldPosEnd =
      Camera::ScreenToWorldPoint(screenPos + mouseDelta, screenPos.y,
                                 invViewProj, viewportWidth, viewportHeight);

  glm::vec3 worldDelta = (worldPosEnd - worldPosStart) * settings.strength *
                         2.0f;  // Grab is less sensitive

  for (size_t i = 0; i < mesh.m_Vertices.size(); ++i) {
    glm::vec3& vertex = mesh.m_Vertices[i];
    float distSq = glm::distance2(hitPoint, vertex);

    if (distSq < brushRadiusSq) {
      float normalizedDist = glm::sqrt(distSq) / settings.radius;
      float falloff = settings.falloff.Evaluate(normalizedDist);
      vertex += worldDelta * falloff;
    }
  }
}