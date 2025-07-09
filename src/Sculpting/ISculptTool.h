#pragma once

#include <glm/glm.hpp>

// Forward declarations
class SculptableMesh;

namespace SculptMode {
enum Mode { Push, Pull, Smooth };
}

/**
 * @class ISculptTool
 * @brief Abstract interface for all sculpting tools.
 *
 * This defines the contract for any tool that can modify a SculptableMesh.
 * It ensures that the application can handle different tools in a uniform way.
 */
class ISculptTool {
 public:
  virtual ~ISculptTool() = default;

  /**
   * @brief Applies the sculpting effect to a mesh.
   * @param mesh The mesh to be modified.
   * @param hitPoint The point in local space where the brush hits the mesh.
   * @param brushRadius The radius of the sculpting brush.
   * @param brushStrength The strength/intensity of the sculpting effect.
   * @param mode The sculpting mode (e.g., Push or Pull).
   */
  virtual void Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                     float brushRadius, float brushStrength,
                     SculptMode::Mode mode) = 0;
};
