#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <vector>

/**
 * @class SculptableMesh
 * @brief Holds the dynamic vertex data for a sculptable object.
 *
 * This class is a pure data container for the mesh data (vertices, normals,
 * indices) that can be dynamically modified by sculpting tools. It contains no
 * rendering API-specific code.
 */
class SculptableMesh {
 public:
  SculptableMesh() = default;
  ~SculptableMesh() = default;

  // Rule of five: Default behavior is fine for a pure data class.
  SculptableMesh(const SculptableMesh&) = default;
  SculptableMesh& operator=(const SculptableMesh&) = default;
  SculptableMesh(SculptableMesh&&) = default;
  SculptableMesh& operator=(SculptableMesh&&) = default;

  /**
   * @brief Initializes the mesh with vertex and index data.
   * @param vertices A vector of floats representing vertex positions (x,y,z).
   * @param indices A vector of unsigned integers for indexed drawing.
   */
  void Initialize(const std::vector<float>& vertices,
                  const std::vector<unsigned int>& indices);

  /**
   * @brief Recalculates the normal for each vertex based on face normals.
   * This should be called after modifying vertex positions.
   */
  void RecalculateNormals();

  // --- Getters ---
  size_t GetIndexCount() const { return m_Indices.size(); }
  const std::vector<glm::vec3>& GetVertices() const { return m_Vertices; }
  const std::vector<glm::vec3>& GetNormals() const { return m_Normals; }
  const std::vector<unsigned int>& GetIndices() const { return m_Indices; }

  // --- Serialization ---
  void Serialize(nlohmann::json& outJson) const;
  void Deserialize(const nlohmann::json& inJson);

  // Public for direct manipulation by tools
  std::vector<glm::vec3> m_Vertices;
  std::vector<glm::vec3> m_Normals;
  std::vector<unsigned int> m_Indices;
};
