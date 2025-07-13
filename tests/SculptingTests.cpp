#include "gtest/gtest.h"
#include "Sculpting/Tools/PushPullTool.h"
#include "Sculpting/Tools/SmoothTool.h"
#include "Sculpting/Tools/GrabTool.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/UI/BrushSettings.h"
#include "Core/Camera.h"
#include <glm/gtc/matrix_transform.hpp>


class SculptingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Initial quad vertices
    vertices_data = {-1.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 0.0f,
                     1.0f,  -1.0f, 0.0f, 1.0f,  1.0f,  0.0f};
    indices_data = {0, 1, 2, 0, 2, 3};
    mesh.Initialize(vertices_data, indices_data);
    
    settings.radius = 1.5f;
    settings.strength = 0.5f;
  }

  SculptableMesh mesh;
  PushPullTool pushPullTool;
  SmoothTool smoothTool;
  GrabTool grabTool;
  BrushSettings settings;
  std::vector<float> vertices_data;
  std::vector<unsigned int> indices_data;
  
  glm::vec3 dummyRayDirection = {0,0,-1};
  glm::vec2 dummyMouseDelta = {0,0};
  glm::mat4 dummyMatrix = glm::mat4(1.0f);
};

// --- Push/Pull Tool Tests ---

TEST_F(SculptingTest, PushPullToolAffectsCorrectVertices) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  settings.mode = SculptMode::Pull;
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  auto finalVertices = mesh.GetVertices();
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_NE(initialVertices[i], finalVertices[i]);
    EXPECT_GT(finalVertices[i].z, initialVertices[i].z);
  }
}

TEST_F(SculptingTest, PushAndPullWorkInOppositeDirections) {
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  // PULL
  mesh.Initialize(vertices_data, indices_data);
  auto verticesBeforePull = mesh.GetVertices();
  settings.mode = SculptMode::Pull;
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  float zChangePull = mesh.GetVertices()[0].z - verticesBeforePull[0].z;
  EXPECT_GT(zChangePull, 0.0f);
  // PUSH
  mesh.Initialize(vertices_data, indices_data);
  auto verticesBeforePush = mesh.GetVertices();
  settings.mode = SculptMode::Push;
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  float zChangePush = mesh.GetVertices()[0].z - verticesBeforePush[0].z;
  EXPECT_LT(zChangePush, 0.0f);
}

TEST_F(SculptingTest, PushPullToolStrengthIsAppliedCorrectly) {
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    settings.mode = SculptMode::Pull;

    // --- Test with LOW strength ---
    mesh.Initialize(vertices_data, indices_data);
    settings.strength = 0.1f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
    float zChangeLowStrength = mesh.GetVertices()[0].z;

    // --- Test with HIGH strength ---
    mesh.Initialize(vertices_data, indices_data);
    settings.strength = 1.0f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
    float zChangeHighStrength = mesh.GetVertices()[0].z;

    EXPECT_GT(zChangeHighStrength, zChangeLowStrength);
}

// --- Smooth Tool Tests ---

TEST_F(SculptingTest, SmoothToolAveragesVertices) {
    std::vector<float> smooth_vertices = {-1, 0, 0,  1, 0, 0,  0, 2, 0};
    std::vector<unsigned int> smooth_indices = {0, 2, 1};
    mesh.Initialize(smooth_vertices, smooth_indices);
    
    settings.radius = 2.0f;
    settings.strength = 1.0f;
    glm::vec3 hitPoint(0.0f, 1.0f, 0.0f);

    float initial_Y_height = mesh.GetVertices()[2].y;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float final_Y_height = mesh.GetVertices()[2].y;
    
    EXPECT_LT(final_Y_height, initial_Y_height);
    EXPECT_GT(final_Y_height, 0.0f);
}

TEST_F(SculptingTest, SmoothToolStrengthIsAppliedCorrectly) {
    std::vector<float> smooth_vertices = {-1, 0, 0,  1, 0, 0,  0, 2, 0};
    std::vector<unsigned int> smooth_indices = {0, 2, 1};
    glm::vec3 hitPoint(0.0f, 1.0f, 0.0f);

    // LOW strength
    mesh.Initialize(smooth_vertices, smooth_indices);
    settings.strength = 0.1f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float heightAfterLowStrength = mesh.GetVertices()[2].y;

    // HIGH strength
    mesh.Initialize(smooth_vertices, smooth_indices);
    settings.strength = 1.0f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float heightAfterHighStrength = mesh.GetVertices()[2].y;
    
    EXPECT_LT(heightAfterHighStrength, heightAfterLowStrength);
}

// --- Grab Tool Tests ---

TEST_F(SculptingTest, GrabToolAffectsVertices) {
    std::vector<float> single_vertex_data = {0.0f, 0.0f, 0.0f};
    std::vector<unsigned int> single_index_data = {0}; // Still need an index for Initialize to call RecalculateNormals
    mesh.Initialize(single_vertex_data, single_index_data);

    settings.radius = 2.0f;
    settings.strength = 1.0f;
    settings.falloff = Curve(); 
    settings.falloff.AddPoint({0.0f, 1.0f});
    settings.falloff.AddPoint({1.0f, 0.0f});
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    glm::vec2 mouseDelta(10.0f, 0.0f);
    
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

    auto initialVertex = mesh.GetVertices()[0];
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    auto finalVertex = mesh.GetVertices()[0];
    
    EXPECT_GT(finalVertex.x, initialVertex.x);
    EXPECT_NEAR(finalVertex.y, initialVertex.y, 1e-6f);
    EXPECT_NEAR(finalVertex.z, initialVertex.z, 1e-6f);
}

