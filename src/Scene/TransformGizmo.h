#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Interfaces.h"

class Camera;
class OpenGLRenderer;  // Forward-declare the renderer
class ISceneObject;

// Definition of a single draggable handle.
struct GizmoHandle {
  uint32_t id;
  std::string propertyName;
  glm::vec3 localDirection;
  glm::vec4 color;
  float directionMultiplier;  // +1 or -1 for opposing handles
};

// Manages the state and logic for the transform gizmo.
class TransformGizmo {
 public:
  TransformGizmo();
  ~TransformGizmo();

  // --- New Draw Method ---
  // This is the correct, object-oriented way to handle drawing.
  void Draw(OpenGLRenderer& renderer, const Camera& camera);

  // --- State Management ---
  void SetTarget(ISceneObject* target);
  ISceneObject* GetTarget() const { return m_Target; }
  const std::vector<GizmoHandle>& GetHandles() const { return m_Handles; }
  void SetActiveHandle(uint32_t id);
  GizmoHandle* GetActiveHandle() { return m_ActiveHandle; }

  // --- Logic ---
  void Update(const Camera& camera, const glm::vec2& mouseDelta,
              bool isDragging, int winWidth, int winHeight);
  glm::mat4 CalculateHandleModelMatrix(const GizmoHandle& handle,
                                       const Camera& camera, float scale) const;

  // --- Helpers ---
  GizmoHandle* GetHandleByID(uint32_t id);
  static bool IsGizmoID(uint32_t id) { return id >= GIZMO_ID_START; }

 private:
  void CreateHandles();

  ISceneObject* m_Target;
  std::vector<GizmoHandle> m_Handles;
  GizmoHandle* m_ActiveHandle;

  static const uint32_t GIZMO_ID_START = 1000000;
};