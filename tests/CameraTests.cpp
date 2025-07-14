#include "gtest/gtest.h"
#include "Core/Camera.h"
#include "Core/Log.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

#include "Core/Application.h" // Needed to access Application::Get()
#include "TestMocks.h" // Needed for MockCamera::s_CameraPosition etc.
#include "Core/MathHelpers.h" // For MathHelpers::ToString, and direct calls

class CameraTest : public ::testing::Test {
protected:
    GLFWwindow* testWindow = nullptr;
    Camera* cameraPtr = nullptr;

    int windowWidth = 800;
    int windowHeight = 600;

    void SetUp() override {
        testWindow = Application::Get().GetWindow();
        ASSERT_NE(testWindow, nullptr) << "GLFW window is not valid in Application. Make sure Application is properly initialized before running Camera tests.";
        
        cameraPtr = new Camera(testWindow, glm::vec3(0.0f, 2.0f, 8.0f));
        ASSERT_NE(cameraPtr, nullptr);

        cameraPtr->ResetToDefault();
        cameraPtr->SetAspectRatio(static_cast<float>(windowWidth) / windowHeight);
    }

    void TearDown() override {
        if (cameraPtr) {
            delete cameraPtr;
            cameraPtr = nullptr;
        }
        MockCamera::s_CameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
        MockCamera::s_ViewMatrix = glm::lookAt(MockCamera::s_CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        MockCamera::s_ProjectionMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    }
};

TEST_F(CameraTest, WorldToScreen_ScreenToWorldPoint_Roundtrip) {
    glm::mat4 viewProj = cameraPtr->GetProjectionMatrix() * cameraPtr->GetViewMatrix();
    glm::mat4 invViewProj = glm::inverse(viewProj);

    glm::vec3 testWorldPoint(1.0f, 2.0f, 0.0f);

    // Use MathHelpers::WorldToScreen directly
    glm::vec2 screenPos = MathHelpers::WorldToScreen(testWorldPoint, viewProj, windowWidth, windowHeight);
    Log::Debug("Test Point: ", MathHelpers::ToString(testWorldPoint), " -> Screen: ", MathHelpers::ToString(screenPos));

    glm::vec4 clipPos = viewProj * glm::vec4(testWorldPoint, 1.0f);
    float originalNdcZ = clipPos.z / clipPos.w;

    // Use MathHelpers::ScreenToWorldPoint directly
    glm::vec3 reconstructedWorldPoint = MathHelpers::ScreenToWorldPoint(screenPos, originalNdcZ, invViewProj, windowWidth, windowHeight);
    Log::Debug("Reconstructed: ", MathHelpers::ToString(reconstructedWorldPoint));

    EXPECT_NEAR(reconstructedWorldPoint.x, testWorldPoint.x, 1e-4f);
    EXPECT_NEAR(reconstructedWorldPoint.y, testWorldPoint.y, 1e-4f);
    EXPECT_NEAR(reconstructedWorldPoint.z, testWorldPoint.z, 1e-4f);
}

TEST_F(CameraTest, WorldToScreen_OriginCenter) {
    Camera tempCamera(testWindow, glm::vec3(0.0f, 0.0f, 5.0f));
    tempCamera.SetYaw(-90.0f);
    tempCamera.SetPitch(0.0f);
    tempCamera.SetAspectRatio(static_cast<float>(windowWidth) / windowHeight);

    glm::mat4 viewProj = tempCamera.GetProjectionMatrix() * tempCamera.GetViewMatrix();
    glm::vec3 worldOrigin(0.0f, 0.0f, 0.0f);
    // Use MathHelpers::WorldToScreen directly
    glm::vec2 screenCenter = MathHelpers::WorldToScreen(worldOrigin, viewProj, windowWidth, windowHeight);
    
    EXPECT_NEAR(screenCenter.x, windowWidth / 2.0f, 1e-3f);
    EXPECT_NEAR(screenCenter.y, windowHeight / 2.0f, 1e-3f);
}

TEST_F(CameraTest, ScreenToWorldPoint_DifferentDepths) {
    glm::mat4 viewProj = cameraPtr->GetProjectionMatrix() * cameraPtr->GetViewMatrix();
    glm::mat4 invViewProj = glm::inverse(viewProj);
    glm::vec2 screenCenter(windowWidth / 2.0f, windowHeight / 2.0f);

    // Use MathHelpers::ScreenToWorldPoint directly
    glm::vec3 worldPointNear = MathHelpers::ScreenToWorldPoint(screenCenter, -1.0f, invViewProj, windowWidth, windowHeight);
    glm::vec3 worldPointFar = MathHelpers::ScreenToWorldPoint(screenCenter, 1.0f, invViewProj, windowWidth, windowHeight);

    EXPECT_LT(glm::distance(worldPointNear, cameraPtr->GetPosition()), glm::distance(worldPointFar, cameraPtr->GetPosition()));
    EXPECT_NE(worldPointNear, worldPointFar);
}

TEST_F(CameraTest, ScreenToWorldRay_IsNormalized) {
    glm::vec2 screenPos(100, 100);
    // Use MathHelpers::ScreenToWorldRay directly, passing camera's matrices
    glm::vec3 rayDirection = MathHelpers::ScreenToWorldRay(screenPos, cameraPtr->GetProjectionMatrix(), cameraPtr->GetViewMatrix(), windowWidth, windowHeight);
    EXPECT_NEAR(glm::length(rayDirection), 1.0f, 1e-6f) << "Ray direction should be normalized.";
}

TEST_F(CameraTest, ScreenToWorldRay_PointsFromCameraPosition) {
    Camera tempCamera(testWindow, glm::vec3(0.0f, 0.0f, 5.0f));
    tempCamera.SetYaw(-90.0f);
    tempCamera.SetPitch(0.0f);
    tempCamera.SetAspectRatio(static_cast<float>(windowWidth) / windowHeight);

    glm::vec2 screenCenterPos(windowWidth / 2.0f, windowHeight / 2.0f);
    // Use MathHelpers::ScreenToWorldRay directly, passing camera's matrices
    glm::vec3 rayDirection = MathHelpers::ScreenToWorldRay(screenCenterPos, tempCamera.GetProjectionMatrix(), tempCamera.GetViewMatrix(), windowWidth, windowHeight);
    
    glm::vec3 expectedRayDirectionAtCenter = glm::normalize(glm::vec3(0,0,0) - tempCamera.GetPosition());
    EXPECT_NEAR(rayDirection.x, expectedRayDirectionAtCenter.x, 1e-4f);
    EXPECT_NEAR(rayDirection.y, expectedRayDirectionAtCenter.y, 1e-4f);
    EXPECT_NEAR(rayDirection.z, expectedRayDirectionAtCenter.z, 1e-4f);
}

TEST_F(CameraTest, ProcessMouseScroll_ChangesPosition) {
    glm::vec3 initialPosition = cameraPtr->GetPosition();
    float yOffset = 1.0f;
    
    cameraPtr->ProcessMouseScroll(yOffset);

    EXPECT_NE(cameraPtr->GetPosition(), initialPosition) << "Camera position should change after scroll.";
}

TEST_F(CameraTest, SetAspectRatio_UpdatesProjection) {
    // Ensure initial aspect ratio is distinct from the test target
    float initialTestAspectRatio = 16.0f / 9.0f; // Default for your camera construction
    float newTestAspectRatio = 1.0f; // A distinct 1:1 aspect ratio

    // First, ensure the camera is at initialTestAspectRatio
    cameraPtr->SetAspectRatio(initialTestAspectRatio);
    glm::mat4 projectionBeforeChange = cameraPtr->GetProjectionMatrix();

    // Now, change to the new aspect ratio
    cameraPtr->SetAspectRatio(newTestAspectRatio);
    glm::mat4 projectionAfterChange = cameraPtr->GetProjectionMatrix();
    
    // FIX: Check the [0][0] element, which is aspect ratio dependent.
    // The [1][1] element depends only on FOV.
    EXPECT_NE(projectionBeforeChange[0][0], projectionAfterChange[0][0]) << "Projection matrix (0,0) element should change after aspect ratio update.";
}

TEST_F(CameraTest, ResetToDefault_ReturnsToInitialState) {
    cameraPtr->ProcessMouseScroll(10.0f);
    cameraPtr->SetAspectRatio(1.0f);

    cameraPtr->ResetToDefault();

    EXPECT_EQ(cameraPtr->GetPosition(), glm::vec3(0.0f, 2.0f, 8.0f)) << "Camera position should reset to default.";
}

// New tests for MathHelpers utility functions
TEST_F(CameraTest, MathHelpers_WorldToScreen_KnownPoints) {
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 100.0f);
    glm::mat4 viewProj = proj * view;

