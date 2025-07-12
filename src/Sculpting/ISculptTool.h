#pragma once

#include <glm/glm.hpp>

// Forward declarations
class SculptableMesh;
struct BrushSettings;

namespace SculptMode {
enum Mode { Push, Pull, Smooth, Grab };
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
   * @param hitPoint The point in world space where the brush hits the mesh.
   * @param rayDirection The direction of the mouse ray in world space.
   * @param mouseDelta The 2D movement of the mouse in screen space.
   * @param settings The brush configuration (radius, strength, falloff).
   * @param viewMatrix The camera's view matrix.
   * @param projectionMatrix The camera's projection matrix.
   * @param viewportWidth The width of the viewport.
   * @param viewportHeight The height of the viewport.
   */
  virtual void Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                     const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                     const BrushSettings& settings, const glm::mat4& viewMatrix,
                     const glm::mat4& projectionMatrix, int viewportWidth,
                     int viewportHeight) = 0;
};