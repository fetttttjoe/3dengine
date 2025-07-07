#include "TransformGizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <variant>

#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/ResourceManager.h"
#include "Shader.h"

const char* GIZMO_VERTEX_SHADER_SRC = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
void main() { gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0); }
)glsl";

const char* GIZMO_FRAGMENT_SHADER_SRC = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 u_Color;
void main() { FragColor = u_Color; }
)glsl";

TransformGizmo::TransformGizmo() : m_Target(nullptr), m_ActiveHandle(nullptr) {
  m_Shader = ResourceManager::LoadShaderFromMemory(
      "gizmo_shader", GIZMO_VERTEX_SHADER_SRC, GIZMO_FRAGMENT_SHADER_SRC);
  InitializeRendererObjects();
}

TransformGizmo::~TransformGizmo() {
  glDeleteBuffers(1, &m_VBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void TransformGizmo::InitializeRendererObjects() {
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f,
                      0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f};
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};
  m_IndexCount = 6;
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &ebo);
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
  glm::vec2 screenPosStart =
      camera.WorldToScreen(objectWorldPos, winWidth, winHeight);
  glm::vec2 screenPosEnd =
      camera.WorldToScreen(objectWorldPos + axisWorldDir, winWidth, winHeight);
  glm::vec2 screenAxis = screenPosEnd - screenPosStart;

  if (glm::length(screenAxis) < 0.001f) return;
  screenAxis = glm::normalize(screenAxis);

  float dot_product = glm::dot(mouseDelta, screenAxis);
  float sensitivity = 0.01f;
  float change =
      dot_product * sensitivity * m_ActiveHandle->directionMultiplier;

  // FIX: Pass the handle's local direction as the axis of interaction
  m_Target->OnGizmoUpdate(m_ActiveHandle->propertyName, change,
                          m_ActiveHandle->localDirection);
}

void TransformGizmo::Draw(const Camera& camera) {
  if (!m_Target || m_Handles.empty()) return;
  m_Shader->Bind();
  m_Shader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_Shader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
  float viz_scale = distance * 0.02f;
  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(m_VAO);
  for (const auto& handle : m_Handles) {
    m_Shader->SetUniformVec4("u_Color", handle.color);
    glm::vec3 handlePosLocal(0.0f);
    const auto& propSet = m_Target->GetPropertySet();
    if (handle.propertyName == PropertyNames::Width &&
        propSet.GetProperty(PropertyNames::Width)) {
      handlePosLocal.x =
          (propSet.GetValue<float>(PropertyNames::Width) / 2.0f) *
          handle.directionMultiplier;
    } else if (handle.propertyName == PropertyNames::Height &&
               propSet.GetProperty(PropertyNames::Height)) {
      float h = propSet.GetValue<float>(PropertyNames::Height);
      handlePosLocal.y = (handle.directionMultiplier > 0) ? h : 0.0f;
    } else if (handle.propertyName == PropertyNames::Depth &&
               propSet.GetProperty(PropertyNames::Depth)) {
      handlePosLocal.z =
          (propSet.GetValue<float>(PropertyNames::Depth) / 2.0f) *
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
    handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(viz_scale));
    m_Shader->SetUniformMat4f("u_Model", handleModelMatrix);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);
}

void TransformGizmo::DrawForPicking(const Camera& camera,
                                    Shader& pickingShader) {
  if (!m_Target || m_Handles.empty()) return;
  pickingShader.Bind();
  pickingShader.SetUniformMat4f("u_View", camera.GetViewMatrix());
  pickingShader.SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
  float viz_scale = distance * 0.02f;
  glBindVertexArray(m_VAO);
  for (const auto& handle : m_Handles) {
    pickingShader.SetUniform1ui("u_ObjectID", handle.id);
    glm::vec3 handlePosLocal(0.0f);
    const auto& propSet = m_Target->GetPropertySet();
    if (handle.propertyName == PropertyNames::Width &&
        propSet.GetProperty(PropertyNames::Width)) {
      handlePosLocal.x =
          (propSet.GetValue<float>(PropertyNames::Width) / 2.0f) *
          handle.directionMultiplier;
    } else if (handle.propertyName == PropertyNames::Height &&
               propSet.GetProperty(PropertyNames::Height)) {
      float h = propSet.GetValue<float>(PropertyNames::Height);
      handlePosLocal.y = (handle.directionMultiplier > 0) ? h : 0.0f;
    } else if (handle.propertyName == PropertyNames::Depth &&
               propSet.GetProperty(PropertyNames::Depth)) {
      handlePosLocal.z =
          (propSet.GetValue<float>(PropertyNames::Depth) / 2.0f) *
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
    handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(viz_scale));
    pickingShader.SetUniformMat4f("u_Model", handleModelMatrix);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}