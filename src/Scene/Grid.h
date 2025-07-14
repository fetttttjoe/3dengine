#pragma once

#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces.h"

class Grid : public ISceneObject {
 public:
  Grid();
  ~Grid() override;

  // --- ISceneObject Overrides ---
  void Draw(class OpenGLRenderer& renderer, const glm::mat4& view,
            const glm::mat4& projection) override;
  void DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                      const glm::mat4& projection) override {}
  void DrawHighlight(const glm::mat4& view,
                     const glm::mat4& projection) const override {}
  std::string GetTypeString() const override;
  void RebuildMesh() override;
  PropertySet& GetPropertySet() override { return m_Properties; }
  const PropertySet& GetPropertySet() const override { return m_Properties; }
  const glm::mat4& GetTransform() const override { return m_Transform; }
  glm::vec3 GetPosition() const override { return glm::vec3(0.0f); }
  glm::quat GetRotation() const override {
    return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  }
  glm::vec3 GetScale() const override { return glm::vec3(1.0f); }
  void SetPosition(const glm::vec3& position) override {}
  void SetRotation(const glm::quat& rotation) override {}
  void SetScale(const glm::vec3& scale) override {}
  void SetEulerAngles(const glm::vec3& eulerAngles) override {}
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override { return {}; }
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override {}
  IEditableMesh* GetEditableMesh() override { return nullptr; }

  bool IsMeshDirty() const override { return m_IsDirty; }
  // CORRECT: Removed 'const' to match the base class declaration.
  void SetMeshDirty(bool dirty) override { m_IsDirty = dirty; }

  bool IsUserCreatable() const override { return false; }
  std::shared_ptr<Shader> GetShader() const override { return nullptr; }

  // --- Grid-specific methods ---
  void SetConfiguration(int size, int divisions);
  glm::vec3 GetClosestGridPoint(const glm::vec3& worldPoint) const;

  const std::vector<float>& GetVertices() const { return m_Vertices; }
  int GetVertexCount() const { return m_VertexCount; }

 private:
  int m_Size;
  int m_Divisions;
  float m_Spacing;

  std::vector<float> m_Vertices;
  int m_VertexCount = 0;
  mutable bool m_IsDirty = true;

  PropertySet m_Properties;
  glm::mat4 m_Transform{1.0f};
};