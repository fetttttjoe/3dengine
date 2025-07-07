#pragma once

#include <memory>
#include <string>
#include <unordered_map>

// Forward-declare
class Shader;

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

 private:
  static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
};