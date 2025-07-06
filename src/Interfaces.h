// =======================================================================
// File: src/Interfaces.h
// =======================================================================
#pragma once
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Core/Log.h"

class Scene;
class Shader;
class Camera;
class ISceneObject;

class IRenderer {
 public:
  virtual ~IRenderer() = default;
  virtual bool Initialize(void *windowHandle) = 0;
  virtual void BeginFrame() = 0;
  virtual void RenderScene(const Scene &scene, const Camera &camera) = 0;
  virtual void RenderUI() = 0;
  virtual void EndFrame() = 0;
  virtual void Shutdown() = 0;
  virtual uint32_t ProcessPicking(int x, int y, const Scene &scene,
                                  const Camera &camera) = 0;
  virtual void RenderHighlight(const ISceneObject &object,
                               const Camera &camera) = 0;
};

// Represents the type of data a property holds
enum class PropertyType { Float, Vec3, Color_Vec4 };

// A flexible, data-driven structure for object properties
struct ObjectProperty {
  std::string name;
  void *value_ptr;
  PropertyType type;
};

class ISceneObject {
 public:
  ISceneObject()
      : id(0), transform(1.0f), name("Unnamed Object"), isSelected(false) {}
  virtual ~ISceneObject() = default;

  virtual void Draw(const glm::mat4 &view, const glm::mat4 &projection) = 0;
  virtual void DrawForPicking(Shader &pickingShader, const glm::mat4 &view,
                              const glm::mat4 &projection) = 0;
  virtual void DrawHighlight(const glm::mat4 &view,
                             const glm::mat4 &projection) const = 0;

  virtual std::string GetTypeString() const = 0;
  virtual const std::vector<ObjectProperty> &GetProperties() const = 0;
  virtual void RebuildMesh() {}

  virtual glm::vec3 GetPosition() const {
    glm::vec3 position, scale, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    return position;
  }
  virtual void SetPosition(const glm::vec3 &newPos) {
    glm::vec3 position, scale, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    transform = glm::translate(glm::mat4(1.0f), newPos) *
                glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
  }

  virtual glm::vec3 GetEulerAngles() const {
    glm::vec3 scale, skew, position;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    return glm::degrees(glm::eulerAngles(rotation));
  }

  virtual void SetEulerAngles(const glm::vec3 &newAngles) {
    glm::vec3 position, scale, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    glm::quat newRotation = glm::quat(glm::radians(newAngles));
    transform = glm::translate(glm::mat4(1.0f), position) *
                glm::mat4_cast(newRotation) *
                glm::scale(glm::mat4(1.0f), scale);
  }

  uint32_t id;
  glm::mat4 transform;
  std::string name;
  bool isSelected;
  bool isSelectable = true;
  bool isStatic = false;
};