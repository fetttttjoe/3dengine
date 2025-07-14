#include "gtest/gtest.h"
#include "Scene/TransformGizmo.h"
#include "Core/Camera.h"
#include "Scene/Objects/Sphere.h"
#include "Core/PropertyNames.h"
#include "Core/Application.h" // Required to access the global application instance

// This is now a simple test, not a test fixture.
TEST(TransformGizmoTest, UpdateCorrectlyModifiesTarget) {
    // 1. Setup
    auto sphere = std::make_unique<Sphere>();

    // Get the valid window handle from the single, global Application instance.
    // This is the correct way to interact with the engine from a test.
    GLFWwindow* testWindow = Application::Get().GetWindow();
    ASSERT_NE(testWindow, nullptr); // Ensure the window is valid
    
    auto camera = std::make_unique<Camera>(testWindow);

    TransformGizmo gizmo;
    gizmo.SetTarget(sphere.get());

    uint32_t handleId = 1000000;
    gizmo.SetActiveHandle(handleId);
    
    GizmoHandle* handle = gizmo.GetHandleByID(handleId);
    ASSERT_NE(handle, nullptr);
    EXPECT_EQ(handle->propertyName, PropertyNames::Scale);

    glm::vec2 mouseDelta(10.0f, 0.0f);
    glm::vec3 initialScale = sphere->GetScale();

    int windowWidth = 800; // Define test window width
    int windowHeight = 600; // Define test window height

    // 2. Act
    // Pass window dimensions to Update
    gizmo.Update(*camera, mouseDelta, true, windowWidth, windowHeight);
    glm::vec3 finalScale = sphere->GetScale();

    // 3. Assert
    EXPECT_GT(finalScale.x, initialScale.x);
    EXPECT_EQ(finalScale.y, initialScale.y);
    EXPECT_EQ(finalScale.z, initialScale.z);
}

TEST(TransformGizmoTest, UpdateCorrectlyModifiesTargetAlongPrincipalAxes) {
    auto sphere = std::make_unique<Sphere>();
    GLFWwindow* testWindow = Application::Get().GetWindow();
    auto camera = std::make_unique<Camera>(testWindow);
    TransformGizmo gizmo;
    gizmo.SetTarget(sphere.get());

    // Test X-Axis Handle
    gizmo.SetActiveHandle(1000000); // Assumes X handle is first
    glm::vec3 initialScale = sphere->GetScale();
    gizmo.Update(*camera, glm::vec2(10.0f, 0.0f), true, 800, 600);
    glm::vec3 finalScaleX = sphere->GetScale();
    EXPECT_GT(finalScaleX.x, initialScale.x);
    EXPECT_EQ(finalScaleX.y, initialScale.y);
    EXPECT_EQ(finalScaleX.z, initialScale.z);

    // Test Y-Axis Handle
    sphere->SetScale(initialScale); // Reset scale
    gizmo.SetActiveHandle(1000002); // Assumes Y handle is second
    gizmo.Update(*camera, glm::vec2(0.0f, -10.0f), true, 800, 600); // Negative Y on screen is up
    glm::vec3 finalScaleY = sphere->GetScale();
    EXPECT_EQ(finalScaleY.x, initialScale.x);
    EXPECT_GT(finalScaleY.y, initialScale.y);
    EXPECT_EQ(finalScaleY.z, initialScale.z);
}