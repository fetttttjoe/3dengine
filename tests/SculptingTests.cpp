#include "Sculpting/Tools/PushPullTool.h"
#include "Sculpting/SculptableMesh.h"
#include "gtest/gtest.h"
#include "Core/UI/BrushSettings.h"

class SculptingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a simple quad mesh for testing
    vertices = {-1.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 0.0f,
                1.0f,  -1.0f, 0.0f, 1.0f,  1.0f,  0.0f};
    indices = {0, 1, 2, 0, 2, 3};
    mesh.Initialize(vertices, indices);
    
    // Default brush settings for tests
    settings.radius = 1.5f;
    settings.strength = 0.5f;
  }

  SculptableMesh mesh;
  PushPullTool tool;
  BrushSettings settings;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  
  // Dummy parameters for the new Apply signature
  glm::vec3 dummyRayDirection = {0,0,-1};
  glm::vec2 dummyMouseDelta = {0,0};
  glm::mat4 dummyMatrix = glm::mat4(1.0f);
};

// --- Positive Tests ---

TEST_F(SculptingTest, ToolAffectsCorrectVertices) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  settings.mode = SculptMode::Pull;

  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);

  auto finalVertices = mesh.GetVertices();

  // All vertices should have moved because the brush covers them all.
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_NE(initialVertices[i], finalVertices[i]);
    // They should move along the Z axis (their normal)
    EXPECT_GT(finalVertices[i].z, initialVertices[i].z);
  }
}

TEST_F(SculptingTest, ToolDoesNotAffectVerticesOutsideRadius) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(2.0f, 2.0f, 0.0f);  // A point far away from the mesh
  settings.radius = 0.5f;
  settings.mode = SculptMode::Pull;

  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);

  auto finalVertices = mesh.GetVertices();

  // No vertices should have moved
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_EQ(initialVertices[i], finalVertices[i]);
  }
}

TEST_F(SculptingTest, PushAndPullWorkInOppositeDirections) {
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);

  // PULL
  mesh.Initialize(vertices, indices);  // Reset mesh
  auto verticesBeforePull = mesh.GetVertices();
  settings.mode = SculptMode::Pull;
  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  auto verticesAfterPull = mesh.GetVertices();
  float zChangePull = verticesAfterPull[0].z - verticesBeforePull[0].z;
  EXPECT_GT(zChangePull, 0.0f);

  // PUSH
  mesh.Initialize(vertices, indices);  // Reset mesh
  auto verticesBeforePush = mesh.GetVertices();
  settings.mode = SculptMode::Push;
  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  auto verticesAfterPush = mesh.GetVertices();
  float zChangePush = verticesAfterPush[0].z - verticesBeforePush[0].z;
  EXPECT_LT(zChangePush, 0.0f);
}

// --- Edge Case Tests ---

TEST_F(SculptingTest, ZeroStrengthHasNoEffect) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  settings.strength = 0.0f;  // Zero strength
  settings.mode = SculptMode::Pull;

  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);

  auto finalVertices = mesh.GetVertices();

  // No vertices should have moved
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_EQ(initialVertices[i], finalVertices[i]);
  }
}

TEST_F(SculptingTest, SerializationAndDeserialization) {
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  settings.mode = SculptMode::Pull;
  tool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  auto verticesBeforeSave = mesh.GetVertices();

  nlohmann::json j;
  mesh.Serialize(j);

  SculptableMesh newMesh;
  newMesh.Initialize(vertices, indices);  // Start with original data
  newMesh.Deserialize(j);
  auto verticesAfterLoad = newMesh.GetVertices();

  ASSERT_EQ(verticesBeforeSave.size(), verticesAfterLoad.size());
  for (size_t i = 0; i < verticesBeforeSave.size(); ++i) {
    EXPECT_EQ(verticesBeforeSave[i], verticesAfterLoad[i]);
  }
}