#include "gtest/gtest.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Scene.h"
#include "Interfaces.h"
#include "Renderer/OpenGLRenderer.h"
#include <GLFW/glfw3.h>

// Include all object types to register them with the factory
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/Sphere.h"
#include "Scene/Objects/Icosphere.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Objects/CustomMesh.h"
#include "Scene/Grid.h"
#include "Scene/Objects/ObjectTypes.h"

class SceneLoadingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // In a test environment, we need to manually register the types that
        // the scene file will reference, just like the main application does.
        factory.Register(std::string(ObjectTypes::Pyramid), []() { return std::make_unique<Pyramid>(); });
        factory.Register(std::string(ObjectTypes::Sphere), []() { return std::make_unique<Sphere>(); });
        factory.Register(std::string(ObjectTypes::Icosphere), []() { return std::make_unique<Icosphere>(); });
        factory.Register(std::string(ObjectTypes::Triangle), []() { return std::make_unique<Triangle>(); });
        factory.Register(std::string(ObjectTypes::CustomMesh), []() { return std::make_unique<CustomMesh>(); });
        factory.Register(std::string(ObjectTypes::Grid), []() { return std::make_unique<Grid>(); });

        scene = std::make_unique<Scene>(&factory);
        renderer = std::make_unique<OpenGLRenderer>();
        renderer->Initialize(glfwGetCurrentContext()); // Initialize with the context from the test runner
    }

    SceneObjectFactory factory;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<OpenGLRenderer> renderer;
};

// This integration test loads a scene containing one of every object type.
// It verifies that all objects can be deserialized and that their core
// functionality (like the Draw call) does not cause a crash.
TEST_F(SceneLoadingTest, LoadAndVerifyAllObjects) {
    try {
        // Load the master scene file.
        scene->Load("test_scene.json");

        // Check that objects were actually loaded.
        ASSERT_FALSE(scene->GetSceneObjects().empty()) << "No objects were loaded from test_scene.json. Make sure the file exists in the build directory.";

        // IMPORTANT: Sync the scene with the renderer. This creates the GPU resources
        // for each object before we try to draw them.
        renderer->SyncSceneObjects(*scene);

        // Dummy matrices for testing the Draw() call.
        glm::mat4 dummyMat(1.0f);

        for (const auto& object : scene->GetSceneObjects()) {
            // 1. Verify the object pointer is valid.
            ASSERT_NE(object, nullptr);

            // 2. Log which object we are checking.
            std::cout << "Verifying object: " << object->name << " (ID: " << object->id << ", Type: " << object->GetTypeString() << ")" << std::endl;

            // 3. Perform a "smoke test" on the Draw() method.
            object->Draw(*renderer, dummyMat, dummyMat);
        }
    }
    catch (const nlohmann::json::exception& e) {
        FAIL() << "A JSON parsing error occurred during the test: " << e.what();
    }
    catch (const std::exception& e) {
        FAIL() << "An unexpected exception occurred during the test: " << e.what();
    }
    // If the loop completes without crashing or failing, all objects are valid.
    SUCCEED();
}