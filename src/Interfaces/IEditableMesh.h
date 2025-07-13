#pragma once

#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>

/**
 * @brief Interface for a mesh that can be edited at the sub-object level
 * (vertices, faces). This follows the Interface Segregation Principle by
 * separating mesh editing from the main ISceneObject interface.
 */
class IEditableMesh {
 public:
  virtual ~IEditableMesh() = default;

  // Read-only accessors
  virtual const std::vector<glm::vec3>& GetVertices() const = 0;
  virtual const std::vector<unsigned int>& GetIndices() const = 0;
  virtual const std::vector<glm::vec3>& GetNormals() const = 0;

  // Writable accessors for modification
  virtual std::vector<glm::vec3>& GetVertices() = 0;
  virtual std::vector<unsigned int>& GetIndices() = 0;
  virtual std::vector<glm::vec3>& GetNormals() = 0;

  // Core mesh operations
  virtual void RecalculateNormals() = 0;

  // Advanced editing operations
  virtual bool ExtrudeFaces(const std::unordered_set<uint32_t>& faceIndices,
                            float distance) = 0;
  virtual bool WeldVertices(const std::unordered_set<uint32_t>& vertexIndices,
                            const glm::vec3& weldPoint) = 0;
};