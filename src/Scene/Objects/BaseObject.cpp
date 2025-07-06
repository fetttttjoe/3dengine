#include "Scene/Objects/BaseObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Core/PropertyNames.h"
#include "Core/ResourceManager.h"
#include "Shader.h"

// Helper functions for JSON serialization of GLM types
namespace glm {
void to_json(nlohmann::json& j, const glm::vec3& v) { j = {v.x, v.y, v.z}; }
void from_json(const nlohmann::json& j, glm::vec3& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
  j.at(2).get_to(v.z);
}
void to_json(nlohmann::json& j, const glm::vec4& v) {
  j = {v.x, v.y, v.z, v.w};
}
void from_json(const nlohmann::json& j, glm::vec4& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
  j.at(2).get_to(v.z);
  j.at(3).get_to(v.w);
}
void to_json(nlohmann::json& j, const glm::quat& q) {
  j = {q.w, q.x, q.y, q.z};
}
void from_json(const nlohmann::json& j, glm::quat& q) {
  j.at(0).get_to(q.w);
  j.at(1).get_to(q.x);
  j.at(2).get_to(q.y);
  j.at(3).get_to(q.z);
}
}  // namespace glm

BaseObject::BaseObject() {
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);
  m_Shader = ResourceManager::LoadShader("default", "shaders/default.vert",
                                         "shaders/default.frag");
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

BaseObject::~BaseObject() {
  if (m_EBO) glDeleteBuffers(1, &m_EBO);
  if (m_VBO) glDeleteBuffers(1, &m_VBO);
  if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void BaseObject::SetupMesh(const std::vector<float>& vertices,
                           const std::vector<unsigned int>& indices) {
  m_IndexCount = static_cast<GLsizei>(indices.size());
  if (m_IndexCount == 0) return;
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void BaseObject::RebuildMesh() {
  std::vector<float> verts;
  std::vector<unsigned int> inds;
  BuildMeshData(verts, inds);
  SetupMesh(verts, inds);
  m_IsTransformDirty = true;
}

void BaseObject::Draw(const glm::mat4& view, const glm::mat4& projection) {
  if (!m_Shader) return;
  m_Shader->Bind();
  m_Shader->SetUniformMat4f("u_Model", GetTransform());
  m_Shader->SetUniformMat4f("u_View", view);
  m_Shader->SetUniformMat4f("u_Projection", projection);
  m_Shader->SetUniformVec4(
      "u_Color", m_Properties.GetValue<glm::vec4>(PropertyNames::Color));
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void BaseObject::DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                                const glm::mat4& projection) {
  pickingShader.Bind();
  pickingShader.SetUniformMat4f("u_Model", GetTransform());
  pickingShader.SetUniformMat4f("u_View", view);
  pickingShader.SetUniformMat4f("u_Projection", projection);
  pickingShader.SetUniform1ui("u_ObjectID", id);
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void BaseObject::DrawHighlight(const glm::mat4& view,
                               const glm::mat4& projection) const {
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

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
}

void ISceneObject::Deserialize(const nlohmann::json& inJson) {
  id = inJson.value("id", 1);
  name = inJson.value("name", "Object");
  if (inJson.contains("properties")) {
    GetPropertySet().Deserialize(inJson["properties"]);
  }
  RebuildMesh();
}