#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/ObjectTypes.h"
#include "Scene/Scene.h"
#include "gtest/gtest.h"  // Assuming use of Google Test framework

// A simple mock object for testing purposes. It allows us to test scene
// logic without needing a full rendering object.
class MockSceneObject : public ISceneObject {
 public:
  explicit MockSceneObject(const std::string& type = "Mock") : m_Type(type) {}

  std::string GetTypeString() const override { return m_Type; }
  void Draw(const glm::mat4& v, const glm::mat4& p) override {}
  void DrawForPicking(Shader& s, const glm::mat4& v,
                      const glm::mat4& p) override {}
  void DrawHighlight(const glm::mat4& v, const glm::mat4& p) const override {}
  const std::vector<ObjectProperty>& GetProperties() const override {
    static std::vector<ObjectProperty> p;
    return p;
  }
  // Implement pure virtual functions from the base interface
  const glm::vec3& GetPosition() const override { return m_Position; }
  const glm::quat& GetRotation() const override { return m_Rotation; }
  const glm::vec3& GetScale() const override { return m_Scale; }
  glm::vec3 GetEulerAngles() const override { return glm::vec3(0.0f); }
  const glm::mat4& GetTransform() const override { return m_Transform; }
  void SetPosition(const glm::vec3& p) override { m_Position = p; }
  void SetRotation(const glm::quat& r) override { m_Rotation = r; }
  void SetScale(const glm::vec3& s) override { m_Scale = s; }
  void SetEulerAngles(const glm::vec3& e) override {}

 private:
  std::string m_Type;
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
  // Act: Attempt to create a registered object type
  std::unique_ptr<ISceneObject> obj =
      factory.Create(std::string(ObjectTypes::Triangle));

  // Assert: The object should be created successfully
  ASSERT_NE(obj, nullptr);
  EXPECT_EQ(obj->GetTypeString(), ObjectTypes::Triangle);
}

TEST_F(SceneTest, FactoryFailsOnUnknownObject) {
  // Act: Attempt to create an unregistered object type
  std::unique_ptr<ISceneObject> obj = factory.Create("UnknownType");

  // Assert: The factory should return a null pointer
  ASSERT_EQ(obj, nullptr);
}

// --- Scene Duplication and Naming Tests ---

TEST_F(SceneTest, DuplicateObjectSmartNaming) {
  // Arrange: Add an object to the scene
  auto original = factory.Create(std::string(ObjectTypes::Triangle));
  original->name = std::string(ObjectTypes::Triangle);
  scene->AddObject(std::move(original));
  uint32_t originalId = scene->GetSceneObjects().front()->id;

  // Act: Duplicate the object we just added
  scene->DuplicateObject(originalId);

  // Assert: Check that a new object exists with the correct name
  const auto& objects = scene->GetSceneObjects();
  ASSERT_EQ(objects.size(), 2);
  EXPECT_EQ(objects[1]->name, "Triangle (1)");
}

TEST_F(SceneTest, DuplicateObjectWithCustomNameUsesTypeForNewName) {
  // Arrange: Add an object and give it a custom name
  auto original = factory.Create(std::string(ObjectTypes::Triangle));
  original->name = "My Favorite Triangle";
  scene->AddObject(std::move(original));
  uint32_t originalId = scene->GetSceneObjects().front()->id;

  // Act: Duplicate the object
  scene->DuplicateObject(originalId);

  // Assert: The new name should be based on the object's TYPE ("Triangle"), not
  // its custom name
  const auto& objects = scene->GetSceneObjects();
  ASSERT_EQ(objects.size(), 2);
  EXPECT_EQ(objects[1]->name, "Triangle (1)");
}

