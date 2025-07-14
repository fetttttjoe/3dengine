#include "gtest/gtest.h"
#include "Sculpting/Tools/PushPullTool.h"
#include "Sculpting/Tools/SmoothTool.h"
#include "Sculpting/Tools/GrabTool.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/UI/BrushSettings.h"
#include "Core/Camera.h" // For glm::lookAt, glm::ortho
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
  int viewportWidth = 800; // Define viewport dimensions for tests
  int viewportHeight = 600; // Define viewport dimensions for tests
};

// --- Push/Pull Tool Tests ---

TEST_F(SculptingTest, PushPullToolAffectsCorrectVertices) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  settings.mode = SculptMode::Pull;
  // Pass correct viewport dimensions
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
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
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
  float zChangePull = mesh.GetVertices()[0].z - verticesBeforePull[0].z;
  EXPECT_GT(zChangePull, 0.0f);
  // PUSH
  mesh.Initialize(vertices_data, indices_data);
  auto verticesBeforePush = mesh.GetVertices();
  settings.mode = SculptMode::Push;
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
  float zChangePush = mesh.GetVertices()[0].z - verticesBeforePush[0].z;
  EXPECT_LT(zChangePush, 0.0f);
}

TEST_F(SculptingTest, PushPullToolStrengthIsAppliedCorrectly) {
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    settings.mode = SculptMode::Pull;

    // --- Test with LOW strength ---
    mesh.Initialize(vertices_data, indices_data);
    settings.strength = 0.1f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
    float zChangeLowStrength = mesh.GetVertices()[0].z;

    // --- Test with HIGH strength ---
    mesh.Initialize(vertices_data, indices_data);
    settings.strength = 1.0f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
    float zChangeHighStrength = mesh.GetVertices()[0].z;

    EXPECT_GT(zChangeHighStrength, zChangeLowStrength);
}

// --- Smooth Tool Tests ---

TEST_F(SculptingTest, SmoothToolAveragesVertices) {
    std::vector<float> smooth_vertices = {-1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 2.0f, 0.0f};
    std::vector<unsigned int> smooth_indices = {0, 2, 1};
    mesh.Initialize(smooth_vertices, smooth_indices);
    
    settings.radius = 2.0f;
    settings.strength = 1.0f;
    glm::vec3 hitPoint(0.0f, 1.0f, 0.0f);

    float initial_Y_height = mesh.GetVertices()[2].y;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
    float final_Y_height = mesh.GetVertices()[2].y;
    
    EXPECT_LT(final_Y_height, initial_Y_height);
    EXPECT_GT(final_Y_height, 0.0f);
}

TEST_F(SculptingTest, SmoothToolStrengthIsAppliedCorrectly) {
    std::vector<float> smooth_vertices = {-1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 2.0f, 0.0f};
    std::vector<unsigned int> smooth_indices = {0, 2, 1};
    glm::vec3 hitPoint(0.0f, 1.0f, 0.0f);

    // LOW strength
    mesh.Initialize(smooth_vertices, smooth_indices);
    settings.strength = 0.1f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
    float heightAfterLowStrength = mesh.GetVertices()[2].y;

    // HIGH strength
    mesh.Initialize(smooth_vertices, smooth_indices);
    settings.strength = 1.0f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, viewportWidth, viewportHeight);
    float heightAfterHighStrength = mesh.GetVertices()[2].y;
    
    EXPECT_LT(heightAfterHighStrength, heightAfterLowStrength);
}

// --- Grab Tool Tests ---

TEST_F(SculptingTest, GrabToolAffectsVertices) {
    // SculptableMesh::Initialize expects std::vector<float> for vertices
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
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, viewportWidth, viewportHeight);
    auto finalVertex = mesh.GetVertices()[0];
    
    EXPECT_GT(finalVertex.x, initialVertex.x);
    EXPECT_NEAR(finalVertex.y, initialVertex.y, 1e-6f);
    EXPECT_NEAR(finalVertex.z, initialVertex.z, 1e-6f);
}

TEST_F(SculptingTest, GrabToolStrengthIsAppliedCorrectly) {
    // SculptableMesh::Initialize expects std::vector<float> for vertices
    std::vector<float> single_vertex_data = {0.0f, 0.0f, 0.0f};
    std::vector<unsigned int> single_index_data = {0}; // Still need an index
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    glm::vec2 mouseDelta(10.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

    // LOW strength
    mesh.Initialize(single_vertex_data, single_index_data);
    settings.strength = 0.1f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, viewportWidth, viewportHeight);
    float xAfterLowStrength = mesh.GetVertices()[0].x;

    // HIGH strength
    mesh.Initialize(single_vertex_data, single_index_data);
    settings.strength = 1.0f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, viewportWidth, viewportHeight);
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
    // SculptableMesh::Initialize expects std::vector<float> for vertices
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
    std::vector<float> test_vertices_float = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}; // Use float vector
    std::vector<unsigned int> test_indices = {0, 1, 2};
    mesh.Initialize(test_vertices_float, test_indices); // Pass float vector
    
    mesh.GetVertices()[0] = glm::vec3(10.0f, 20.0f, 30.0f); // Access via GetVertices()
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

