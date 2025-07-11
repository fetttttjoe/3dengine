#include "gtest/gtest.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Core/Log.h"
#include "Core/ResourceManager.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // To test components that use OpenGL, we need a valid OpenGL context.
    // We can create a simple, hidden window with GLFW for this purpose.
    if (!glfwInit()) {
        Log::Debug("Failed to initialize GLFW for testing.");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Keep the window hidden

    GLFWwindow* testWindow = glfwCreateWindow(100, 100, "Test Context", NULL, NULL);
    if (!testWindow) {
        Log::Debug("Failed to create GLFW window for testing.");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(testWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::Debug("Failed to initialize GLAD for testing.");
        glfwTerminate();
        return -1;
    }

    // Initialize managers that the tests might depend on
    ResourceManager::Initialize();

    int result = RUN_ALL_TESTS();

    // Cleanup
    ResourceManager::Shutdown();
    glfwDestroyWindow(testWindow);
    glfwTerminate();

    return result;
}