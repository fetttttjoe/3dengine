#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <vector>

#include "Interfaces/IEditableMesh.h"
#include "Sculpting/SubObjectSelection.h" // For PairHash

// Correctly inherit from the IEditableMesh interface
class SculptableMesh : public IEditableMesh {
 public:
  SculptableMesh() = default;
  ~SculptableMesh() override = default;

  // Rule of five is still fine
  SculptableMesh(const SculptableMesh&) = default;
  SculptableMesh& operator=(const SculptableMesh&) = default;
  SculptableMesh(SculptableMesh&&) = default;
  SculptableMesh& operator=(SculptableMesh&&) = default;

  void Initialize(const std::vector<float>& vertices,
                  const std::vector<unsigned int>& indices);

  // --- IEditableMesh Interface Implementation ---
  void RecalculateNormals() override;

  const std::vector<glm::vec3>& GetVertices() const override {
    return m_Vertices;
  }
  const std::vector<unsigned int>& GetIndices() const override {
    return m_Indices;
  }
  const std::vector<glm::vec3>& GetNormals() const override {
    return m_Normals;
  }

  std::vector<glm::vec3>& GetVertices() override { return m_Vertices; }
  std::vector<unsigned int>& GetIndices() override { return m_Indices; }
  std::vector<glm::vec3>& GetNormals() override { return m_Normals; }

  bool ExtrudeFaces(const std::unordered_set<uint32_t>& faceIndices,
                    float distance) override;
  bool WeldVertices(const std::unordered_set<uint32_t>& vertexIndices,
                    const glm::vec3& weldPoint) override;

  bool BevelEdges(const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& edges, float amount) override;

  // --- Serialization ---
  void Serialize(nlohmann::json& outJson) const;
  void Deserialize(const nlohmann::json& inJson);

 private:
  std::vector<glm::vec3> m_Vertices;
  std::vector<glm::vec3> m_Normals;
  std::vector<unsigned int> m_Indices;
};
