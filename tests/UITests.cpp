#include "gtest/gtest.h"
#include "Core/Application.h"
#include "Core/UI/InspectorView.h"
#include "Core/UI/HierarchyView.h"
#include "Core/UI/MenuBar.h"
#include "Core/UI/SettingsWindow.h"
#include "Core/UI/AppUI.h"
#include "Scene/Scene.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Core/UI/BrushSettings.h"
#include "Core/SettingsManager.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Grid.h"
#include "Scene/Objects/Icosphere.h"
#include "Renderer/OpenGLRenderer.h"
#include "Sculpting/SubObjectSelection.h"
#include "Interfaces/IEditableMesh.h"
#include <vector>

class UITest : public ::testing::Test {
protected:
    Application& app = Application::Get();
    InspectorView* inspectorView = nullptr;
    ISceneObject* test_object_ptr = nullptr;
    uint32_t test_object_id = 0;

    void SetUp() override {
        app.GetScene()->Clear();
        app.GetScene()->ProcessDeferredDeletions();
        app.GetScene()->Load("test_scene_empty.json");

        std::unique_ptr<ISceneObject> new_test_obj = app.GetObjectFactory()->Create(std::string(ObjectTypes::Icosphere));
        app.GetScene()->AddObject(std::move(new_test_obj)); 
        test_object_ptr = app.GetScene()->GetSceneObjects()[0].get(); 
        test_object_id = test_object_ptr->id;

        app.SelectObject(test_object_id);
        app.SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::VERTEX);

        inspectorView = app.GetUI()->GetView<InspectorView>();
        ASSERT_NE(inspectorView, nullptr);
    }

    void TearDown() override {
        app.GetScene()->Load("test_scene_empty.json");
    }
};

TEST_F(UITest, ExtrudeOperation_IncreasesMeshComplexity) {
    ASSERT_NE(test_object_ptr, nullptr);
    auto* mesh = test_object_ptr->GetEditableMesh();
    ASSERT_NE(mesh, nullptr);

    size_t initialVertexCount = mesh->GetVertices().size();
    size_t initialIndexCount = mesh->GetIndices().size();

    app.SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::FACE);
    app.GetSelection()->SelectFaceForTest(0); // CORRECTED: Consistent function name
    ASSERT_EQ(app.GetSelection()->GetSelectedFaces().size(), 1);

    app.RequestExtrude(0.2f);
    app.ProcessPendingActions_ForTests();

    EXPECT_GT(mesh->GetVertices().size(), initialVertexCount);
    EXPECT_GT(mesh->GetIndices().size(), initialIndexCount);
}

TEST_F(UITest, WeldOperation_MergesVertices) {
    ASSERT_NE(test_object_ptr, nullptr);
    auto* mesh = test_object_ptr->GetEditableMesh();
    ASSERT_NE(mesh, nullptr);

    // CORRECTED: Consistent function name
    app.GetSelection()->SelectVertexForTest(0);
    app.GetSelection()->SelectVertexForTest(5);
    ASSERT_EQ(app.GetSelection()->GetSelectedVertices().size(), 2);

    glm::vec3 initialPos0 = mesh->GetVertices()[0];
    glm::vec3 initialPos5 = mesh->GetVertices()[5];
    glm::vec3 expectedWeldPoint = (initialPos0 + initialPos5) / 2.0f;

    app.RequestWeld();
    app.ProcessPendingActions_ForTests();

    glm::vec3 finalPos0 = mesh->GetVertices()[0];
    EXPECT_NEAR(glm::distance(finalPos0, expectedWeldPoint), 0.0f, 1e-5f);
    EXPECT_TRUE(app.GetSelection()->GetSelectedVertices().empty());
}

TEST_F(UITest, MoveAlongNormalOperation_DisplacesVertices) {
    ASSERT_NE(test_object_ptr, nullptr);
    auto* mesh = test_object_ptr->GetEditableMesh();
    ASSERT_NE(mesh, nullptr);

    const uint32_t vertexIndexToMove = 10;
    app.GetSelection()->SelectVertexForTest(vertexIndexToMove); // CORRECTED: Consistent function name
    ASSERT_EQ(app.GetSelection()->GetSelectedVertices().size(), 1);
    
    mesh->RecalculateNormals();
    glm::vec3 initialPos = mesh->GetVertices()[vertexIndexToMove];
    glm::vec3 normal = mesh->GetNormals()[vertexIndexToMove];
    ASSERT_GT(glm::length(normal), 0.0f);

    const float moveDist = 0.25f;
    app.RequestMoveSelection(moveDist);
    app.ProcessPendingActions_ForTests();

    glm::vec3 finalPos = mesh->GetVertices()[vertexIndexToMove];
    glm::vec3 displacement = finalPos - initialPos;

    EXPECT_NEAR(glm::length(displacement), moveDist, 1e-5f);
    EXPECT_GT(glm::dot(glm::normalize(displacement), glm::normalize(normal)), 0.99f);
}
