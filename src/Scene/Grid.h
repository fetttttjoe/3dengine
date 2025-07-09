#pragma once

#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces.h"

class Shader;

class Grid : public ISceneObject {
 public:
  Grid(int size = 40, int divisions = 40);
  ~Grid() override;

  // ISceneObject Overrides
  void Draw(const glm::mat4& view, const glm::mat4& projection) override;
  void DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                      const glm::mat4& projection) override;
  void DrawHighlight(const glm::mat4& view,
                     const glm::mat4& projection) const override;
  std::string GetTypeString() const override;
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

  // <<< MODIFIED: Implemented new pure virtual methods from ISceneObject
  SculptableMesh* GetSculptableMesh() override { return nullptr; }
  bool IsMeshDirty() const override { return false; }
  void SetMeshDirty(bool dirty) override {}

  // Grid-specific methods
  void SetConfiguration(int size, int divisions);
  glm::vec3 GetClosestGridPoint(const glm::vec3& worldPoint) const;

 private:
  void Initialize();

  // Rendering
  unsigned int m_VAO = 0, m_VBO = 0;
  std::shared_ptr<Shader> m_Shader;
  int m_VertexCount = 0;

  // Configuration
  int m_Size;
  int m_Divisions;
  float m_Spacing;

  PropertySet m_Properties;
  glm::mat4 m_Transform{1.0f};
};
