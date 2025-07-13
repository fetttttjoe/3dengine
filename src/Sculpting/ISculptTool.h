#pragma once

#include <cstdint>
#include <glm/glm.hpp>

class IEditableMesh;
struct BrushSettings;

namespace SculptMode {
// This enum defines different *brush tool types*.
// MoveVertex and SelectFace have been moved out as they are now sub-object
// modes.
enum Mode { Pull = 0, Push, Smooth, Grab };  // Corrected enum definition
}  // namespace SculptMode

/**
 * @brief Base interface for all sculpting tools.
 */
class ISculptTool {
 public:
  virtual ~ISculptTool() = default;

  /**
   * @brief Applies the sculpting effect to the mesh.
   * @param mesh The mesh to sculpt.
   * @param hitPoint The world-space point where the ray intersected the mesh
   * (center of brush).
   * @param rayDirection The world-space direction of the ray.
   * @param mouseDelta The 2D mouse movement delta.
   * @param settings The current brush settings (radius, strength, falloff,
   * mode).
   * @param viewMatrix The current view matrix of the camera.
   * @param projectionMatrix The current projection matrix of the camera.
   * @param viewportWidth The width of the viewport.
   * @param viewportHeight The height of the viewport.
   */
  virtual void Apply(IEditableMesh& mesh, const glm::vec3& hitPoint,
                     const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                     const BrushSettings& settings, const glm::mat4& viewMatrix,
                     const glm::mat4& projectionMatrix, int viewportWidth,
                     int viewportHeight) = 0;
};