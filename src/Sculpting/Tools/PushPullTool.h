#pragma once

#include "Sculpting/ISculptTool.h"

/**
 * @class PushPullTool
 * @brief A concrete sculpting tool that pushes or pulls vertices along their
 * normals.
 */
class PushPullTool : public ISculptTool {
 public:
  void Apply(SculptableMesh& mesh, const glm::vec3& hitPoint, float brushRadius,
             float brushStrength, SculptMode::Mode mode) override;
};
