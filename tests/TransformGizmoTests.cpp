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

    // 2. Act
    gizmo.Update(*camera, mouseDelta, true, 800, 600);
    glm::vec3 finalScale = sphere->GetScale();

    // 3. Assert
    EXPECT_GT(finalScale.x, initialScale.x);
    EXPECT_EQ(finalScale.y, initialScale.y);
    EXPECT_EQ(finalScale.z, initialScale.z);
}