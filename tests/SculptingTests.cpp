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
    vertices = {-1.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 0.0f,
                1.0f,  -1.0f, 0.0f, 1.0f,  1.0f,  0.0f};
    indices = {0, 1, 2, 0, 2, 3};
    mesh.Initialize(vertices, indices);
    
    settings.radius = 1.5f;
    settings.strength = 0.5f;
  }

  SculptableMesh mesh;
  PushPullTool pushPullTool;
  SmoothTool smoothTool;
  GrabTool grabTool;
  BrushSettings settings;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  
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
  mesh.Initialize(vertices, indices);
  auto verticesBeforePull = mesh.GetVertices();
  settings.mode = SculptMode::Pull;
  pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
  float zChangePull = mesh.GetVertices()[0].z - verticesBeforePull[0].z;
  EXPECT_GT(zChangePull, 0.0f);
  // PUSH
  mesh.Initialize(vertices, indices);
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
    mesh.Initialize(vertices, indices);
    settings.strength = 0.1f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
    float zChangeLowStrength = mesh.GetVertices()[0].z;

    // --- Test with HIGH strength ---
    mesh.Initialize(vertices, indices);
    settings.strength = 1.0f;
    pushPullTool.Apply(mesh, hitPoint, dummyRayDirection, dummyMouseDelta, settings, dummyMatrix, dummyMatrix, 0, 0);
    float zChangeHighStrength = mesh.GetVertices()[0].z;

    // Assert that the high-strength application moved the vertex MORE than the low-strength one.
    EXPECT_GT(zChangeHighStrength, zChangeLowStrength);
}

// --- Smooth Tool Tests ---

TEST_F(SculptingTest, SmoothToolAveragesVertices) {
    vertices = {-1, 0, 0,  1, 0, 0,  0, 1, 0,  0, 0, -1};
    indices = {0, 2, 1, 0, 3, 2, 1, 2, 3};
    mesh.Initialize(vertices, indices);
    settings.radius = 2.0f;
    settings.strength = 1.0f;
    glm::vec3 hitPoint(0.0f, 0.5f, 0.0f);
    float initial_Y_height = mesh.GetVertices()[2].y;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float final_Y_height = mesh.GetVertices()[2].y;
    EXPECT_LT(final_Y_height, initial_Y_height);
}

TEST_F(SculptingTest, SmoothToolStrengthIsAppliedCorrectly) {
    vertices = {-1, 0, 0,  1, 0, 0,  0, 2, 0};
    indices = {0, 2, 1};
    glm::vec3 hitPoint(0.0f, 1.0f, 0.0f);
    // LOW strength
    mesh.Initialize(vertices, indices);
    settings.strength = 0.1f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float heightAfterLowStrength = mesh.GetVertices()[2].y;
    // HIGH strength
    mesh.Initialize(vertices, indices);
    settings.strength = 1.0f;
    smoothTool.Apply(mesh, hitPoint, {0,-1,0}, {0,0}, settings, dummyMatrix, dummyMatrix, 800, 600);
    float heightAfterHighStrength = mesh.GetVertices()[2].y;
    // Assert
    EXPECT_LT(heightAfterHighStrength, heightAfterLowStrength);
}

// --- Grab Tool Tests ---

TEST_F(SculptingTest, GrabToolAffectsVertices) {
    vertices = {0.0f, 0.0f, 0.0f};
    indices = {0};
    mesh.Initialize(vertices, indices);
    settings.radius = 2.0f;
    settings.strength = 1.0f;
    settings.falloff = Curve(); 
    settings.falloff.AddPoint({0.0f, 1.0f});
    settings.falloff.AddPoint({1.0f, 0.0f});
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    glm::vec2 mouseDelta(10.0f, 0.0f);
    auto initialVertex = mesh.GetVertices()[0];
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    auto finalVertex = mesh.GetVertices()[0];
    EXPECT_GT(finalVertex.x, initialVertex.x);
}

TEST_F(SculptingTest, GrabToolStrengthIsAppliedCorrectly) {
    vertices = {0.0f, 0.0f, 0.0f};
    indices = {0};
    glm::vec3 hitPoint(0.0f, 0.0f, 0.0f);
    glm::vec2 mouseDelta(10.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

    // LOW strength
    mesh.Initialize(vertices, indices);
    settings.strength = 0.1f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    float xAfterLowStrength = mesh.GetVertices()[0].x;

    // HIGH strength
    mesh.Initialize(vertices, indices);
    settings.strength = 1.0f;
    grabTool.Apply(mesh, hitPoint, {0,0,-1}, mouseDelta, settings, viewMatrix, projectionMatrix, 800, 600);
    float xAfterHighStrength = mesh.GetVertices()[0].x;
    
    // Assert
    EXPECT_GT(xAfterHighStrength, xAfterLowStrength);
}