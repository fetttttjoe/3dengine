#include "gtest/gtest.h"
#include "Sculpting/SubObjectSelection.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/MathHelpers.h"
#include <glm/gtc/matrix_transform.hpp>

class SelectionTest : public ::testing::Test {
protected:
    SubObjectSelection selection;
    SculptableMesh mesh;
    
    // Mock camera and viewport settings
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    int viewportWidth = 800;
    int viewportHeight = 600;

    void SetUp() override {
        // Create a simple triangle mesh
        std::vector<float> vertices = {
            0.0f, 1.0f, 0.0f,  // Vertex 0 (Top)
           -1.0f, -1.0f, 0.0f, // Vertex 1 (Left)
            1.0f, -1.0f, 0.0f   // Vertex 2 (Right)
        };
        std::vector<unsigned int> indices = {0, 1, 2};
        mesh.Initialize(vertices, indices);

        // Set up a simple orthographic camera looking at the triangle
        viewMatrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        projMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    }
};

TEST_F(SelectionTest, FindClosestVertex_SelectsCorrectVertex) {
    // Arrange: Project the world position of the top vertex (Vertex 0) to screen space
    glm::vec3 topVertexWorldPos = mesh.GetVertices()[0];
    glm::vec2 topVertexScreenPos = MathHelpers::WorldToScreen(
        topVertexWorldPos, projMatrix * viewMatrix, viewportWidth, viewportHeight
    );

    // Act: "Click" exactly on the screen position of the top vertex
    int closestIndex = selection.FindClosestVertex_ForTests(
        mesh, glm::mat4(1.0f), topVertexScreenPos, viewMatrix, projMatrix,
        viewportWidth, viewportHeight, 10.0f
    );

    // Assert: The returned index should be 0
    EXPECT_EQ(closestIndex, 0);
}

TEST_F(SelectionTest, FindClosestVertex_MissesWhenFarAway) {
    // Arrange: Define a screen position that is far from any vertex
    glm::vec2 farAwayScreenPos(0.0f, 0.0f);

    // Act: "Click" on the far away position
    int closestIndex = selection.FindClosestVertex_ForTests(
        mesh, glm::mat4(1.0f), farAwayScreenPos, viewMatrix, projMatrix,
        viewportWidth, viewportHeight, 10.0f
    );

    // Assert: No vertex should be found
    EXPECT_EQ(closestIndex, -1);
}

TEST_F(SelectionTest, FindClosestVertex_SelectsWithTransform) {
    // Arrange: Move the object and find the new screen position of a vertex
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    glm::vec3 rightVertexWorldPos = glm::vec3(modelMatrix * glm::vec4(mesh.GetVertices()[2], 1.0f));
    
    glm::vec2 rightVertexScreenPos = MathHelpers::WorldToScreen(
        rightVertexWorldPos, projMatrix * viewMatrix, viewportWidth, viewportHeight
    );

    // Act: "Click" on the transformed vertex's screen position
    int closestIndex = selection.FindClosestVertex_ForTests(
        mesh, modelMatrix, rightVertexScreenPos, viewMatrix, projMatrix,
        viewportWidth, viewportHeight, 10.0f
    );

    // Assert: The correct vertex (index 2) should be found
    EXPECT_EQ(closestIndex, 2);
}