TEST_F(SculptingTest, SculptableMesh_ExtrudeFaces_SingleFace) {
    // Start with a single triangle
    std::vector<float> initial_vertices = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    std::vector<unsigned int> initial_indices = {0, 1, 2};
    mesh.Initialize(initial_vertices, initial_indices);
    mesh.RecalculateNormals();

    // Store initial state
    size_t initialVertexCount = mesh.GetVertices().size(); // 3
    size_t initialIndexCount = mesh.GetIndices().size();   // 3

    std::unordered_set<uint32_t> facesToExtrude = {0}; // Extrude the first (and only) face
    float extrudeDistance = 1.0f;

    bool extruded = mesh.ExtrudeFaces(facesToExtrude, extrudeDistance);
    ASSERT_TRUE(extruded);

    // Expected: 3 new vertices + 3 original vertices (in m_Vertices, effectively the old ones) = 6 vertices total in vector
    // 1 original face (now extruded) + 3 new side quads (6 triangles) = 1 + 6 = 7 faces
    // Indices: 3 (for new top face) + 18 (for sides) = 21 indices
    EXPECT_EQ(mesh.GetVertices().size(), initialVertexCount + 3) << "Expected 3 new vertices to be added.";
    EXPECT_EQ(mesh.GetIndices().size(), initialIndexCount + 18) << "Expected 18 new indices for side faces.";

    // Verify properties of the extruded mesh (e.g., normal change on original vertices)
    glm::vec3 extrudedVertexPos = mesh.GetVertices()[mesh.GetIndices()[0]]; // Get position of the *new* vertex, pointed by original first index
    EXPECT_GT(extrudedVertexPos.z, 0.0f); // Assuming original was at Z=0 and normal is +Z
}

TEST_F(SculptingTest, SculptableMesh_WeldVertices) {
    // Start with two vertices, make them close for welding
    std::vector<float> initial_vertices = {0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    std::vector<unsigned int> initial_indices = {0, 1, 2}; // Forms a triangle
    mesh.Initialize(initial_vertices, initial_indices);
    mesh.RecalculateNormals();

    // Weld vertex 1 into vertex 0
    std::unordered_set<uint32_t> verticesToWeld = {0, 1};
    glm::vec3 weldPoint = glm::vec3(0.05f, 0.0f, 0.0f); // Midpoint of 0 and 1

    bool welded = mesh.WeldVertices(verticesToWeld, weldPoint);
    ASSERT_TRUE(welded);

    // Vertex 0 should be at the weldPoint
    EXPECT_EQ(mesh.GetVertices()[0], weldPoint);
    // Vertex 1 should be untouched in the m_Vertices vector, but no index should point to it anymore
    EXPECT_EQ(mesh.GetVertices()[1], glm::vec3(0.1f, 0.0f, 0.0f)); 

    // All indices that pointed to vertex 1 should now point to vertex 0
    EXPECT_EQ(mesh.GetIndices()[0], 0); // Was 0
    EXPECT_EQ(mesh.GetIndices()[1], 0); // Was 1, now points to 0
    EXPECT_EQ(mesh.GetIndices()[2], 2); // Remains 2

    // Test welding with less than 2 vertices
    mesh.Initialize(initial_vertices, initial_indices);
    std::unordered_set<uint32_t> singleVertex = {0};
    welded = mesh.WeldVertices(singleVertex, glm::vec3(0.0f));
    ASSERT_FALSE(welded) << "Should not weld with less than 2 vertices.";
}

TEST_F(SculptingTest, SculptableMesh_ExtrudeFaces_NoFacesSelected) {
    // Test extruding with no faces selected
    std::vector<float> initial_vertices = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    std::vector<unsigned int> initial_indices = {0, 1, 2};
    mesh.Initialize(initial_vertices, initial_indices);
    size_t initialVertexCount = mesh.GetVertices().size();
    size_t initialIndexCount = mesh.GetIndices().size();

    std::unordered_set<uint32_t> emptyFaces;
    bool extruded = mesh.ExtrudeFaces(emptyFaces, 1.0f);
    ASSERT_FALSE(extruded) << "Should not extrude if no faces are selected.";
    EXPECT_EQ(mesh.GetVertices().size(), initialVertexCount);
    EXPECT_EQ(mesh.GetIndices().size(), initialIndexCount);
}

TEST_F(SculptingTest, SculptableMesh_WeldVertices_MultipleVertices) {
    std::vector<float> initial_vertices_multi = {
        0.0f, 0.0f, 0.0f, // 0
        0.1f, 0.0f, 0.0f, // 1
        0.2f, 0.0f, 0.0f, // 2
        1.0f, 1.0f, 0.0f  // 3
    };
    std::vector<unsigned int> initial_indices_multi = {0, 1, 3,  1, 2, 3}; // Two triangles sharing edge 1-3
    mesh.Initialize(initial_vertices_multi, initial_indices_multi);
    mesh.RecalculateNormals();

    // Weld vertices 0, 1, and 2 into vertex 0
    std::unordered_set<uint32_t> verticesToWeld = {0, 1, 2};
    glm::vec3 weldPoint = glm::vec3(0.1f, 0.0f, 0.0f); // Average of 0, 1, 2

    bool welded = mesh.WeldVertices(verticesToWeld, weldPoint);
    ASSERT_TRUE(welded);

    EXPECT_EQ(mesh.GetVertices()[0], weldPoint); // Target vertex should be at weld point

    // Check indices: all references to 1 and 2 should now be 0
    EXPECT_EQ(mesh.GetIndices()[0], 0); // Was 0
    EXPECT_EQ(mesh.GetIndices()[1], 0); // Was 1, now 0
    EXPECT_EQ(mesh.GetIndices()[2], 3); // Was 3
    EXPECT_EQ(mesh.GetIndices()[3], 0); // Was 1, now 0
    EXPECT_EQ(mesh.GetIndices()[4], 0); // Was 2, now 0
    EXPECT_EQ(mesh.GetIndices()[5], 3); // Was 3

    // The mesh should now be degenerate or have collapsed faces, but the indices are correctly remapped.
}