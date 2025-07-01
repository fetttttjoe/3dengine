// =======================================================================
// File: src/Interfaces.h
// =======================================================================
#pragma once
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Scene;
class Shader;
class Camera;
class ISceneObject;

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Initialize(void* windowHandle) = 0;
    virtual void BeginFrame() = 0;
    virtual void RenderScene(const Scene& scene, const Camera& camera) = 0;
    virtual void RenderUI() = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
    virtual uint32_t ProcessPicking(int x, int y, const Scene& scene, const Camera& camera) = 0;
    virtual void RenderHighlight(const ISceneObject& object, const Camera& camera) = 0;
};

// Represents a configurable property of an object for the UI.
struct ObjectProperty {
    std::string name;
    float* value;
};

class ISceneObject {
public:
    ISceneObject() : id(0), transform(1.0f), name("Unnamed Object"), isSelected(false) {}
    virtual ~ISceneObject() = default;
    virtual void Draw(const glm::mat4& view, const glm::mat4& projection) = 0;
    virtual void DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) = 0;

    // NEW: Add const qualifier here
    virtual void DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const = 0; //

    virtual std::string GetTypeString() const = 0;

    virtual std::vector<ObjectProperty> GetProperties() { return {}; }
    virtual void RebuildMesh() {}

    virtual glm::vec3 GetPosition() const {
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, position, skew, perspective);
        return position;
    }
    virtual void SetPosition(const glm::vec3& newPos) {
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, position, skew, perspective);
        transform = glm::translate(glm::mat4(1.0f), newPos) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
    }

    uint32_t id;
    glm::mat4 transform;
    std::string name;
    bool isSelected;
};