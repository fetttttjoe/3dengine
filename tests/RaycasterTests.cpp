#include "Core/Raycaster.h"
#include "Sculpting/SculptableMesh.h" // Still include SculptableMesh
#include "gtest/gtest.h"
#include <glm/gtc/matrix_transform.hpp>

TEST(RaycasterTest, HitTriangle) {
    glm::vec3 rayOrigin(0, 0, 5);
    glm::vec3 rayDirection(0, 0, -1);
    
    SculptableMesh mesh; // Use SculptableMesh, which implements IEditableMesh
    std::vector<float> vertices = {-1, -1, 0,  1, -1, 0,  0, 1, 0};
    std::vector<unsigned int> indices = {0, 1, 2};
    mesh.Initialize(vertices, indices);

    Raycaster::RaycastResult result;
    // Pass SculptableMesh by const reference to IEditableMesh
    bool hit = Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, glm::mat4(1.0f), result);

    ASSERT_TRUE(hit);
    EXPECT_NEAR(result.distance, 5.0f, 1e-6);
    EXPECT_NEAR(result.hitPoint.x, 0.0f, 1e-6);
    EXPECT_NEAR(result.hitPoint.y, 0.0f, 1e-6);
    EXPECT_NEAR(result.hitPoint.z, 0.0f, 1e-6);
}

TEST(RaycasterTest, MissTriangle) {
    glm::vec3 rayOrigin(3, 3, 5); // Start far away from the triangle
    glm::vec3 rayDirection(0, 0, -1);
    
    SculptableMesh mesh; // Use SculptableMesh, which implements IEditableMesh
    std::vector<float> vertices = {-1, -1, 0,  1, -1, 0,  0, 1, 0};
    std::vector<unsigned int> indices = {0, 1, 2};
    mesh.Initialize(vertices, indices);

    Raycaster::RaycastResult result;
    // Pass SculptableMesh by const reference to IEditableMesh
    bool hit = Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, glm::mat4(1.0f), result);

    ASSERT_FALSE(hit);
}

TEST(RaycasterTest, ParallelRayMissesTriangle) {
    glm::vec3 rayOrigin(0, 0, 5);
    // A ray direction in the XY plane, parallel to the triangle
    glm::vec3 rayDirection(1, 0, 0); 
    
    SculptableMesh mesh;
    std::vector<float> vertices = {-1, -1, 0,  1, -1, 0,  0, 1, 0};
    std::vector<unsigned int> indices = {0, 1, 2};
    mesh.Initialize(vertices, indices);

    Raycaster::RaycastResult result;
    bool hit = Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, glm::mat4(1.0f), result);

    ASSERT_FALSE(hit);
}

TEST(RaycasterTest, HitTranslatedTriangle) {
    glm::vec3 rayOrigin(5, 6, 5);
    glm::vec3 rayDirection(0, 0, -1);
    
    SculptableMesh mesh; // Use SculptableMesh, which implements IEditableMesh
    std::vector<float> vertices = {-1, -1, 0,  1, -1, 0,  0, 1, 0};
    std::vector<unsigned int> indices = {0, 1, 2};
    mesh.Initialize(vertices, indices);
    
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(5, 6, 0));

    Raycaster::RaycastResult result;
    // Pass SculptableMesh by const reference to IEditableMesh
    bool hit = Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, translation, result);

    ASSERT_TRUE(hit);
    EXPECT_NEAR(result.hitPoint.x, 0.0f, 1e-6);
    EXPECT_NEAR(result.hitPoint.y, 0.0f, 1e-6);
}