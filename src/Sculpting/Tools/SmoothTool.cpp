#include "Sculpting/Tools/SmoothTool.h"

#include <glm/gtx/norm.hpp>

#include "Core/UI/BrushSettings.h"
#include "Interfaces/IEditableMesh.h"

void SmoothTool::Apply(IEditableMesh& mesh, const glm::vec3& hitPoint,
                       const glm::vec3& rayDirection,
                       const glm::vec2& mouseDelta,
                       const BrushSettings& settings,
                       const glm::mat4& viewMatrix,
                       const glm::mat4& projectionMatrix, int viewportWidth,
                       int viewportHeight) {
  float brushRadiusSq = settings.radius * settings.radius;
  auto& vertices = mesh.GetVertices();
  std::vector<glm::vec3> smoothedVertices = vertices;
  std::vector<size_t> affectedIndices;

  for (size_t i = 0; i < vertices.size(); ++i) {
    if (glm::distance2(hitPoint, vertices[i]) < brushRadiusSq) {
      affectedIndices.push_back(i);
    }
  }

  if (affectedIndices.size() < 2) {
    return;
  }

  glm::vec3 centerOfMass(0.0f);
  for (size_t index : affectedIndices) {
    centerOfMass += vertices[index];
  }
  centerOfMass /= affectedIndices.size();

  for (size_t index : affectedIndices) {
    const glm::vec3& originalVertex = vertices[index];
    float falloff = settings.falloff.Evaluate(
        glm::sqrt(glm::distance2(hitPoint, originalVertex)) / settings.radius);
    smoothedVertices[index] =
        glm::mix(originalVertex, centerOfMass, settings.strength * falloff);
  }

  mesh.GetVertices() = smoothedVertices;
}