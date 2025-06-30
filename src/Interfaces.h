#pragma once
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

// Forward-declarations
class Scene;
class Shader;
class Camera;

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Initialize(void* windowHandle) = 0;
    virtual void BeginFrame() = 0;
    virtual void RenderScene(const Scene& scene, const Camera& camera) = 0;
    virtual void RenderUI() = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
};

class ISceneObject {
public:
    ISceneObject() : transform(1.0f), name("Unnamed Object") {}
    virtual ~ISceneObject() = default;
    virtual void Draw(const glm::mat4& view, const glm::mat4& projection) = 0;
    
    glm::mat4 transform;
    std::string name;
};