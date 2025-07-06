#pragma once

#include "Interfaces.h"
#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader;

class BaseObject : public ISceneObject
{
public:
  BaseObject();
  ~BaseObject() override;

  // ISceneObject overrides
  void Draw(const glm::mat4 &view, const glm::mat4 &projection) override;
  void DrawForPicking(Shader &pickingShader, const glm::mat4 &view, const glm::mat4 &projection) override;
  void DrawHighlight(const glm::mat4 &view, const glm::mat4 &projection) const override;

  std::string GetTypeString() const override = 0; // Pure virtual, must be implemented by concrete classes
  const std::vector<ObjectProperty>& GetProperties() const override;
  void RebuildMesh() override;

protected:
  // For derived classes to define their geometry
  virtual void BuildMeshData(std::vector<float> &vertices, std::vector<unsigned int> &indices) = 0;
  
  // Helper to set up OpenGL buffers
  void SetupMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
  
  // Helper for derived classes to register their properties
  void AddProperty(const std::string& name, void* value_ptr, PropertyType type);

  // Rendering
  GLuint m_VAO = 0;
  GLuint m_VBO = 0;
  GLuint m_EBO = 0;
  GLsizei m_IndexCount = 0;
  std::shared_ptr<Shader> m_Shader;

  // Common properties
  std::vector<ObjectProperty> m_Properties;
  glm::vec4 m_Color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
  float m_Width = 1.0f;
  float m_Height = 1.0f;
  float m_Depth = 1.0f;
};