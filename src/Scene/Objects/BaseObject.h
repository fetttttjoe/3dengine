#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces.h"

class Shader;

class BaseObject : public ISceneObject {
 public:
  BaseObject();
  ~BaseObject() override;

  void Draw(const glm::mat4& view, const glm::mat4& projection) override;
  void DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                      const glm::mat4& projection) override;
  void DrawHighlight(const glm::mat4& view,
                     const glm::mat4& projection) const override;
  std::string GetTypeString() const override = 0;
  void RebuildMesh() override;
  PropertySet& GetPropertySet() override { return m_Properties; }
  const PropertySet& GetPropertySet() const override { return m_Properties; }
  const glm::mat4& GetTransform() const override;
  glm::vec3 GetPosition() const override;
  glm::quat GetRotation() const override;
  glm::vec3 GetScale() const override;
  void SetPosition(const glm::vec3& position) override;
  void SetRotation(const glm::quat& rotation) override;
  void SetScale(const glm::vec3& scale) override;
  void SetEulerAngles(const glm::vec3& eulerAngles) override;

  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override;
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override;

 protected:
  virtual void BuildMeshData(std::vector<float>& vertices,
                             std::vector<unsigned int>& indices) = 0;
  virtual glm::vec3 GetLocalCenter() const;
  void SetupMesh(const std::vector<float>& vertices,
                 const std::vector<unsigned int>& indices);

  GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
  GLsizei m_IndexCount = 0;
  std::shared_ptr<Shader> m_Shader;
  PropertySet m_Properties;
  mutable bool m_IsTransformDirty = true;

 private:
  void RecalculateTransformMatrix();
  mutable glm::mat4 m_TransformMatrix;
};