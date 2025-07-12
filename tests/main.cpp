#include "gtest/gtest.h"
#include "Core/Application.h"

// The main function for tests now only needs to initialize Google Test
// and manage the lifetime of the core Application object.
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int result = 0;
    // Create a scope to ensure the Application instance is created before
    // any tests run and destroyed after all tests have completed.
    {
        auto app = std::make_unique<Application>(100, 100);
        result = RUN_ALL_TESTS();
    }
    // When this scope ends, 'app' is destroyed, and all engine resources
    // are cleanly released before the program exits.

    return result;
}