#include "Scene/Objects/BaseObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Core/JsonGlmHelpers.h"
#include "Core/PropertyNames.h"
#include "Core/ResourceManager.h"
#include "Sculpting/SculptableMesh.h"
#include "Shader.h"

BaseObject::BaseObject() {
  m_SculptableMesh = std::make_unique<SculptableMesh>();
  m_Shader = ResourceManager::LoadShader("lit_shader", "shaders/lit.vert",
                                         "shaders/lit.frag");

  auto onTransformChanged = [this]() { m_IsTransformDirty = true; };
  auto onMeshChanged = [this]() { RebuildMesh(); };

  m_Properties.Add(PropertyNames::Position, glm::vec3(0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Rotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Scale, glm::vec3(1.0f), onTransformChanged);
  m_Properties.Add(PropertyNames::Color, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
  m_Properties.Add(PropertyNames::Width, 1.0f, onMeshChanged);
  m_Properties.Add(PropertyNames::Height, 1.0f, onMeshChanged);
  m_Properties.Add(PropertyNames::Depth, 1.0f, onMeshChanged);
  RecalculateTransformMatrix();
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
}

void BaseObject::Draw(const glm::mat4& view, const glm::mat4& projection) {}

void BaseObject::DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                                const glm::mat4& projection) {}

void BaseObject::DrawHighlight(const glm::mat4& view,
                               const glm::mat4& projection) const {}

void BaseObject::RecalculateTransformMatrix() {
  glm::mat4 transform_T = glm::translate(glm::mat4(1.0f), GetPosition());
  glm::mat4 transform_R = glm::toMat4(GetRotation());
  glm::mat4 transform_S = glm::scale(glm::mat4(1.0f), GetScale());
  glm::mat4 transform_C =
      glm::translate(glm::mat4(1.0f), -this->GetLocalCenter());
  m_TransformMatrix = transform_T * transform_R * transform_S * transform_C;
  m_IsTransformDirty = false;
}

const glm::mat4& BaseObject::GetTransform() const {
  if (m_IsTransformDirty) {
    const_cast<BaseObject*>(this)->RecalculateTransformMatrix();
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
  std::vector<GizmoHandleDef> defs;
  if (m_Properties.GetProperty(PropertyNames::Width)) {
    defs.push_back({PropertyNames::Width, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}});
  }
  if (m_Properties.GetProperty(PropertyNames::Height)) {
    defs.push_back({PropertyNames::Height, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}});
  }
  if (m_Properties.GetProperty(PropertyNames::Depth)) {
    if (m_Properties.GetValue<float>(PropertyNames::Depth) > 0.0f) {
      defs.push_back({PropertyNames::Depth, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}});
    }
  }
  return defs;
}

void BaseObject::OnGizmoUpdate(const std::string& propertyName, float delta,
                               const glm::vec3& axis) {
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

void ISceneObject::Serialize(nlohmann::json& outJson) const {
  outJson["type"] = GetTypeString();
  outJson["id"] = id;
  outJson["name"] = name;
  nlohmann::json propsJson;
  GetPropertySet().Serialize(propsJson);
  outJson["properties"] = propsJson;

  if (const_cast<ISceneObject*>(this)->GetSculptableMesh()) {
    const_cast<ISceneObject*>(this)->GetSculptableMesh()->Serialize(outJson);
  }
}

void ISceneObject::Deserialize(const nlohmann::json& inJson) {
  id = inJson.value("id", 1);
  name = inJson.value("name", "Object");
  if (inJson.contains("properties")) {
    GetPropertySet().Deserialize(inJson["properties"]);
  }

  RebuildMesh();

  if (GetSculptableMesh()) {
    GetSculptableMesh()->Deserialize(inJson);
    SetMeshDirty(true);
  }
}
