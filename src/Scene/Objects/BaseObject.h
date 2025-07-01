// src/Scene/Objects/BaseObject.h
#pragma once

#include "Interfaces.h"
#include <memory>
#include <vector>
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

  std::string GetTypeString() const override { return m_TypeString; }
  glm::vec4 GetColor() const { return m_Color; }
  void SetColor(const glm::vec4 &c) { m_Color = c; }
  std::vector<ObjectProperty> GetProperties() override;
  void RebuildMesh() override;

  // --- NEW HELPER METHODS ---
  // You should add these to the ISceneObject interface as well
  glm::vec3 GetPosition() const override;
  void SetPosition(const glm::vec3& position) override;
  glm::vec3 GetEulerAngles() const override;


protected:
  virtual void BuildMeshData(std::vector<float> &vertices, std::vector<unsigned int> &indices) = 0;
  void SetupMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

  GLuint m_VAO = 0;
  GLuint m_VBO = 0;
  GLuint m_EBO = 0;
  GLsizei m_IndexCount = 0;

  glm::vec4 m_Color = glm::vec4(1.0f);

  float m_Width = 1.0f;
  float m_Height = 1.0f;
  float m_Depth = 1.0f;

  std::unique_ptr<Shader> m_Shader;
  std::string m_TypeString = "BaseObject";
};