TEST_F(SculptingTest, GrabToolStrengthIsAppliedCorrectly) {
    std::vector<float> single_vertex_data = {0.0f, 0.0f, 0.0f};
    std::vector<unsigned int> single_index_data = {0}; // Still need an index
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    glm::vec2 mouseDelta(10.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

    // LOW strength
    mesh.Initialize(single_vertex_data, single_index_data);
    settings.strength = 0.1f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    float xAfterLowStrength = mesh.GetVertices()[0].x;

    // HIGH strength
    mesh.Initialize(single_vertex_data, single_index_data);
    settings.strength = 1.0f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    float xAfterHighStrength = mesh.GetVertices()[0].x;
    
    EXPECT_GT(xAfterHighStrength, xAfterLowStrength);
}

// --- SculptableMesh Advanced Tests ---

TEST_F(SculptingTest, SculptableMesh_InitializeEmpty) {
    mesh.Initialize({}, {});
    EXPECT_TRUE(mesh.GetVertices().empty());
    EXPECT_TRUE(mesh.GetIndices().empty());
    EXPECT_TRUE(mesh.GetNormals().empty());
    mesh.RecalculateNormals(); 
    EXPECT_TRUE(mesh.GetNormals().empty());
}

TEST_F(SculptingTest, SculptableMesh_InitializeSingleVertex) {
    // FIX: Provide a dummy index even for a single vertex to ensure RecalculateNormals
    // doesn't cause issues if it iterates over indices.
    // Or, ensure RecalculateNormals is robust for empty indices.
    // The current RecalculateNormals check for empty indices is:
    // `if (m_Vertices.empty() || m_Indices.empty()) return;`
    // This looks fine.
    
    mesh.Initialize({0.0f,0.0f,0.0f}, {}); // Single vertex, no indices
    EXPECT_EQ(mesh.GetVertices().size(), 1);
    EXPECT_TRUE(mesh.GetIndices().empty());
    EXPECT_EQ(mesh.GetNormals().size(), 1) << "Normals should be sized to match vertices.";
    
    // Normals should remain zero as no faces to calculate from
    // Use EXPECT_NEAR for float comparison of vec3 components
    EXPECT_NEAR(mesh.GetNormals()[0].x, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[0].y, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[0].z, 0.0f, 1e-6f);
}

TEST_F(SculptingTest, SculptableMesh_DegenerateTriangleNormals) {
    std::vector<float> degenerate_vertices = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        2.0f, 0.0f, 0.0f
    };
    std::vector<unsigned int> degenerate_indices = {0, 1, 2};
    mesh.Initialize(degenerate_vertices, degenerate_indices);
    
    EXPECT_NEAR(glm::length(mesh.GetNormals()[0]), 0.0f, 1e-5f);
    EXPECT_NEAR(glm::length(mesh.GetNormals()[1]), 0.0f, 1e-5f);
    EXPECT_NEAR(glm::length(mesh.GetNormals()[2]), 0.0f, 1e-5f);
}

TEST_F(SculptingTest, SculptableMesh_InvalidIndicesHandling) {
    std::vector<float> invalid_indexed_vertices = { 0.0f,0.0f,0.0f, 1.0f,0.0f,0.0f }; // Only 2 vertices
    std::vector<unsigned int> invalid_indices = {0, 1, 5}; // Index 5 is out of bounds
    mesh.Initialize(invalid_indexed_vertices, invalid_indices);
    
    EXPECT_EQ(mesh.GetVertices().size(), 2);
    EXPECT_EQ(mesh.GetNormals().size(), 2);
    EXPECT_NEAR(mesh.GetNormals()[0].x, 0.0f, 1e-6f); // Use EXPECT_NEAR for components
    EXPECT_NEAR(mesh.GetNormals()[0].y, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[0].z, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[1].x, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[1].y, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[1].z, 0.0f, 1e-6f);

    std::vector<float> invalid_indexed_vertices_2 = { 0.0f,0.0f,0.0f, 1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f };
    std::vector<unsigned int> invalid_indices_2 = {0, 1, 99};
    mesh.Initialize(invalid_indexed_vertices_2, invalid_indices_2);
    EXPECT_NEAR(mesh.GetNormals()[0].x, 0.0f, 1e-6f); // Use EXPECT_NEAR for components
    EXPECT_NEAR(mesh.GetNormals()[0].y, 0.0f, 1e-6f);
    EXPECT_NEAR(mesh.GetNormals()[0].z, 0.0f, 1e-6f);
}

TEST_F(SculptingTest, SculptableMesh_SerializationDeserialization) {
    std::vector<float> test_vertices = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    std::vector<unsigned int> test_indices = {0, 1, 2};
    mesh.Initialize(test_vertices, test_indices);
    
    mesh.m_Vertices[0] = glm::vec3(10.0f, 20.0f, 30.0f);
    mesh.RecalculateNormals();

    nlohmann::json j;
    mesh.Serialize(j);

    SculptableMesh loadedMesh;
    loadedMesh.Deserialize(j);

    ASSERT_EQ(loadedMesh.GetVertices().size(), mesh.GetVertices().size());
    EXPECT_EQ(loadedMesh.GetVertices()[0], glm::vec3(10.0f, 20.0f, 30.0f));
    EXPECT_EQ(loadedMesh.GetVertices()[1], glm::vec3(1.0f, 0.0f, 0.0f));
    ASSERT_EQ(loadedMesh.GetIndices().size(), mesh.GetIndices().size());
    ASSERT_EQ(loadedMesh.GetNormals().size(), mesh.GetNormals().size());

    SculptableMesh emptyLoadedMesh;
    nlohmann::json empty_j = {{"some_other_key", 123}};
    emptyLoadedMesh.Deserialize(empty_j);
    EXPECT_TRUE(emptyLoadedMesh.GetVertices().empty());
}