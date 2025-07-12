#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Interfaces.h"  // Now includes the new gizmo definitions

// Forward declarations
class Camera;
class Shader;
class ISceneObject;

// Represents a single draggable handle instance created from a GizmoHandleDef
struct GizmoHandle {
  uint32_t id;
  std::string propertyName;
  glm::vec3 localDirection;
  glm::vec4 color;
  float directionMultiplier;  // +1 or -1
};

// Manages all handles for a selected object
class TransformGizmo {
 public:
  TransformGizmo();
  ~TransformGizmo();

  void SetTarget(ISceneObject* target);
  ISceneObject* GetTarget() const { return m_Target; }

  void Update(const Camera& camera, const glm::vec2& mouseDelta,
              bool isDragging, int winWidth, int winHeight);

  void Draw(const Camera& camera);
  void DrawForPicking(const Camera& camera, Shader& pickingShader);

  void SetActiveHandle(uint32_t id);
  GizmoHandle* GetActiveHandle() { return m_ActiveHandle; }

  // This function is now public so it can be used by the unit tests.
  GizmoHandle* GetHandleByID(uint32_t id);

  static bool IsGizmoID(uint32_t id) { return id >= GIZMO_ID_START; }

 private:
  void CreateHandles();
  void InitializeRendererObjects();
  // GetHandleByID has been moved from here to the public section.

  ISceneObject* m_Target;
  std::vector<GizmoHandle> m_Handles;
  GizmoHandle* m_ActiveHandle;
  // Rendering resources
  std::shared_ptr<Shader> m_Shader;
  GLuint m_VAO = 0;
  GLuint m_VBO = 0;
  GLsizei m_IndexCount = 0;

  // A constant to offset gizmo IDs from object IDs
  static const uint32_t GIZMO_ID_START = 1000000;
};