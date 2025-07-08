#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h" // Includes PropertySet, ISceneObject, etc.
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "gtest/gtest.h" // Google Test framework

/**
 * @class MockSceneObject
 * @brief A minimal, non-rendering implementation of ISceneObject for testing.
 *
 * This mock class implements all pure virtual functions from ISceneObject
 * and IGizmoClient, allowing us to test scene logic (like adding, deleting,
 * and duplicating objects) without needing a complete rendering context.
 */
class MockSceneObject : public ISceneObject {
 public:
  explicit MockSceneObject(const std::string& type = "Mock") : m_Type(type) {}

  // --- ISceneObject Overrides ---
  std::string GetTypeString() const override { return m_Type; }
  void Draw(const glm::mat4&, const glm::mat4&) override {}
  void DrawForPicking(Shader&, const glm::mat4&, const glm::mat4&) override {}
  void DrawHighlight(const glm::mat4&, const glm::mat4&) const override {}
  void RebuildMesh() override {}

  PropertySet& GetPropertySet() override { return m_Properties; }
  const PropertySet& GetPropertySet() const override { return m_Properties; }

  // --- Transform Overrides ---
  // These now correctly match the return types in ISceneObject.
  const glm::mat4& GetTransform() const override { return m_Transform; }
  glm::vec3 GetPosition() const override { return m_Position; }
  glm::quat GetRotation() const override { return m_Rotation; }
  glm::vec3 GetScale() const override { return m_Scale; }

  void SetPosition(const glm::vec3& p) override { m_Position = p; }
  void SetRotation(const glm::quat& r) override { m_Rotation = r; }
  void SetScale(const glm::vec3& s) override { m_Scale = s; }
  void SetEulerAngles(const glm::vec3&) override {}

  // --- IGizmoClient Overrides ---
  // These must be implemented for the class to be concrete.
  std::vector<GizmoHandleDef> GetGizmoHandleDefs() override { return {}; }
  void OnGizmoUpdate(const std::string&, float, const glm::vec3&) override {}

 private:
  std::string m_Type;
  PropertySet m_Properties;
  glm::mat4 m_Transform{1.0f};
  glm::vec3 m_Position{0.0f};
  glm::quat m_Rotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 m_Scale{1.0f};
};

// Test fixture to set up a clean scene and factory for each test case
class SceneTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Register mock object types in the factory
    factory.Register(std::string(ObjectTypes::Triangle), []() {
      return std::make_unique<MockSceneObject>(
          std::string(ObjectTypes::Triangle));
    });
    factory.Register(std::string(ObjectTypes::Pyramid), []() {
      return std::make_unique<MockSceneObject>(
          std::string(ObjectTypes::Pyramid));
    });

    // Create the scene with the factory
    scene = std::make_unique<Scene>(&factory);
  }

  SceneObjectFactory factory;
  std::unique_ptr<Scene> scene;
};

// --- Factory Tests ---

TEST_F(SceneTest, FactoryCreatesKnownObject) {
  std::unique_ptr<ISceneObject> obj =
      factory.Create(std::string(ObjectTypes::Triangle));
  ASSERT_NE(obj, nullptr);
  EXPECT_EQ(obj->GetTypeString(), ObjectTypes::Triangle);
}

TEST_F(SceneTest, FactoryFailsOnUnknownObject) {
  std::unique_ptr<ISceneObject> obj = factory.Create("UnknownType");
  ASSERT_EQ(obj, nullptr);
}