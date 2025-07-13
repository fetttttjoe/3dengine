#include "gtest/gtest.h"
#include "Sculpting/Tools/MoveVertexTool.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/UI/BrushSettings.h"
#include "Core/Log.h"
#include "Core/MathHelpers.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <iostream>
#include <limits>

#include <imgui.h>
#include "TestMocks.h" // Include common mocks

// MockImGui and MockCamera definitions are in TestMocks.h and TestMocks.cpp

class MoveVertexToolAdvancedTest : public ::testing::Test {
protected:
    MoveVertexTool tool;
    SculptableMesh mesh;
    BrushSettings settings;

    int viewportWidth = 800;
    int viewportHeight = 600;

    void SetUp() override {
        std::vector<float> vertices_data = {
            -1.0f, 1.0f, 0.0f,   // Vertex 0
            1.0f, 1.0f, 0.0f,    // Vertex 1
            -1.0f, -1.0f, 0.0f,  // Vertex 2
            1.0f, -1.0f, 0.0f,   // Vertex 3
            0.0f, 0.0f, 2.0f,    // Vertex 4 (in front of the quad)
            0.0f, 0.0f, -2.0f    // Vertex 5 (behind the quad)
        };
        std::vector<unsigned int> indices_data = { 0, 2, 1, 1, 2, 3 };

        mesh.Initialize(vertices_data, indices_data);

        MockImGui::g_MockIO.MouseDelta = ImVec2(0, 0);
        tool.Reset();
    }
};

// ... (existing tests FindClosestVertex_Found, NotFound, WithinThreshold, OutsideThreshold) ...

TEST_F(MoveVertexToolAdvancedTest, OnMouseDrag_TinyDeltaDoesNotMoveVertex) {
    // 1. Select Vertex 0
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
    glm::vec3 initialVertexPos = mesh.m_Vertices[0];

    // 2. Simulate a TRULY ZERO mouse drag delta.
    // This ensures the `if (glm::length(mouseDelta) == 0.0f)` condition in OnMouseDrag
    // is met, and no calculations are performed, preventing floating point errors.
    glm::vec2 zeroMouseDelta = glm::vec2(0.0f, 0.0f);

    // 3. Call OnMouseDrag
    tool.OnMouseDrag(mesh, zeroMouseDelta, // Pass a truly zero delta
                     MockCamera::s_ViewMatrix, MockCamera::s_ProjectionMatrix,
                     viewportWidth, viewportHeight);

    // 4. Assert position did NOT significantly change
    // Using EXPECT_NEAR with a small epsilon is still good practice for float comparisons,
    // but with a zero delta, it should ideally be exact.
    EXPECT_NEAR(mesh.m_Vertices[0].x, initialVertexPos.x, 1e-6f);
    EXPECT_NEAR(mesh.m_Vertices[0].y, initialVertexPos.y, 1e-6f);
    EXPECT_NEAR(mesh.m_Vertices[0].z, initialVertexPos.z, 1e-6f);
    
    // Cleanup
    tool.OnMouseRelease(mesh);
}