TEST_F(SceneTest, DuplicateNumberedObjectIncrementsCorrectly) {
  // Arrange: Add a base object and a numbered copy
  scene->AddObject(factory.Create(std::string(ObjectTypes::Triangle)));
  scene->AddObject(factory.Create(std::string(ObjectTypes::Triangle)));

  // Manually set names to simulate an existing scene
  scene->GetSceneObjects()[0]->name = "Triangle";
  scene->GetSceneObjects()[1]->name = "Triangle (1)";
  uint32_t idToDuplicate = scene->GetSceneObjects()[0]->id;

  // Act: Duplicate the original "Triangle"
  scene->DuplicateObject(idToDuplicate);

  // Assert: The new object should be the next available number
  const auto& objects = scene->GetSceneObjects();
  ASSERT_EQ(objects.size(), 3);
  EXPECT_EQ(objects[2]->name, "Triangle (2)");
}

TEST_F(SceneTest, GetNextAvailableIndexForNameWithGaps) {
  // Arrange: Create a scene with a gap in the naming sequence
  scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
  scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
  scene->GetSceneObjects()[0]->name = "Pyramid";
  scene->GetSceneObjects()[1]->name =
      "Pyramid (3)";  // A gap exists at (1) and (2)
  uint32_t idToDuplicate = scene->GetSceneObjects()[0]->id;

  // Act: Duplicate the original "Pyramid"
  scene->DuplicateObject(idToDuplicate);

  // Assert: The logic finds the max and adds 1, so it should produce "Pyramid
  // (4)".
  const auto& objects = scene->GetSceneObjects();
  ASSERT_EQ(objects.size(), 3);
  EXPECT_EQ(objects[2]->name, "Pyramid (4)");
}

// --- Deletion and Selection Tests ---

TEST_F(SceneTest, DeleteSelectedObjectClearsSelection) {
  // Arrange
  scene->AddObject(factory.Create(std::string(ObjectTypes::Triangle)));
  uint32_t id = scene->GetSceneObjects().front()->id;
  scene->SetSelectedObjectByID(id);
  ASSERT_NE(scene->GetSelectedObject(), nullptr);

  // Act
  scene->DeleteSelectedObject();

  // Assert
  EXPECT_EQ(scene->GetSceneObjects().size(), 0);
  EXPECT_EQ(scene->GetSelectedObject(), nullptr);
}

TEST_F(SceneTest, DeleteObjectByIDUpdatesSelectionIndex) {
  // Arrange
  scene->AddObject(
      factory.Create(std::string(ObjectTypes::Triangle)));  // obj 0, id 1
  scene->AddObject(
      factory.Create(std::string(ObjectTypes::Pyramid)));  // obj 1, id 2
  scene->AddObject(
      factory.Create(std::string(ObjectTypes::Triangle)));  // obj 2, id 3

  uint32_t id_obj0 = scene->GetSceneObjects()[0]->id;
  uint32_t id_obj2 = scene->GetSceneObjects()[2]->id;

  scene->SetSelectedObjectByID(id_obj2);  // Select the 3rd object (index 2)
  ASSERT_EQ(scene->GetSelectedObject()->id, id_obj2);

  // Act: Delete the first object (index 0)
  scene->DeleteObjectByID(id_obj0);

  // Assert
  ASSERT_EQ(scene->GetSceneObjects().size(), 2);
  // The selected object should still be the same one, but its index in the
  // scene's internal list should now be 1 instead of 2.
  ASSERT_NE(scene->GetSelectedObject(), nullptr);
  EXPECT_EQ(scene->GetSelectedObject()->id, id_obj2);
  // We can verify this by checking the object at index 1 is the selected one
  EXPECT_EQ(scene->GetSceneObjects()[1]->id, id_obj2);
}

TEST_F(SceneTest, CannotDeleteNonSelectableObject) {
  // Arrange
  auto obj = factory.Create(std::string(ObjectTypes::Triangle));
  obj->isSelectable = false;
  uint32_t id = obj->id;
  scene->AddObject(std::move(obj));
  scene->SetSelectedObjectByID(id);  // Attempt to select

  // Assert: Selection should fail
  ASSERT_EQ(scene->GetSelectedObject(), nullptr);
  ASSERT_EQ(scene->GetSceneObjects().size(), 1);

  // Act: Attempt to delete by ID
  scene->DeleteObjectByID(id);

  // Assert: Deletion should fail
  EXPECT_EQ(scene->GetSceneObjects().size(), 1);
}