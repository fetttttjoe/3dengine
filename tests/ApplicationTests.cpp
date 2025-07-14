#include "gtest/gtest.h"
#include "Core/Application.h"
#include "Scene/Scene.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/Sphere.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Core/SettingsManager.h"
#include "Scene/TransformGizmo.h"
#include "Scene/Grid.h"
#include "Scene/Objects/Icosphere.h"
#include "Scene/Objects/CustomMesh.h" // Needed for import test
#include "Renderer/OpenGLRenderer.h"
#include <vector>
#include <fstream> // Needed for import test
#include <cstdio>  // For std::remove

class ApplicationTest : public ::testing::Test {
protected:
    Application& app = Application::Get();
    ISceneObject* test_object_ptr = nullptr;
    uint32_t test_object_id = 0;

    void SetUp() override {
        app.GetScene()->Clear();
        app.GetScene()->ProcessDeferredDeletions();
        app.GetScene()->Load("test_scene_empty.json");

        app.SetEditorMode(EditorMode::TRANSFORM);
        
        std::unique_ptr<ISceneObject> new_test_obj = app.GetObjectFactory()->Create(std::string(ObjectTypes::Icosphere));
        
        app.GetScene()->AddObject(std::move(new_test_obj)); 
        test_object_ptr = app.GetScene()->GetSceneObjects()[0].get(); 
        test_object_id = test_object_ptr->id;

        app.SelectObject(test_object_id); 
    }

    void TearDown() override {
        app.GetScene()->Load("test_scene_empty.json");
    }
};

TEST_F(ApplicationTest, RequestObjectCreation_AddsCorrectObjectToScene) {
    ASSERT_EQ(app.GetScene()->GetSceneObjects().size(), 1);
    app.RequestObjectCreation(std::string(ObjectTypes::Pyramid));
    app.ProcessPendingActions_ForTests();
    ASSERT_EQ(app.GetScene()->GetSceneObjects().size(), 2);
    EXPECT_EQ(app.GetScene()->GetSceneObjects().back()->GetTypeString(), ObjectTypes::Pyramid);
}

TEST_F(ApplicationTest, RequestObjectDuplication_CreatesCorrectClone) {
    ISceneObject* original = test_object_ptr;
    original->name = "OriginalIcosphere";
    ASSERT_EQ(app.GetScene()->GetSceneObjects().size(), 1);
    app.RequestObjectDuplication(original->id);
    app.ProcessPendingActions_ForTests();
    ASSERT_EQ(app.GetScene()->GetSceneObjects().size(), 2);
    ISceneObject* clone = app.GetScene()->GetObjectByID(original->id + 1);
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->name, "OriginalIcosphere (1)");
}

TEST_F(ApplicationTest, RequestObjectDeletion_RemovesObjectFromScene) {
    uint32_t idToDelete = test_object_ptr->id;
    app.RequestObjectDeletion(idToDelete);
    app.ProcessPendingActions_ForTests();
    app.GetScene()->ProcessDeferredDeletions();
    EXPECT_EQ(app.GetScene()->GetSceneObjects().size(), 0);
    EXPECT_EQ(app.GetScene()->GetObjectByID(idToDelete), nullptr);
}

TEST_F(ApplicationTest, SetEditorMode_TransitionsCorrectlyAndManagesGizmo) {
    EXPECT_EQ(app.GetEditorMode(), EditorMode::TRANSFORM);
    EXPECT_NE(app.GetTransformGizmo()->GetTarget(), nullptr);
    app.SetEditorMode(EditorMode::SCULPT); 
    EXPECT_EQ(app.GetTransformGizmo()->GetTarget(), nullptr);
    app.SetEditorMode(EditorMode::TRANSFORM);
    EXPECT_NE(app.GetTransformGizmo()->GetTarget(), nullptr);
}

TEST_F(ApplicationTest, SetEditorMode_WithNoSelection_RevertsToTransform) {
    app.SelectObject(0);
    ASSERT_EQ(app.GetScene()->GetSelectedObject(), nullptr);
    app.SetEditorMode(EditorMode::SCULPT);
    EXPECT_EQ(app.GetEditorMode(), EditorMode::TRANSFORM);
}

TEST_F(ApplicationTest, ImportModel_AddsCustomMeshToScene) {
    // Setup: Create a simple temporary OBJ file
    const char* filepath = "temp_import_test.obj";
    std::ofstream objFile(filepath);
    objFile << "v 1.0 1.0 0.0\n";
    objFile << "v 1.0 0.0 0.0\n";
    objFile << "v 0.0 1.0 0.0\n";
    objFile << "f 1 2 3\n";
    objFile.close();

    size_t initialObjectCount = app.GetScene()->GetSceneObjects().size(); // Should be 1

    // Action: Call the import function
    app.ImportModel(filepath);

    // Assert: A new object should be in the scene
    ASSERT_EQ(app.GetScene()->GetSceneObjects().size(), initialObjectCount + 1);
    
    // Assert: The new object should be a CustomMesh
    ISceneObject* importedObject = app.GetScene()->GetSceneObjects().back().get();
    ASSERT_NE(importedObject, nullptr);
    EXPECT_EQ(importedObject->GetTypeString(), ObjectTypes::CustomMesh);

    // Assert: The mesh data should be correct
    IEditableMesh* mesh = importedObject->GetEditableMesh();
    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->GetVertices().size(), 3);
    EXPECT_EQ(mesh->GetIndices().size(), 3);

    // Cleanup
    std::remove(filepath);
}
