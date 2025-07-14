#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>  // Required for inverse()
#include <sstream>
#include <string>

#include "imgui.h"

// Provides simple conversion and complex 3D math helper functions.
namespace MathHelpers {

// --- Simple Type Conversions ---
// Converts an ImVec2 (ImGui's 2D vector type) to a glm::vec2.
inline glm::vec2 ToGlm(const ImVec2& v) { return glm::vec2(v.x, v.y); }
// Converts a glm::vec2 to an ImVec2 (ImGui's 2D vector type).
inline ImVec2 ToImGui(const glm::vec2& v) { return ImVec2(v.x, v.y); }

// --- String Conversions for Debugging ---
// Converts a glm::vec2 to a human-readable string format for debugging.
inline std::string ToString(const glm::vec2& v) {
  std::stringstream ss;
  ss << "vec2(" << v.x << ", " << v.y << ")";
  return ss.str();
}
// Converts a glm::vec3 to a human-readable string format for debugging.
inline std::string ToString(const glm::vec3& v) {
  std::stringstream ss;
  ss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
  return ss.str();
}
// Converts a glm::vec4 to a human-readable string format for debugging.
inline std::string ToString(const glm::vec4& v) {
  std::stringstream ss;
  ss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
  return ss.str();
}
// Converts an array of two ImVec2 points to a human-readable string format for
// debugging.
inline std::string ToString(const std::array<ImVec2, 2>& arr) {
  std::stringstream ss;
  ss << "[(" << arr[0].x << ", " << arr[0].y << "), (" << arr[1].x << ", "
     << arr[1].y << ")]";
  return ss.str();
}

// --- 3D Space Conversion Functions ---

/**
 * @brief Projects a 3D world-space point into 2D screen-space coordinates.
 *
 * This function takes a 3D point in your game or simulation world and
 * calculates where it would appear on the 2D screen of the application window.
 * It uses the combined View-Projection matrix (which transforms 3D world
 * coordinates into clip space) and then performs perspective division to get
 * Normalized Device Coordinates (NDC). Finally, NDC are scaled and shifted
 * to match your window's pixel coordinates.
 *
 * @param worldPos The 3D point in world coordinates.
 * @param viewProj The combined View and Projection matrix (camera's
 * perspective).
 * @param windowWidth The width of the application window in pixels.
 * @param windowHeight The height of the application window in pixels.
 * @return A glm::vec2 representing the 2D screen coordinates (x, y) of the
 * point. Returns (-1.0f, -1.0f) if the point's W component becomes zero after
 *         transformation, which indicates an invalid or infinitely distant
 * point.
 */
inline glm::vec2 WorldToScreen(const glm::vec3& worldPos,
                               const glm::mat4& viewProj, int windowWidth,
                               int windowHeight) {
  // Transform the world position into clip space using the view-projection
  // matrix.
  glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);

  // Check for division by zero. If w is 0, the point is at infinity or invalid.
  if (clipPos.w == 0.0f) {
    return glm::vec2(-1.0f, -1.0f);  // Should not happen with valid inputs.
  }

  // Perform perspective division to get Normalized Device Coordinates (NDC).
  // NDC range from -1 to 1 for x and y.
  glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

  // Convert NDC to screen coordinates.
  // Screen X: scales NDC x from [-1, 1] to [0, windowWidth]
  float screenX = (ndcPos.x + 1.0f) / 2.0f * windowWidth;
  // Screen Y: scales NDC y from [-1, 1] to [0, windowHeight], flipping Y so 0
  // is top.
  float screenY = (1.0f - ndcPos.y) / 2.0f * windowHeight;
  return glm::vec2(screenX, screenY);
}

