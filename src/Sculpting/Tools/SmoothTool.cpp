#include "Sculpting/Tools/SmoothTool.h"

#include <glm/gtx/norm.hpp>

#include "Core/UI/BrushSettings.h"
#include "Sculpting/SculptableMesh.h"

void SmoothTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                       const glm::vec3& rayDirection,
                       const glm::vec2& mouseDelta,
                       const BrushSettings& settings,
                       const glm::mat4& viewMatrix,
                       const glm::mat4& projectionMatrix, int viewportWidth,
                       int viewportHeight) {
  float brushRadiusSq = settings.radius * settings.radius;
  std::vector<glm::vec3> smoothedVertices = mesh.m_Vertices;
  std::vector<size_t> affectedIndices;

  // 1. Identify all vertices that are under the brush's influence.
  for (size_t i = 0; i < mesh.m_Vertices.size(); ++i) {
    if (glm::distance2(hitPoint, mesh.m_Vertices[i]) < brushRadiusSq) {
      affectedIndices.push_back(i);
    }
  }

  // We need at least two vertices to perform a smooth operation.
  if (affectedIndices.size() < 2) {
    return;
  }

  // 2. Calculate the single average position (center of mass) for ALL affected
  // vertices.
  glm::vec3 centerOfMass(0.0f);
  for (size_t index : affectedIndices) {
    centerOfMass += mesh.m_Vertices[index];
  }
  centerOfMass /= affectedIndices.size();

  // 3. For each affected vertex, interpolate it towards the calculated center
  // of mass.
  for (size_t index : affectedIndices) {
    const glm::vec3& originalVertex = mesh.m_Vertices[index];

    // The falloff is based on the vertex's distance from the brush center.
    float falloff = settings.falloff.Evaluate(
        glm::sqrt(glm::distance2(hitPoint, originalVertex)) / settings.radius);

    // The final position is a mix between the original position and the
    // average, controlled by the brush strength and falloff.
    smoothedVertices[index] =
        glm::mix(originalVertex, centerOfMass, settings.strength * falloff);
  }

  // 4. Apply the newly calculated positions back to the mesh.
  mesh.m_Vertices = smoothedVertices;
}