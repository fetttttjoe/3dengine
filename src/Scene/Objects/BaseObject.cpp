#include "Scene/Objects/BaseObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>  // CORRECT: Added the missing header for glm::toMat4

#include "Core/Application.h"
#include "Core/JsonGlmHelpers.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/ResourceManager.h"
#include "Renderer/OpenGLRenderer.h"
#include "Shader.h"

BaseObject::BaseObject() {
  m_SculptableMesh = std::make_unique<SculptableMesh>();
  m_Shader = ResourceManager::LoadShader("lit_shader", "shaders/lit.vert",
                                         "shaders/lit.frag");

  auto onTransformChanged = [this]() {
    m_IsTransformDirty = true;
    Application::Get().RequestSceneRender();
  };

  auto onRenderStateChanged = [this]() {
    Application::Get().RequestSceneRender();
  };

  m_Properties.Add(PropertyNames::Position, glm::vec3(0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Rotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Scale, glm::vec3(1.0f), onTransformChanged);
  m_Properties.Add(PropertyNames::Color, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
                   onRenderStateChanged);
}

BaseObject::~BaseObject() = default;

void BaseObject::RebuildMesh() {
  std::vector<float> verts;
  std::vector<unsigned int> inds;
  BuildMeshData(verts, inds);

  if (m_SculptableMesh) {
    m_SculptableMesh->Initialize(verts, inds);
    m_IsMeshDirty = true;
  }

  m_IsTransformDirty = true;
  Application::Get().RequestSceneRender();
}

void BaseObject::Draw(OpenGLRenderer& renderer, const glm::mat4& view,
                      const glm::mat4& projection) {
  renderer.RenderObject(*this, *Application::Get().GetCamera());
}

void BaseObject::DrawHighlight(const glm::mat4& view,
                               const glm::mat4& projection) const {
  Application::Get().GetRenderer()->RenderObjectHighlight(
      *this, *Application::Get().GetCamera());
}

void BaseObject::DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                                const glm::mat4& projection) {
  Application::Get().GetRenderer()->RenderObjectForPicking(
      *this, pickingShader, *Application::Get().GetCamera());
}

void BaseObject::RecalculateTransformMatrix() const {
  glm::mat4 transform_T = glm::translate(glm::mat4(1.0f), GetPosition());
  glm::mat4 transform_R = glm::toMat4(GetRotation());  // This line now works
  glm::mat4 transform_S = glm::scale(glm::mat4(1.0f), GetScale());
  glm::mat4 transform_C =
      glm::translate(glm::mat4(1.0f), -this->GetLocalCenter());
  m_TransformMatrix = transform_T * transform_R * transform_S * transform_C;
  m_IsTransformDirty = false;
}

const glm::mat4& BaseObject::GetTransform() const {
  if (m_IsTransformDirty) {
    RecalculateTransformMatrix();
  }
  return m_TransformMatrix;
}

glm::vec3 BaseObject::GetLocalCenter() const { return glm::vec3(0.0f); }
glm::vec3 BaseObject::GetPosition() const {
  return m_Properties.GetValue<glm::vec3>(PropertyNames::Position);
}
glm::quat BaseObject::GetRotation() const {
  return m_Properties.GetValue<glm::quat>(PropertyNames::Rotation);
}
glm::vec3 BaseObject::GetScale() const {
  return m_Properties.GetValue<glm::vec3>(PropertyNames::Scale);
}
void BaseObject::SetPosition(const glm::vec3& position) {
  m_Properties.SetValue(PropertyNames::Position, position);
}
void BaseObject::SetRotation(const glm::quat& rotation) {
  m_Properties.SetValue(PropertyNames::Rotation, rotation);
}
void BaseObject::SetScale(const glm::vec3& scale) {
  m_Properties.SetValue(PropertyNames::Scale, scale);
}
void BaseObject::SetEulerAngles(const glm::vec3& eulerAngles) {
  m_Properties.SetValue(PropertyNames::Rotation,
                        glm::quat(glm::radians(eulerAngles)));
}

std::vector<GizmoHandleDef> BaseObject::GetGizmoHandleDefs() {
  return {{PropertyNames::Scale, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}},
          {PropertyNames::Scale, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}},
          {PropertyNames::Scale, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}}};
}

void BaseObject::OnGizmoUpdate(const std::string& propertyName, float delta,
                               const glm::vec3& axis) {
  if (propertyName == PropertyNames::Scale) {
    glm::vec3 currentScale =
        m_Properties.GetValue<glm::vec3>(PropertyNames::Scale);
    glm::vec3 scaleChange(std::abs(axis.x) > 0.5f ? delta : 0.0f,
                          std::abs(axis.y) > 0.5f ? delta : 0.0f,
                          std::abs(axis.z) > 0.5f ? delta : 0.0f);
    glm::vec3 newScale = currentScale + scaleChange;
    newScale = glm::max(newScale, glm::vec3(0.05f));
    m_Properties.SetValue<glm::vec3>(PropertyNames::Scale, newScale);
  } else {
    try {
      float currentValue = m_Properties.GetValue<float>(propertyName);
      float newValue = currentValue + delta;
      if (newValue < 0.05f) {
        newValue = 0.05f;
      }
      m_Properties.SetValue<float>(propertyName, newValue);
    } catch (const std::exception& e) {
      Log::Debug("Gizmo update failed: ", e.what());
    }
  }
}