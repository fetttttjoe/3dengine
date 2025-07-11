#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "gtest/gtest.h"
#include "Core/PropertyNames.h"
#include <cstdio> // Required for std::remove

class MockSceneObject : public ISceneObject {
public:
    explicit MockSceneObject(const std::string& type = "Mock") : m_Type(type) {
        m_Properties.Add<glm::vec3>(PropertyNames::Position, {0.0f, 0.0f, 0.0f});
    }
    std::string GetTypeString() const override { return m_Type; }
    void Draw(class OpenGLRenderer& renderer, const glm::mat4& view, const glm::mat4& projection) override {}
    void DrawForPicking(Shader&, const glm::mat4&, const glm::mat4&) override {}
    void DrawHighlight(const glm::mat4&, const glm::mat4&) const override {}
    void RebuildMesh() override {}
    PropertySet& GetPropertySet() override { return m_Properties; }
    const PropertySet& GetPropertySet() const override { return m_Properties; }
    const glm::mat4& GetTransform() const override { return m_Transform; }
    glm::vec3 GetPosition() const override { return m_Properties.GetValue<glm::vec3>(PropertyNames::Position); }
    glm::quat GetRotation() const override { return m_Rotation; }
    glm::vec3 GetScale() const override { return m_Scale; }
    void SetPosition(const glm::vec3& p) override { m_Properties.SetValue<glm::vec3>(PropertyNames::Position, p); }
    void SetRotation(const glm::quat& r) override { m_Rotation = r; }
    void SetScale(const glm::vec3& s) override { m_Scale = s; }
    void SetEulerAngles(const glm::vec3&) override {}
    std::vector<GizmoHandleDef> GetGizmoHandleDefs() override { return {}; }
    void OnGizmoUpdate(const std::string&, float, const glm::vec3&) override {}

    SculptableMesh* GetSculptableMesh() override { return nullptr; }
    bool IsMeshDirty() const override { return false; }
    void SetMeshDirty(bool dirty) override {}

private:
    std::string m_Type;
    PropertySet m_Properties;
    glm::mat4 m_Transform{1.0f};
    glm::quat m_Rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_Scale{1.0f};
};

class SceneTest : public ::testing::Test {
protected:
    void SetUp() override {
        factory.Register(std::string(ObjectTypes::Pyramid), []() {
            return std::make_unique<MockSceneObject>(std::string(ObjectTypes::Pyramid));
        });
        scene = std::make_unique<Scene>(&factory);
    }
    SceneObjectFactory factory;
    std::unique_ptr<Scene> scene;
};

// --- Positive Tests ---

TEST_F(SceneTest, AddAndGetObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    ASSERT_EQ(scene->GetSceneObjects().size(), 1);
    ISceneObject* obj = scene->GetObjectByID(1);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->id, 1);
}

TEST_F(SceneTest, DeleteObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    ASSERT_EQ(scene->GetSceneObjects().size(), 1);
    scene->QueueForDeletion(1);
    scene->ProcessDeferredDeletions();
    ASSERT_EQ(scene->GetSceneObjects().size(), 0);
}

TEST_F(SceneTest, SelectObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    scene->SetSelectedObjectByID(1);
    ISceneObject* selected = scene->GetSelectedObject();
    ASSERT_NE(selected, nullptr);
    EXPECT_EQ(selected->id, 1);
    EXPECT_TRUE(selected->isSelected);
}

TEST_F(SceneTest, DuplicateObject) {
    auto pyramid = factory.Create(std::string(ObjectTypes::Pyramid));
    pyramid->SetPosition({1, 2, 3});
    scene->AddObject(std::move(pyramid));
    scene->DuplicateObject(1);
    ASSERT_EQ(scene->GetSceneObjects().size(), 2);
    ISceneObject* original = scene->GetObjectByID(1);
    ISceneObject* clone = scene->GetObjectByID(2);
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->GetTypeString(), original->GetTypeString());
    EXPECT_NE(clone->id, original->id);
    EXPECT_NE(clone->GetPosition(), original->GetPosition());
}

TEST_F(SceneTest, SaveAndLoad) {
    const char* tempFilename = "temporary_scene_for_save_test.json";
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    ISceneObject* obj = scene->GetObjectByID(1);
    obj->name = "MyPyramid";
    obj->SetPosition({5, 5, 5});
    scene->Save(tempFilename);
    scene->Clear();

    // Use a mock factory for loading since we only need to test deserialization
    SceneObjectFactory loadFactory;
    loadFactory.Register(std::string(ObjectTypes::Pyramid), []() {
        return std::make_unique<MockSceneObject>(std::string(ObjectTypes::Pyramid));
    });
    Scene loadScene(&loadFactory);
    loadScene.Load(tempFilename);
    
    ASSERT_EQ(loadScene.GetSceneObjects().size(), 1);
    ISceneObject* loadedObject = loadScene.GetObjectByID(1);
    ASSERT_NE(loadedObject, nullptr);
    EXPECT_EQ(loadedObject->name, "MyPyramid");
    EXPECT_EQ(loadedObject->GetPosition(), glm::vec3(5, 5, 5));
    std::remove(tempFilename);
}

// --- Negative and Edge Case Tests ---

TEST_F(SceneTest, GetNonExistentObject) {
    EXPECT_EQ(scene->GetObjectByID(999), nullptr);
}

TEST_F(SceneTest, DeleteNonExistentObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    ASSERT_EQ(scene->GetSceneObjects().size(), 1);
    scene->QueueForDeletion(999);
    scene->ProcessDeferredDeletions();
    EXPECT_EQ(scene->GetSceneObjects().size(), 1);
}

TEST_F(SceneTest, DuplicateNonExistentObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    ASSERT_EQ(scene->GetSceneObjects().size(), 1);
    scene->DuplicateObject(999);
    EXPECT_EQ(scene->GetSceneObjects().size(), 1);
}

TEST_F(SceneTest, ClearScene) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    scene->SetSelectedObjectByID(1);
    scene->Clear();
    EXPECT_EQ(scene->GetSceneObjects().size(), 0);
    EXPECT_EQ(scene->GetSelectedObject(), nullptr);
}

TEST_F(SceneTest, UniqueIDsAreAssigned) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid))); // ID 1
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid))); // ID 2
    scene->QueueForDeletion(1);
    scene->ProcessDeferredDeletions();
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid))); // Should get ID 3
    EXPECT_NE(scene->GetObjectByID(2), nullptr);
    EXPECT_NE(scene->GetObjectByID(3), nullptr);
}

TEST_F(SceneTest, DeselectObject) {
    scene->AddObject(factory.Create(std::string(ObjectTypes::Pyramid)));
    scene->SetSelectedObjectByID(1);
    ASSERT_NE(scene->GetSelectedObject(), nullptr);
    scene->SetSelectedObjectByID(0); // ID 0 is reserved for "no selection"
    EXPECT_EQ(scene->GetSelectedObject(), nullptr);
}