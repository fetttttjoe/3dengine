#include "Scene/Grid.h"

#include "Core/Application.h"
#include "Core/SettingsManager.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Objects/ObjectTypes.h"

Grid::Grid() {
  name = std::string(ObjectTypes::Grid);
  isSelectable = false;
  isStatic = true;
  SetConfiguration(SettingsManager::Get().gridSize,
                   SettingsManager::Get().gridDivisions);
}

Grid::~Grid() {}

void Grid::SetConfiguration(int size, int divisions) {
  m_Size = size;
  m_Divisions = divisions;
  RebuildMesh();
}

void Grid::RebuildMesh() {
  m_Vertices.clear();
  if (m_Divisions == 0) {
    m_VertexCount = 0;
    m_IsDirty = true;
    return;
  }

  m_Spacing = (float)m_Size / m_Divisions;
  float halfSize = (float)m_Size / 2.0f;

  for (int i = 0; i <= m_Divisions; ++i) {
    float pos = -halfSize + i * m_Spacing;
    m_Vertices.push_back(-halfSize);
    m_Vertices.push_back(0.0f);
    m_Vertices.push_back(pos);
    m_Vertices.push_back(halfSize);
    m_Vertices.push_back(0.0f);
    m_Vertices.push_back(pos);
    m_Vertices.push_back(pos);
    m_Vertices.push_back(0.0f);
    m_Vertices.push_back(-halfSize);
    m_Vertices.push_back(pos);
    m_Vertices.push_back(0.0f);
    m_Vertices.push_back(halfSize);
  }
  m_VertexCount = static_cast<int>(m_Vertices.size() / 3);
  m_IsDirty = true;
  Application::Get().RequestSceneRender();
}

void Grid::Draw(OpenGLRenderer& renderer, const glm::mat4& view,
                const glm::mat4& projection) {
  renderer.RenderGrid(*this, *Application::Get().GetCamera());
}

glm::vec3 Grid::GetClosestGridPoint(const glm::vec3& worldPoint) const {
  if (m_Spacing == 0.0f) return worldPoint;
  float snappedX = std::round(worldPoint.x / m_Spacing) * m_Spacing;
  float snappedY = 0.0f;
  float snappedZ = std::round(worldPoint.z / m_Spacing) * m_Spacing;
  return glm::vec3(snappedX, snappedY, snappedZ);
}

std::string Grid::GetTypeString() const {
  return std::string(ObjectTypes::Grid);
}