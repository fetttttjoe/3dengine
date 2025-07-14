#include "Core/ResourceManager.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include "Core/Log.h"
#include "Shader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <functional> // For std::hash


// Initialize static variables
std::unordered_map<std::string, std::shared_ptr<Shader>>
    ResourceManager::s_Shaders;

void ResourceManager::Initialize() {
  // This can be empty or used for pre-loading common resources.
}

void ResourceManager::Shutdown() {
  s_Shaders.clear();
  Log::Debug("ResourceManager Shutdown.");
}
std::shared_ptr<Shader> ResourceManager::LoadShader(
    const std::string& name, const std::string& vShaderFile,
    const std::string& fShaderFile) {
  // CORRECTED: Use the provided 'name' as the key for caching.
  if (s_Shaders.count(name)) {
    return s_Shaders[name];
  }

  try {
    auto shader = std::make_shared<Shader>(vShaderFile, fShaderFile);
    s_Shaders[name] = shader; // Cache by name
    Log::Debug("ResourceManager: Compiled and loaded shader '", name,
               "' from files: ", vShaderFile);
    return shader;
  } catch (const std::exception& e) {
    Log::Debug("!!! ResourceManager: FAILED to load shader '", name,
               "'. Reason: ", e.what());
    s_Shaders[name] = nullptr; // Cache the failure to avoid re-trying
    throw; // Re-throw the exception so the caller knows it failed
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
  if (s_Shaders.count(name)) {
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
  // CORRECTED: Use a map to track unique vertices based on their full index triplet
  std::unordered_map<tinyobj::index_t, uint32_t> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      if (uniqueVertices.count(index) == 0) {
        // This is a new, unique vertex
        uniqueVertices[index] = static_cast<uint32_t>(vertices.size() / 3);
        
        // Position
        vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
        vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
        vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);
        
        // We could also add normals and texcoords here if our vertex format supported it
      }
      indices.push_back(uniqueVertices[index]);
    }
  }
  return {vertices, indices};
}
