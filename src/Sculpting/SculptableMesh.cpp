#include "Sculpting/SculptableMesh.h"

#include <algorithm> // For std::min_element
#include <map>
#include <numeric>

#include "Core/JsonGlmHelpers.h"
#include "Core/Log.h"

void SculptableMesh::Initialize(const std::vector<float>& vertices,
                                const std::vector<unsigned int>& indices) {
  m_Vertices.clear();
  for (size_t i = 0; i < vertices.size(); i += 3) {
    m_Vertices.emplace_back(vertices[i], vertices[i + 1], vertices[i + 2]);
  }

  m_Indices = indices;

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
      Log::Debug(
          "SculptableMesh::RecalculateNormals: Skipping invalid triangle "
          "(index out of bounds): ",
          i0, ", ", i1, ", ", i2, " (Vertices size: ", m_Vertices.size(), ")");
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
  outJson["sculpt_indices"] = m_Indices;
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

bool SculptableMesh::ExtrudeFaces(
    const std::unordered_set<uint32_t>& faceIndices, float distance) {
  if (faceIndices.empty()) return false;

  std::map<uint32_t, uint32_t> oldToNewVertexMap;
  glm::vec3 averageNormal(0.0f);

  for (uint32_t faceIndex : faceIndices) {
    uint32_t i0 = m_Indices[faceIndex * 3 + 0];
    uint32_t i1 = m_Indices[faceIndex * 3 + 1];
    uint32_t i2 = m_Indices[faceIndex * 3 + 2];
    averageNormal +=
        glm::normalize(m_Normals[i0] + m_Normals[i1] + m_Normals[i2]);
  }
  averageNormal = glm::normalize(averageNormal);

  for (uint32_t faceIndex : faceIndices) {
    for (int i = 0; i < 3; ++i) {
      uint32_t oldIndex = m_Indices[faceIndex * 3 + i];
      if (oldToNewVertexMap.find(oldIndex) == oldToNewVertexMap.end()) {
        glm::vec3 newVertex = m_Vertices[oldIndex] + averageNormal * distance;
        m_Vertices.push_back(newVertex);
        oldToNewVertexMap[oldIndex] =
            static_cast<uint32_t>(m_Vertices.size() - 1);
      }
    }
  }

  std::vector<uint32_t> newFacesIndices;
  std::vector<uint32_t> originalFaceIndices;

  for (uint32_t faceIndex : faceIndices) {
    uint32_t i0 = m_Indices[faceIndex * 3 + 0];
    uint32_t i1 = m_Indices[faceIndex * 3 + 1];
    uint32_t i2 = m_Indices[faceIndex * 3 + 2];

    // Replace the original face with the new extruded face
    m_Indices[faceIndex * 3 + 0] = oldToNewVertexMap[i0];
    m_Indices[faceIndex * 3 + 1] = oldToNewVertexMap[i1];
    m_Indices[faceIndex * 3 + 2] = oldToNewVertexMap[i2];

    // Stitch the sides
    for (int i = 0; i < 3; ++i) {
      uint32_t p1_old = m_Indices[faceIndex * 3 + i];
      uint32_t p2_old = m_Indices[faceIndex * 3 + ((i + 1) % 3)];

      uint32_t p1_new = oldToNewVertexMap.at(i0);
      uint32_t p2_new = oldToNewVertexMap.at(i1);
      if (i == 1) {
        p1_new = oldToNewVertexMap.at(i1);
        p2_new = oldToNewVertexMap.at(i2);
      }
      if (i == 2) {
        p1_new = oldToNewVertexMap.at(i2);
        p2_new = oldToNewVertexMap.at(i0);
      }

      newFacesIndices.push_back(p1_old);
      newFacesIndices.push_back(p2_new);
      newFacesIndices.push_back(p1_new);

      newFacesIndices.push_back(p1_old);
      newFacesIndices.push_back(p2_old);
      newFacesIndices.push_back(p2_new);
    }
  }

  m_Indices.insert(m_Indices.end(), newFacesIndices.begin(),
                   newFacesIndices.end());
  RecalculateNormals();
  return true;
}

bool SculptableMesh::WeldVertices(
    const std::unordered_set<uint32_t>& vertexIndices,
    const glm::vec3& weldPoint) {
  if (vertexIndices.size() < 2) return false;

  // Make target vertex selection deterministic by choosing the smallest index
  uint32_t targetVertexIndex = *std::min_element(vertexIndices.begin(), vertexIndices.end());
  m_Vertices[targetVertexIndex] = weldPoint;

  std::unordered_set<uint32_t> verticesToRemap = vertexIndices;
  verticesToRemap.erase(targetVertexIndex); // Keep target vertex out of remapping set

  for (size_t i = 0; i < m_Indices.size(); ++i) {
    if (verticesToRemap.count(m_Indices[i])) { // Check if the index needs to be remapped
      m_Indices[i] = targetVertexIndex;
    }
  }

  RecalculateNormals();
  return true;
}

bool SculptableMesh::BevelEdges(const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& edges, float amount) {
    if (edges.empty()) return false;

    std::vector<uint32_t> newIndices;
    std::map<uint32_t, uint32_t> oldToNewVertexMap;

    for (const auto& edge : edges) {
        uint32_t v0_idx = edge.first;
        uint32_t v1_idx = edge.second;

        // For each vertex on the edge, create a new beveled vertex if one doesn't exist yet
        if (oldToNewVertexMap.find(v0_idx) == oldToNewVertexMap.end()) {
            glm::vec3 offset = m_Normals[v0_idx] * amount;
            m_Vertices.push_back(m_Vertices[v0_idx] + offset);
            oldToNewVertexMap[v0_idx] = m_Vertices.size() - 1;
        }
        if (oldToNewVertexMap.find(v1_idx) == oldToNewVertexMap.end()) {
            glm::vec3 offset = m_Normals[v1_idx] * amount;
            m_Vertices.push_back(m_Vertices[v1_idx] + offset);
            oldToNewVertexMap[v1_idx] = m_Vertices.size() - 1;
        }

        uint32_t v0_new_idx = oldToNewVertexMap[v0_idx];
        uint32_t v1_new_idx = oldToNewVertexMap[v1_idx];

        // Create two new triangles to form the beveled face
        newIndices.push_back(v0_idx);
        newIndices.push_back(v1_idx);
        newIndices.push_back(v1_new_idx);

        newIndices.push_back(v0_idx);
        newIndices.push_back(v1_new_idx);
        newIndices.push_back(v0_new_idx);
    }

    // A simple implementation just adds new faces. A more complex one would replace existing ones.
    m_Indices.insert(m_Indices.end(), newIndices.begin(), newIndices.end());
    RecalculateNormals();
    return true;
}