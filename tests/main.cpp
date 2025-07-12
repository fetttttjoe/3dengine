#include "gtest/gtest.h"
#include "Core/Application.h" // The Application header handles all necessary setup.

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int result = 0;

    // Create a scope for the application instance.
    // Its constructor will handle all GLFW/GLAD/Resource Manager setup.
    // Its destructor will handle all cleanup in the correct order.
    {
        auto app = std::make_unique<Application>(100, 100);
        result = RUN_ALL_TESTS();
    }
    // When this scope ends, 'app' is destroyed, and all resources are
    // cleanly released before the program exits.

    return result;
}