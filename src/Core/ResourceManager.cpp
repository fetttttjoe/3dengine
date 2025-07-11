#include "Core/ResourceManager.h"

#include "Core/Log.h"
#include "Shader.h"

// Initialize static variables
std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;

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
  
  // Create a unique key from the file paths to ensure one shader per file pair.
  std::string file_key = vShaderFile + "::" + fShaderFile;

  // Check if a shader from these files is already loaded.
  if (s_Shaders.count(file_key)) {
    // It exists, return the cached version.
    return s_Shaders[file_key];
  }

  // The shader does not exist yet, so we create it.
  try {
    auto shader = std::make_shared<Shader>(vShaderFile, fShaderFile);
    // Store it in the cache using the file path key.
    s_Shaders[file_key] = shader;
    Log::Debug("ResourceManager: Compiled and loaded shader '", name,
               "' from files: ", vShaderFile);
    return shader;
  } catch (const std::exception& e) {
    Log::Debug("!!! ResourceManager: FAILED to load shader '", name,
               "'. Reason: ", e.what());
    // Store nullptr to prevent trying again.
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