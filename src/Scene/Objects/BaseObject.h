// Scene/Objects/BaseObject.h
#pragma once

#include "Interfaces.h" // ↳ defines ISceneObject, ObjectProperty
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
  void Draw(const glm::mat4 &view,
            const glm::mat4 &projection) override;
  void DrawForPicking(Shader &pickingShader,
                      const glm::mat4 &view,
                      const glm::mat4 &projection) override;
  void DrawHighlight(const glm::mat4 &view,
                     const glm::mat4 &projection) const override;

  std::string GetTypeString() const override { return m_TypeString; }
  glm::vec4   GetColor() const               { return m_Color; }
  void        SetColor(const glm::vec4 &c)   { m_Color = c; }
  std::vector<ObjectProperty> GetProperties() override;
  void        RebuildMesh() override;

protected:
  // mesh‐builder hook
  virtual void BuildMeshData(std::vector<float> &vertices,
                             std::vector<unsigned int> &indices) = 0;

  // uploads buffers & records index count
  void        SetupMesh(const std::vector<float> &vertices,
                        const std::vector<unsigned int> &indices);

  // GL handles
  GLuint      m_VAO   = 0;
  GLuint      m_VBO   = 0;
  GLuint      m_EBO   = 0;
  GLsizei     m_IndexCount = 0;

  // simple material
  glm::vec4   m_Color    = glm::vec4(1.0f);

  // mesh dimensions
  float       m_Width    = 1.0f;
  float       m_Height   = 1.0f;
  float       m_Depth    = 1.0f;

  // convenience
  std::unique_ptr<Shader> m_Shader;
  std::string m_TypeString = "BaseObject";
};
