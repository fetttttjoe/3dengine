#include "TransformGizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <variant>

#include "Core/Application.h"
#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/MathHelpers.h"
#include "Core/PropertyNames.h"
#include "Renderer/OpenGLRenderer.h"  // Include the renderer for the Draw call

TransformGizmo::TransformGizmo() : m_Target(nullptr), m_ActiveHandle(nullptr) {}

TransformGizmo::~TransformGizmo() {}

// The new Draw method delegates the actual rendering to the renderer.
void TransformGizmo::Draw(OpenGLRenderer& renderer, const Camera& camera) {
  renderer.RenderGizmo(*this, camera);
}

void TransformGizmo::SetTarget(ISceneObject* target) {
  m_Target = target;
  m_Handles.clear();
  m_ActiveHandle = nullptr;
  if (m_Target) {
    CreateHandles();
  }
}

void TransformGizmo::CreateHandles() {
  uint32_t currentId = GIZMO_ID_START;
  auto defs = m_Target->GetGizmoHandleDefs();
  for (const auto& def : defs) {
    m_Handles.push_back(
        {currentId++, def.propertyName, def.localDirection, def.color, 1.0f});
    m_Handles.push_back(
        {currentId++, def.propertyName, def.localDirection, def.color, -1.0f});
  }
}

GizmoHandle* TransformGizmo::GetHandleByID(uint32_t id) {
  for (auto& handle : m_Handles) {
    if (handle.id == id) return &handle;
  }
  return nullptr;
}

void TransformGizmo::SetActiveHandle(uint32_t id) {
  m_ActiveHandle = GetHandleByID(id);
}

void TransformGizmo::Update(const Camera& camera, const glm::vec2& mouseDelta,
                            bool isDragging, int winWidth, int winHeight) {
  if (!m_Target || !m_ActiveHandle || !isDragging) return;

  glm::mat4 objectRotationMatrix = glm::toMat4(m_Target->GetRotation());
  glm::vec3 axisWorldDir = glm::normalize(glm::vec3(
      objectRotationMatrix * glm::vec4(m_ActiveHandle->localDirection, 0.0f)));
  glm::vec3 objectWorldPos = m_Target->GetPosition();
  glm::mat4 viewProj = camera.GetProjectionMatrix() * camera.GetViewMatrix();

  glm::vec2 screenPosStart =
      MathHelpers::WorldToScreen(objectWorldPos, viewProj, winWidth, winHeight);
  glm::vec2 screenPosEnd = MathHelpers::WorldToScreen(
      objectWorldPos + axisWorldDir, viewProj, winWidth, winHeight);
  glm::vec2 screenAxis = screenPosEnd - screenPosStart;

  if (glm::length(screenAxis) < 0.001f) return;
  screenAxis = glm::normalize(screenAxis);

  float dot_product = glm::dot(mouseDelta, screenAxis);
  float sensitivity = 0.01f;
  float change =
      dot_product * sensitivity * m_ActiveHandle->directionMultiplier;

  m_Target->OnGizmoUpdate(m_ActiveHandle->propertyName, change, axisWorldDir);
}

glm::mat4 TransformGizmo::CalculateHandleModelMatrix(const GizmoHandle& handle,
                                                     const Camera& camera,
                                                     float scale) const {
  glm::vec3 handlePosLocal(0.0f);
  const auto& propSet = m_Target->GetPropertySet();

  if (handle.propertyName == PropertyNames::Width &&
      propSet.GetProperty(PropertyNames::Width)) {
    handlePosLocal.x = (propSet.GetValue<float>(PropertyNames::Width) / 2.0f) *
                       handle.directionMultiplier;
  } else if (handle.propertyName == PropertyNames::Height &&
             propSet.GetProperty(PropertyNames::Height)) {
    float h = propSet.GetValue<float>(PropertyNames::Height);
    handlePosLocal.y = (handle.directionMultiplier > 0) ? h : 0.0f;
  } else if (handle.propertyName == PropertyNames::Depth &&
             propSet.GetProperty(PropertyNames::Depth)) {
    handlePosLocal.z = (propSet.GetValue<float>(PropertyNames::Depth) / 2.0f) *
                       handle.directionMultiplier;
  } else if (handle.propertyName == PropertyNames::Scale &&
             propSet.GetProperty(PropertyNames::Radius)) {
    float radius = propSet.GetValue<float>(PropertyNames::Radius);
    handlePosLocal =
        handle.localDirection * radius * handle.directionMultiplier;
  }

  glm::vec3 handleWorldPosition =
      glm::vec3(m_Target->GetTransform() * glm::vec4(handlePosLocal, 1.0f));

  glm::mat4 handleModelMatrix =
      glm::translate(glm::mat4(1.0f), handleWorldPosition);
  handleModelMatrix =
      handleModelMatrix *
      glm::mat4(glm::mat3(glm::inverse(camera.GetViewMatrix())));
  handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(scale));

  return handleModelMatrix;
}