#pragma once

#include "Sculpting/ISculptTool.h"

/**
 * @class SmoothTool
 * @brief A sculpting tool that averages vertex positions to smooth the mesh.
 */
class SmoothTool : public ISculptTool {
 public:
  void Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
             const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
             const BrushSettings& settings, const glm::mat4& viewMatrix,
             const glm::mat4& projectionMatrix, int viewportWidth,
             int viewportHeight) override;
};