#include "Core/ResourceManager.h"

#include <glm/glm.hpp>
#include <unordered_map>

#include "Core/Log.h"
#include "Shader.h"
#include "tiny_obj_loader.h"

// Define the hash function for glm::vec3 so the map can use it
struct Vec3Hash {
  std::size_t operator()(const glm::vec3& v) const {
    auto h1 = std::hash<float>()(v.x);
    auto h2 = std::hash<float>()(v.y);
    auto h3 = std::hash<float>()(v.z);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

// Initialize static variables
std::unordered_map<std::string, std::shared_ptr<Shader>>
    ResourceManager::s_Shaders;

void ResourceManager::Initialize() {
  Log::Debug("ResourceManager Initialized.");
}

void ResourceManager::Shutdown() {
  s_Shaders.clear();
  Log::Debug("ResourceManager Shutdown.");
}
std::shared_ptr<Shader> ResourceManager::LoadShader(
    const std::string& name, const std::string& vShaderFile,
    const std::string& fShaderFile) {
  std::string file_key = vShaderFile + "::" + fShaderFile;

  if (s_Shaders.count(file_key)) {
    return s_Shaders[file_key];
  }

  try {
    auto shader = std::make_shared<Shader>(vShaderFile, fShaderFile);
    s_Shaders[file_key] = shader;
    Log::Debug("ResourceManager: Compiled and loaded shader '", name,
               "' from files: ", vShaderFile);
    return shader;
  } catch (const std::exception& e) {
    Log::Debug("!!! ResourceManager: FAILED to load shader '", name,
               "'. Reason: ", e.what());
    s_Shaders[file_key] = nullptr;
    return nullptr;
  }
}
std::shared_ptr<Shader> ResourceManager::LoadShaderFromMemory(
    const std::string& name, const char* vShaderSource,
    const char* fShaderSource) {
  if (s_Shaders.find(name) == s_Shaders.end()) {
    auto shader = std::make_shared<Shader>(vShaderSource, fShaderSource, true);
    s_Shaders[name] = shader;
    Log::Debug("ResourceManager: Compiled and loaded shader '", name,
               "' from memory.");
  }
  return s_Shaders[name];
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name) {
  if (s_Shaders.find(name) != s_Shaders.end()) {
    return s_Shaders[name];
  }
  Log::Debug("Error: Shader '", name, "' not found in ResourceManager.");
  return nullptr;
}

std::pair<std::vector<float>, std::vector<unsigned int>>
ResourceManager::LoadMesh(const std::string& filepath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filepath.c_str())) {
    Log::Debug("Failed to load/parse .obj file: ", filepath);
    Log::Debug("Warn: ", warn);
    Log::Debug("Err: ", err);
    return {};
  }

  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  std::unordered_map<glm::vec3, uint32_t, Vec3Hash> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      glm::vec3 vertex{};
      vertex.x = attrib.vertices[3 * index.vertex_index + 0];
      vertex.y = attrib.vertices[3 * index.vertex_index + 1];
      vertex.z = attrib.vertices[3 * index.vertex_index + 2];

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size() / 3);
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }
  return {vertices, indices};
}