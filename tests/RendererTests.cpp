#include "gtest/gtest.h"
#include "Core/Application.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Objects/Icosphere.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Factories/SceneObjectFactory.h"

class RendererTest : public ::testing::Test {
protected:
    Application& app = Application::Get();
    OpenGLRenderer* renderer = nullptr;
    Scene* scene = nullptr;

    void SetUp() override {
        // Ensure we have a clean state for each test
        app.GetScene()->Load("test_scene_empty.json");
        app.GetScene()->ProcessDeferredDeletions();

        renderer = app.GetRenderer();
        scene = app.GetScene();
        ASSERT_NE(renderer, nullptr);
        ASSERT_NE(scene, nullptr);
    }
};

TEST_F(RendererTest, SyncSceneObjects_CreatesGpuResourcesForNewObject) {
    // Initial state: No GPU resources should exist for our object yet
    uint32_t objectId = 1; // This will be the ID of the first object added
    ASSERT_EQ(renderer->GetGpuResources().count(objectId), 0);

    // Action: Add a new object to the scene and sync the renderer
    scene->AddObject(app.GetObjectFactory()->Create(std::string(ObjectTypes::Icosphere)));
    ISceneObject* newObject = scene->GetObjectByID(objectId);
    ASSERT_NE(newObject, nullptr);
    newObject->SetMeshDirty(true); // Ensure the renderer will process it

    renderer->SyncSceneObjects(*scene);

    // Assert: GPU resources should now exist for the new object
    ASSERT_EQ(renderer->GetGpuResources().count(objectId), 1);
    const auto& resources = renderer->GetGpuResources().at(objectId);
    EXPECT_NE(resources.vao, 0);
    EXPECT_NE(resources.vboPositions, 0);
    EXPECT_NE(resources.ebo, 0);
    EXPECT_GT(resources.indexCount, 0);
}

TEST_F(RendererTest, SyncSceneObjects_ReleasesGpuResourcesForDeletedObject) {
    // Setup: Add an object and sync to ensure its resources are created
    scene->AddObject(app.GetObjectFactory()->Create(std::string(ObjectTypes::Icosphere)));
    uint32_t objectId = scene->GetSceneObjects().back()->id;
    scene->GetSceneObjects().back()->SetMeshDirty(true);
    renderer->SyncSceneObjects(*scene);
    ASSERT_EQ(renderer->GetGpuResources().count(objectId), 1);

    // Action: Delete the object from the scene and sync again
    scene->QueueForDeletion(objectId);
    scene->ProcessDeferredDeletions();
    renderer->SyncSceneObjects(*scene);

    // Assert: The GPU resources for the deleted object should be gone
    EXPECT_EQ(renderer->GetGpuResources().count(objectId), 0);
}

TEST_F(RendererTest, ClearAllGpuResources_EmptiesResourceMap) {
    // Setup: Add a couple of objects and sync them
    scene->AddObject(app.GetObjectFactory()->Create(std::string(ObjectTypes::Icosphere)));
    scene->AddObject(app.GetObjectFactory()->Create(std::string(ObjectTypes::Pyramid)));
    for(const auto& obj : scene->GetSceneObjects()) {
        obj->SetMeshDirty(true);
    }
    renderer->SyncSceneObjects(*scene);
    ASSERT_EQ(renderer->GetGpuResources().size(), 2);

    // Action: Call the clear function
    renderer->ClearAllGpuResources();

    // Assert: The map of GPU resources should be empty
    EXPECT_TRUE(renderer->GetGpuResources().empty());
}
