#include "gtest/gtest.h"
#include "Sculpting/Tools/MoveVertexTool.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/UI/BrushSettings.h"
#include "Core/Log.h" // For Log::Debug
#include "Core/MathHelpers.h" // For ToString and ToGlm/ToImGui conversions
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>             // For glm::distance2
#include <vector>
#include <iostream> // Keep for potential other std::ostream uses, but not for glm::vec logging
#include <limits>   // For std::numeric_limits

#include <imgui.h> // IMPORTANT: ImGui header must be included before using its types like ImVec2, ImGuiMouseButton_

#include "TestMocks.h"



class MoveVertexToolTest : public ::testing::Test {
protected:
    MoveVertexTool tool;
    SculptableMesh mesh;
    BrushSettings settings;

    int viewportWidth = 800;
    int viewportHeight = 600;

    void SetUp() override {
        std::vector<float> vertices_data = {
            -1.0f, 1.0f, 0.0f,  // Vertex 0 (Top-Left, World)
            1.0f, 1.0f, 0.0f,   // Vertex 1 (Top-Right, World)
            -1.0f, -1.0f, 0.0f, // Vertex 2 (Bottom-Left, World)
            1.0f, -1.0f, 0.0f   // Vertex 3 (Bottom-Right, World)
        };
        std::vector<unsigned int> indices_data = { 0, 2, 1, 1, 2, 3 };

        mesh.Initialize(vertices_data, indices_data);

        MockImGui::g_MockIO.MouseDelta = ImVec2(0, 0);
        
        tool.Reset();
    }
};

