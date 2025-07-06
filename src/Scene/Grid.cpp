#include "Scene/Grid.h"

#include <glad/glad.h>

#include <cmath>
#include <vector>

#include "Core/ResourceManager.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Shader.h"

Grid::Grid(int size, int divisions) : m_Size(size), m_Divisions(divisions) {
  name = std::string(ObjectTypes::Grid);
  isSelectable = false;
  isStatic = true;
  Initialize();
  RebuildMesh();
}

Grid::~Grid() {
  if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
  if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
}

void Grid::Initialize() {
  m_Shader = ResourceManager::LoadShader("grid_shader", "shaders/default.vert",
                                         "shaders/default.frag");

  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);

  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Grid::SetConfiguration(int size, int divisions) {
  m_Size = size;
  m_Divisions = divisions;
  RebuildMesh();
}

void Grid::RebuildMesh() {
  std::vector<float> vertices;
  if (m_Divisions == 0) return;

  m_Spacing = (float)m_Size / m_Divisions;
  float halfSize = (float)m_Size / 2.0f;

  for (int i = 0; i <= m_Divisions; ++i) {
    float pos = -halfSize + i * m_Spacing;
    vertices.push_back(-halfSize);
    vertices.push_back(0.0f);
    vertices.push_back(pos);
    vertices.push_back(halfSize);
    vertices.push_back(0.0f);
    vertices.push_back(pos);
    vertices.push_back(pos);
    vertices.push_back(0.0f);
    vertices.push_back(-halfSize);
    vertices.push_back(pos);
    vertices.push_back(0.0f);
    vertices.push_back(halfSize);
  }
  m_VertexCount = static_cast<int>(vertices.size() / 3);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Grid::Draw(const glm::mat4& view, const glm::mat4& projection) {
  if (!m_Shader) return;

  m_Shader->Bind();
  m_Shader->SetUniformMat4f("u_Model", GetTransform());
  m_Shader->SetUniformMat4f("u_View", view);
  m_Shader->SetUniformMat4f("u_Projection", projection);
  m_Shader->SetUniform4f("u_Color", 0.3f, 0.3f, 0.3f, 1.0f);

  glBindVertexArray(m_VAO);
  glDrawArrays(GL_LINES, 0, m_VertexCount);
  glBindVertexArray(0);
}

glm::vec3 Grid::GetClosestGridPoint(const glm::vec3& worldPoint) const {
  if (m_Spacing == 0.0f) return worldPoint;
  float snappedX = std::round(worldPoint.x / m_Spacing) * m_Spacing;
  float snappedY = 0.0f;
  float snappedZ = std::round(worldPoint.z / m_Spacing) * m_Spacing;
  return glm::vec3(snappedX, snappedY, snappedZ);
}

// --- Stubbed ISceneObject Methods ---

void Grid::DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                          const glm::mat4& projection) {}
void Grid::DrawHighlight(const glm::mat4& view,
                         const glm::mat4& projection) const {}
std::string Grid::GetTypeString() const {
  return std::string(ObjectTypes::Grid);
}

const glm::mat4& Grid::GetTransform() const { return m_Transform; }
glm::vec3 Grid::GetPosition() const { return glm::vec3(0.0f); }
glm::quat Grid::GetRotation() const {
  return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}
glm::vec3 Grid::GetScale() const { return glm::vec3(1.0f); }
void Grid::SetPosition(const glm::vec3& position) {}
void Grid::SetRotation(const glm::quat& rotation) {}
void Grid::SetScale(const glm::vec3& scale) {}
void Grid::SetEulerAngles(const glm::vec3& eulerAngles) {}

std::vector<GizmoHandleDef> Grid::GetGizmoHandleDefs() { return {}; }
void Grid::OnGizmoUpdate(const std::string& propertyName, float delta,
                         const glm::vec3& axis) {}