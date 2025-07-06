#include "ResourceManager.h"
#include "Shader.h"
#include "Core/Log.h"

// Initialize static variables
std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;

void ResourceManager::Initialize() {
    Log::Debug("ResourceManager Initialized.");
}

void ResourceManager::Shutdown() {
    s_Shaders.clear();
    Log::Debug("ResourceManager Shutdown.");
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, const std::string& vShaderFile, const std::string& fShaderFile) {
    if (s_Shaders.find(name) == s_Shaders.end()) {
        auto shader = std::make_shared<Shader>(vShaderFile, fShaderFile);
        s_Shaders[name] = shader;
        Log::Debug("ResourceManager: Compiled and loaded shader '", name, "' from file.");
    }
    return s_Shaders[name];
}

std::shared_ptr<Shader> ResourceManager::LoadShaderFromMemory(const std::string& name, const char* vShaderSource, const char* fShaderSource) {
    if (s_Shaders.find(name) == s_Shaders.end()) {
        auto shader = std::make_shared<Shader>(vShaderSource, fShaderSource, true);
        s_Shaders[name] = shader;
        Log::Debug("ResourceManager: Compiled and loaded shader '", name, "' from memory.");
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