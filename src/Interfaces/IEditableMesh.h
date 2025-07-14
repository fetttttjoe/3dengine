#pragma once
#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>
#include <utility> // For std::pair

// Forward-declare the custom hasher
struct PairHash;

class IEditableMesh {
 public:
  virtual ~IEditableMesh() = default;

  virtual const std::vector<glm::vec3>& GetVertices() const = 0;
  virtual const std::vector<unsigned int>& GetIndices() const = 0;
  virtual const std::vector<glm::vec3>& GetNormals() const = 0;

  virtual std::vector<glm::vec3>& GetVertices() = 0;
  virtual std::vector<unsigned int>& GetIndices() = 0;
  virtual std::vector<glm::vec3>& GetNormals() = 0;

  virtual void RecalculateNormals() = 0;

  virtual bool ExtrudeFaces(const std::unordered_set<uint32_t>& faceIndices, float distance) = 0;
  virtual bool WeldVertices(const std::unordered_set<uint32_t>& vertexIndices, const glm::vec3& weldPoint) = 0;
  virtual bool BevelEdges(const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& edges, float amount) = 0;
};
