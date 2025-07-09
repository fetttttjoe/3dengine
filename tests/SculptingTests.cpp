#include "Sculpting/Tools/PushPullTool.h"
#include "Sculpting/SculptableMesh.h"
#include "gtest/gtest.h"

class SculptingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a simple quad mesh for testing
    vertices = {-1.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 0.0f,
                1.0f,  -1.0f, 0.0f, 1.0f,  1.0f,  0.0f};
    indices = {0, 1, 2, 0, 2, 3};
    mesh.Initialize(vertices, indices);
  }

  SculptableMesh mesh;
  PushPullTool tool;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

// --- Positive Tests ---


TEST_F(SculptingTest, ToolAffectsCorrectVertices) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  float radius = 1.5f;   // A radius that covers all vertices
  float strength = 0.5f;

  tool.Apply(mesh, hitPoint, radius, strength, SculptMode::Pull);

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
  float radius = 0.5f;                  // A small radius
  float strength = 0.5f;

  tool.Apply(mesh, hitPoint, radius, strength, SculptMode::Pull);

  auto finalVertices = mesh.GetVertices();

  // No vertices should have moved
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_EQ(initialVertices[i], finalVertices[i]);
  }
}

TEST_F(SculptingTest, PushAndPullWorkInOppositeDirections) {
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  float radius = 1.5f;
  float strength = 0.5f;

  // PULL
  mesh.Initialize(vertices, indices);  // Reset mesh
  auto verticesBeforePull = mesh.GetVertices();
  tool.Apply(mesh, hitPoint, radius, strength, SculptMode::Pull);
  auto verticesAfterPull = mesh.GetVertices();
  float zChangePull = verticesAfterPull[0].z - verticesBeforePull[0].z;
  EXPECT_GT(zChangePull, 0.0f);

  // PUSH
  mesh.Initialize(vertices, indices);  // Reset mesh
  auto verticesBeforePush = mesh.GetVertices();
  tool.Apply(mesh, hitPoint, radius, strength, SculptMode::Push);
  auto verticesAfterPush = mesh.GetVertices();
  float zChangePush = verticesAfterPush[0].z - verticesBeforePush[0].z;
  EXPECT_LT(zChangePush, 0.0f);
}

// --- Edge Case Tests ---

TEST_F(SculptingTest, ZeroStrengthHasNoEffect) {
  auto initialVertices = mesh.GetVertices();
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  float radius = 1.5f;
  float strength = 0.0f;  // Zero strength

  tool.Apply(mesh, hitPoint, radius, strength, SculptMode::Pull);

  auto finalVertices = mesh.GetVertices();

  // No vertices should have moved
  for (size_t i = 0; i < initialVertices.size(); ++i) {
    EXPECT_EQ(initialVertices[i], finalVertices[i]);
  }
}

TEST_F(SculptingTest, SerializationAndDeserialization) {
  glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
  tool.Apply(mesh, hitPoint, 1.5f, 0.5f, SculptMode::Pull);
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