    // Point at origin should be near center of screen
    glm::vec2 screenCenter = MathHelpers::WorldToScreen(glm::vec3(0, 0, 0), viewProj, windowWidth, windowHeight);
    EXPECT_NEAR(screenCenter.x, windowWidth / 2.0f, 1.0f);
    EXPECT_NEAR(screenCenter.y, windowHeight / 2.0f, 1.0f);

    // Point to the right
    glm::vec2 screenRight = MathHelpers::WorldToScreen(glm::vec3(1, 0, 0), viewProj, windowWidth, windowHeight);
    EXPECT_GT(screenRight.x, screenCenter.x);

    // Point to the left
    glm::vec2 screenLeft = MathHelpers::WorldToScreen(glm::vec3(-1, 0, 0), viewProj, windowWidth, windowHeight);
    EXPECT_LT(screenLeft.x, screenCenter.x);
}

TEST_F(CameraTest, MathHelpers_ScreenToWorldPoint_KnownPoints) {
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 100.0f);
    glm::mat4 invViewProj = glm::inverse(proj * view);

    // Screen center, near plane (NDC Z = -1.0)
    glm::vec2 screenCenter(windowWidth / 2.0f, windowHeight / 2.0f);
    glm::vec3 worldPointNear = MathHelpers::ScreenToWorldPoint(screenCenter, -1.0f, invViewProj, windowWidth, windowHeight);
    // The exact value depends on the projection matrix's near plane and FOV, but it should be on the Z-axis in world space.
    EXPECT_NEAR(worldPointNear.x, 0.0f, 1e-4f);
    EXPECT_NEAR(worldPointNear.y, 0.0f, 1e-4f);
    EXPECT_LT(worldPointNear.z, cameraPtr->GetPosition().z); // Should be in front of camera

    // Screen center, far plane (NDC Z = 1.0)
    glm::vec3 worldPointFar = MathHelpers::ScreenToWorldPoint(screenCenter, 1.0f, invViewProj, windowWidth, windowHeight);
    EXPECT_NEAR(worldPointFar.x, 0.0f, 1e-4f);
    EXPECT_NEAR(worldPointFar.y, 0.0f, 1e-4f);
    EXPECT_LT(worldPointFar.z, worldPointNear.z); // Should be further away
}

TEST_F(CameraTest, MathHelpers_ScreenToWorldRay_Accuracy) {
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 100.0f);

    // Ray from center of screen should point directly at (0,0,0) (camera looks at origin)
    glm::vec2 screenCenter(windowWidth / 2.0f, windowHeight / 2.0f);
    glm::vec3 rayDirection = MathHelpers::ScreenToWorldRay(screenCenter, proj, view, windowWidth, windowHeight);
    
    // Expected direction from (0,0,5) to (0,0,0) is (0,0,-1)
    EXPECT_NEAR(rayDirection.x, 0.0f, 1e-4f);
    EXPECT_NEAR(rayDirection.y, 0.0f, 1e-4f);
    EXPECT_NEAR(rayDirection.z, -1.0f, 1e-4f);
    EXPECT_NEAR(glm::length(rayDirection), 1.0f, 1e-6f);
}