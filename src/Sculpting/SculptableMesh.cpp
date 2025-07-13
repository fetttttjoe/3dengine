#include "Sculpting/SculptableMesh.h"

#include <numeric>
#include <algorithm> // Required for std::fill

#include "Core/JsonGlmHelpers.h"
#include "Core/Log.h"

void SculptableMesh::Initialize(const std::vector<float>& vertices,
                                const std::vector<unsigned int>& indices) {
  m_Vertices.clear();
  for (size_t i = 0; i < vertices.size(); i += 3) {
    m_Vertices.emplace_back(vertices[i], vertices[i + 1], vertices[i + 2]);
  }

  m_Indices = indices; // Indices are set here!

  m_Normals.resize(m_Vertices.size(), glm::vec3(0.0f));

  RecalculateNormals();
}

void SculptableMesh::RecalculateNormals() {
  if (m_Vertices.empty()) {
      m_Normals.clear();
      return;
  }

  if (m_Normals.size() != m_Vertices.size()) {
      m_Normals.resize(m_Vertices.size(), glm::vec3(0.0f));
  }

  if (m_Indices.empty()) {
      std::fill(m_Normals.begin(), m_Normals.end(), glm::vec3(0.0f));
      return;
  }

  std::fill(m_Normals.begin(), m_Normals.end(), glm::vec3(0.0f));

  for (size_t i = 0; i + 2 < m_Indices.size(); i += 3) {
    unsigned int i0 = m_Indices[i];
    unsigned int i1 = m_Indices[i + 1];
    unsigned int i2 = m_Indices[i + 2];

    if (i0 >= m_Vertices.size() || i1 >= m_Vertices.size() ||
        i2 >= m_Vertices.size()) {
        Log::Debug("SculptableMesh::RecalculateNormals: Skipping invalid triangle (index out of bounds): ", i0, ", ", i1, ", ", i2, " (Vertices size: ", m_Vertices.size(), ")");
        continue;
    }

    const glm::vec3& v0 = m_Vertices[i0];
    const glm::vec3& v1 = m_Vertices[i1];
    const glm::vec3& v2 = m_Vertices[i2];

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 faceNormal = glm::cross(edge1, edge2);

    m_Normals[i0] += faceNormal;
    m_Normals[i1] += faceNormal;
    m_Normals[i2] += faceNormal;
  }

  for (auto& normal : m_Normals) {
    if (glm::length(normal) > 0.0f) {
      normal = glm::normalize(normal);
    }
  }
}

void SculptableMesh::Serialize(nlohmann::json& outJson) const {
  outJson["sculpt_vertices"] = m_Vertices;
  outJson["sculpt_indices"] = m_Indices; // <--- ADDED: Serialize indices
}

void SculptableMesh::Deserialize(const nlohmann::json& inJson) {
  m_Vertices.clear();
  m_Indices.clear();
  m_Normals.clear();

  if (inJson.contains("sculpt_vertices")) {
    const auto& jsonVertices = inJson["sculpt_vertices"];
    m_Vertices.reserve(jsonVertices.size());
    for (const auto& jv : jsonVertices) {
      m_Vertices.push_back(jv.get<glm::vec3>());
    }
  }

  if (inJson.contains("sculpt_indices")) { 
    const auto& jsonIndices = inJson["sculpt_indices"];
    m_Indices.reserve(jsonIndices.size());
    for (const auto& ji : jsonIndices) {
      m_Indices.push_back(ji.get<unsigned int>());
    }
  }
  
  m_Normals.resize(m_Vertices.size(), glm::vec3(0.0f));
  RecalculateNormals();
}