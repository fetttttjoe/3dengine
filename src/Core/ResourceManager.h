#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional> // For std::hash

// Include tinyobjloader here so we can define helpers for its types
#include "tiny_obj_loader.h"

// Forward-declare the Shader class
class Shader;

// --- Helpers for using tinyobj::index_t as a key in std::unordered_map ---
// The custom equality operator MUST be visible before std::unordered_map is used with the type.
// Placing it in the namespace of the type itself (tinyobj) is best practice.
namespace tinyobj {
    inline bool operator==(const index_t& a, const index_t& b) {
        return a.vertex_index == b.vertex_index &&
               a.normal_index == b.normal_index &&
               a.texcoord_index == b.texcoord_index;
    }
}

// The hash function specialization MUST be in the std namespace.
namespace std {
    template <>
    struct hash<tinyobj::index_t> {
        size_t operator()(const tinyobj::index_t& i) const {
            // Combine the hashes of the members to create a unique hash for the triplet.
            size_t h1 = hash<int>()(i.vertex_index);
            size_t h2 = hash<int>()(i.normal_index);
            size_t h3 = hash<int>()(i.texcoord_index);
            
            // A common way to combine hashes
            size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}
// --- End of Helpers ---


class ResourceManager {
 public:
  // Disallow instantiation
  ResourceManager() = delete;

  static void Initialize();
  static void Shutdown();

  // Shaders
  static std::shared_ptr<Shader> LoadShader(const std::string& name,
                                            const std::string& vShaderFile,
                                            const std::string& fShaderFile);
  static std::shared_ptr<Shader> LoadShaderFromMemory(
      const std::string& name, const char* vShaderSource,
      const char* fShaderSource);
  static std::shared_ptr<Shader> GetShader(const std::string& name);

  // New home for the mesh loading function
  static std::pair<std::vector<float>, std::vector<unsigned int>> LoadMesh(
      const std::string& filepath);

 private:
  static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
};