/**
 * @brief Unprojects a 2D screen-space point (with a given depth) into a 3D
 * world-space point.
 *
 * This function performs the reverse of WorldToScreen. It takes a 2D point on
 * the screen, along with a depth value (typically from the depth buffer or a
 * known plane), and converts it back into its corresponding 3D world
 * coordinate. This is useful for tasks like picking objects in a 3D scene based
 * on a mouse click.
 *
 * @param screenPos The 2D point on the screen (pixel coordinates).
 * @param ndcZ The Normalized Device Coordinate Z-value (depth) for the point.
 *             This usually ranges from 0.0 (near plane) to 1.0 (far plane).
 * @param invViewProj The inverse of the combined View and Projection matrix.
 * @param windowWidth The width of the application window in pixels.
 * @param windowHeight The height of the application window in pixels.
 * @return A glm::vec3 representing the 3D world coordinates of the unprojected
 * point. Returns glm::vec3(0.0f) if the W component becomes zero, indicating an
 * issue.
 */
inline glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                                    const glm::mat4& invViewProj,
                                    int windowWidth, int windowHeight) {
  // Convert screen coordinates to Normalized Device Coordinates (NDC).
  // NDC X: scales screen X from [0, windowWidth] to [-1, 1].
  float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
  // NDC Y: scales screen Y from [0, windowHeight] to [-1, 1], flipping Y.
  float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f;

  // Create a 4D vector in NDC space. The W component is 1.0 for a point.
  glm::vec4 ndcPoint = glm::vec4(ndcX, ndcY, ndcZ, 1.0f);

  // Transform the NDC point back into world space using the inverse
  // view-projection matrix.
  glm::vec4 worldPos = invViewProj * ndcPoint;

  // Perform perspective division (undo the division done in WorldToScreen).
  // If worldPos.w is 0, the point is at infinity or invalid.
  if (worldPos.w == 0.0f) {
    return glm::vec3(0.0f);  // Should not happen with valid inputs.
  }
  return glm::vec3(worldPos) / worldPos.w;
}

/**
 * @brief Creates a normalized direction vector (a ray) from the camera's
 * position through a point on the screen.
 *
 * This function is crucial for "ray casting" or "picking" in 3D applications.
 * It generates a vector that originates from the camera's eye position and
 * points into the 3D scene through the specified 2D screen coordinate.
 * You can then use this ray to intersect with objects in your scene to
 * determine what the user is "pointing" at.
 *
 * @param screenPos The 2D point on the screen (pixel coordinates) to cast the
 * ray through.
 * @param projectionMatrix The camera's projection matrix.
 * @param viewMatrix The camera's view matrix.
 * @param windowWidth The width of the application window in pixels.
 * @param windowHeight The height of the application window in pixels.
 * @return A normalized glm::vec3 representing the direction of the ray in world
 * space.
 */
inline glm::vec3 ScreenToWorldRay(const glm::vec2& screenPos,
                                  const glm::mat4& projectionMatrix,
                                  const glm::mat4& viewMatrix, int windowWidth,
                                  int windowHeight) {
  // Convert screen coordinates to Normalized Device Coordinates (NDC).
  // NDC X: scales screen X from [0, windowWidth] to [-1, 1].
  float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
  // NDC Y: scales screen Y from [0, windowHeight] to [-1, 1], flipping Y.
  float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f;

  // Create a 4D vector in clip space.
  // Z is -1.0 (on the near plane) and W is 1.0 for a ray direction.
  glm::vec4 rayClip(ndcX, ndcY, -1.0, 1.0);

  // Transform from clip space to eye space (camera's local space).
  // We use the inverse of the projection matrix.
  glm::mat4 invProj = glm::inverse(projectionMatrix);
  glm::vec4 rayEye = invProj * rayClip;

  // For a ray, the W component should be 0, as it represents a direction, not a
  // point. The Z component is set to -1.0 because in eye space, the camera
  // looks down the negative Z-axis.
  rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

  // Transform from eye space to world space.
  // We use the inverse of the view matrix.
  glm::mat4 invView = glm::inverse(viewMatrix);
  glm::vec3 rayWorld = glm::vec3(invView * rayEye);

  // Normalize the vector to get a unit direction vector.
  return glm::normalize(rayWorld);
}

}  // namespace MathHelpers