TEST_F(MoveVertexToolTest, FindClosestVertex_Found) {
    glm::vec3 targetWorldPos = mesh.m_Vertices[0];
    glm::vec2 mouseScreenPos = MockCamera::WorldToScreen(
        targetWorldPos, MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    float pickPixelThreshold = 5.0f;

    int selectedIndex = tool.FindClosestVertex(mesh, mouseScreenPos,
                                               MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
                                               viewportWidth, viewportHeight, pickPixelThreshold);
    ASSERT_EQ(selectedIndex, 0) << "Should select Vertex 0 when mouse is directly over its projected screen position.";
}

TEST_F(MoveVertexToolTest, FindClosestVertex_NotFound) {
    glm::vec2 mouseScreenPos = glm::vec2(0.0f, 0.0f);
    float pickPixelThreshold = 5.0f;

    int selectedIndex = tool.FindClosestVertex(mesh, mouseScreenPos,
                                               MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
                                               viewportWidth, viewportHeight, pickPixelThreshold);
    ASSERT_EQ(selectedIndex, -1) << "Should not select any vertex when mouse is far away.";
}

TEST_F(MoveVertexToolTest, FindClosestVertex_WithinThreshold) {
    glm::vec3 targetWorldPos = mesh.m_Vertices[1];
    glm::vec2 baseScreenPos = MockCamera::WorldToScreen(
        targetWorldPos, MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    glm::vec2 mouseScreenPos = baseScreenPos + glm::vec2(3.0f, 2.0f);
    float pickPixelThreshold = 5.0f;

    int selectedIndex = tool.FindClosestVertex(mesh, mouseScreenPos,
                                               MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
                                               viewportWidth, viewportHeight, pickPixelThreshold);
    ASSERT_EQ(selectedIndex, 1) << "Should select Vertex 1 if mouse is slightly off but within pixel threshold.";
}

TEST_F(MoveVertexToolTest, FindClosestVertex_OutsideThreshold) {
    glm::vec3 targetWorldPos = mesh.m_Vertices[1];
    glm::vec2 baseScreenPos = MockCamera::WorldToScreen(
        targetWorldPos, MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    glm::vec2 mouseScreenPos = baseScreenPos + glm::vec2(6.0f, 6.0f);
    float pickPixelThreshold = 5.0f;

    int selectedIndex = tool.FindClosestVertex(mesh, mouseScreenPos,
                                               MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
                                               viewportWidth, viewportHeight, pickPixelThreshold);
    ASSERT_EQ(selectedIndex, -1) << "Should not select Vertex 1 if mouse is outside pixel threshold.";
}


TEST_F(MoveVertexToolTest, OnMouseDown_SelectsVertex) {
    glm::vec3 rayOrigin = MockCamera::GetPosition();
    glm::vec3 rayDirection = MockCamera::ScreenToWorldRay(glm::vec2(0,0), viewportWidth, viewportHeight);
    
    glm::vec2 mouseScreenPos = MockCamera::WorldToScreen(
        mesh.m_Vertices[0], MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    tool.OnMouseDown(mesh, rayOrigin, rayDirection,
                     mouseScreenPos,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    ASSERT_EQ(tool.GetSelectedVertexIndex(), 0) << "Vertex 0 should be selected on mouse down.";
}

TEST_F(MoveVertexToolTest, OnMouseDown_NoSelectionWhenMiss) {
    glm::vec3 rayOrigin = MockCamera::GetPosition();
    glm::vec3 rayDirection = MockCamera::ScreenToWorldRay(glm::vec2(0,0), viewportWidth, viewportHeight);

    glm::vec2 mouseScreenPos = glm::vec2(0.0f, 0.0f);

    tool.OnMouseDown(mesh, rayOrigin, rayDirection,
                     mouseScreenPos,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    ASSERT_EQ(tool.GetSelectedVertexIndex(), -1) << "No vertex should be selected if mouse down misses.";
}

TEST_F(MoveVertexToolTest, OnMouseDrag_MovesSelectedVertex) {
    glm::vec3 rayOrigin = MockCamera::GetPosition();
    glm::vec3 rayDirection = MockCamera::ScreenToWorldRay(glm::vec2(0,0), viewportWidth, viewportHeight);
    glm::vec2 mouseScreenPosForClick = MockCamera::WorldToScreen(
        mesh.m_Vertices[0], MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    tool.OnMouseDown(mesh, rayOrigin, rayDirection,
                     mouseScreenPosForClick,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    ASSERT_EQ(tool.GetSelectedVertexIndex(), 0) << "Vertex 0 should be selected.";
    glm::vec3 initialPos = mesh.m_Vertices[0];

    glm::vec2 mouseDelta = glm::vec2(100.0f, -50.0f);

    glm::mat4 currentViewProj = MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix;
    glm::mat4 invInitialViewProj = glm::inverse(currentViewProj);

    glm::vec2 initialSelectedVertexScreenPos_from_tool_internal_logic = MockCamera::WorldToScreen(
        initialPos, currentViewProj, viewportWidth, viewportHeight
    );
    glm::vec2 newScreenPos = initialSelectedVertexScreenPos_from_tool_internal_logic + mouseDelta;

    glm::vec4 clipPosInitial = currentViewProj * glm::vec4(initialPos, 1.0f);
    float storedDepthNDC = (clipPosInitial.w != 0.0f) ? (clipPosInitial.z / clipPosInitial.w) : 0.0f;

    glm::vec3 expectedWorldPosAfterDrag = MockCamera::ScreenToWorldPoint(
        newScreenPos, storedDepthNDC, invInitialViewProj, viewportWidth, viewportHeight
    );

    tool.OnMouseDrag(mesh, mouseDelta,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);

    EXPECT_NEAR(mesh.m_Vertices[0].x, expectedWorldPosAfterDrag.x, 0.01f) << "Vertex X should move.";
    EXPECT_NEAR(mesh.m_Vertices[0].y, expectedWorldPosAfterDrag.y, 0.01f) << "Vertex Y should move.";
    EXPECT_NEAR(mesh.m_Vertices[0].z, expectedWorldPosAfterDrag.z, 0.01f) << "Vertex Z should move.";

    EXPECT_EQ(mesh.m_Vertices[1], glm::vec3(1.0f, 1.0f, 0.0f)) << "Other vertices should not move.";
    EXPECT_EQ(mesh.m_Vertices[2], glm::vec3(-1.0f, -1.0f, 0.0f)) << "Other vertices should not move.";
    EXPECT_EQ(mesh.m_Vertices[3], glm::vec3(1.0f, -1.0f, 0.0f)) << "Other vertices should not move.";
}

TEST_F(MoveVertexToolTest, OnMouseDrag_DoesNothingIfNotSelected) {
    glm::vec3 initialVertexPos = mesh.m_Vertices[0];
    glm::vec2 mouseDelta = glm::vec2(10.0f, 10.0f);

    tool.OnMouseDrag(mesh, mouseDelta,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    
    EXPECT_EQ(mesh.m_Vertices[0], initialVertexPos) << "Vertex should not move if not selected.";
    ASSERT_EQ(tool.GetSelectedVertexIndex(), -1) << "Tool should remain unselected.";
}


TEST_F(MoveVertexToolTest, OnMouseRelease_ResetsStateAndRecalculatesNormals) {
    glm::vec3 rayOrigin = MockCamera::GetPosition();
    glm::vec3 rayDirection = MockCamera::ScreenToWorldRay(glm::vec2(0,0), viewportWidth, viewportHeight);
    glm::vec2 mouseScreenPosForClick = MockCamera::WorldToScreen(
        mesh.m_Vertices[0], MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);

    tool.OnMouseDown(mesh, rayOrigin, rayDirection,
                     mouseScreenPosForClick,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    ASSERT_EQ(tool.GetSelectedVertexIndex(), 0);

    tool.OnMouseDrag(mesh, glm::vec2(1.0f, 1.0f),
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);

    tool.OnMouseRelease(mesh);

    ASSERT_EQ(tool.GetSelectedVertexIndex(), -1) << "Tool state should be reset after release.";
}

TEST_F(MoveVertexToolTest, Apply_DoesNothingDirectlyIfNoInteraction) {
    glm::vec3 initialVertexPos = mesh.m_Vertices[0];
    tool.Apply(mesh, glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec2(0,0), settings,
               MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix, viewportWidth, viewportHeight);
    EXPECT_EQ(mesh.m_Vertices[0], initialVertexPos) << "Apply should not move vertex if mouse delta is zero and no active drag.";
    ASSERT_EQ(tool.GetSelectedVertexIndex(), -1) << "Tool should remain unselected.";
}

TEST_F(MoveVertexToolTest, Apply_CallsOnMouseDragIfActiveAndMoving) {
    glm::vec3 rayOriginForMouseDown = MockCamera::GetPosition();
    glm::vec3 rayDirectionForMouseDown = MockCamera::ScreenToWorldRay(glm::vec2(0,0), viewportWidth, viewportHeight);
    glm::vec2 mouseScreenPosForClick = MockCamera::WorldToScreen(
        mesh.m_Vertices[0], MockCamera::s_ProjectionMatrix * MockCamera::s_ViewMatrix,
        viewportWidth, viewportHeight);
        
    tool.OnMouseDown(mesh, rayOriginForMouseDown, rayDirectionForMouseDown,
                     mouseScreenPosForClick,
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);
    ASSERT_EQ(tool.GetSelectedVertexIndex(), 0);
    glm::vec3 initialVertexPos = mesh.m_Vertices[0];

    glm::vec2 testMouseDelta = glm::vec2(50.0f, 50.0f);
    tool.Apply(mesh, glm::vec3(0,0,0), glm::vec3(0,0,-1), testMouseDelta, settings,
               MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix, viewportWidth, viewportHeight);
    
    EXPECT_NE(mesh.m_Vertices[0], initialVertexPos) << "Vertex should move if Apply calls OnMouseDrag.";

    tool.OnMouseRelease(mesh);
}