#include "gtest/gtest.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Scene.h"
#include "Interfaces.h"
#include "Renderer/OpenGLRenderer.h"
#include <GLFW/glfw3.h>
#include "Core/Application.h" // Required to get the global renderer

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
        factory.Register(std::string(ObjectTypes::Pyramid), []() { return std::make_unique<Pyramid>(); });
        factory.Register(std::string(ObjectTypes::Sphere), []() { return std::make_unique<Sphere>(); });
        factory.Register(std::string(ObjectTypes::Icosphere), []() { return std::make_unique<Icosphere>(); });
        factory.Register(std::string(ObjectTypes::Triangle), []() { return std::make_unique<Triangle>(); });
        factory.Register(std::string(ObjectTypes::CustomMesh), []() { return std::make_unique<CustomMesh>(); });
        factory.Register(std::string(ObjectTypes::Grid), []() { return std::make_unique<Grid>(); });

        scene = std::make_unique<Scene>(&factory);
        // Get the renderer from the global Application instance instead of creating a new one.
        renderer = Application::Get().GetRenderer();
    }

    SceneObjectFactory factory;
    std::unique_ptr<Scene> scene;
    // The renderer is now a raw pointer, as its lifetime is managed by the Application.
    OpenGLRenderer* renderer;
};

TEST_F(SceneLoadingTest, LoadAndVerifyAllObjects) {
    try {
        scene->Load("test_scene.json");

        ASSERT_FALSE(scene->GetSceneObjects().empty()) << "No objects were loaded from test_scene.json. Make sure the file exists in the build directory.";

        // Sync the scene with the renderer.
        renderer->SyncSceneObjects(*scene);

        glm::mat4 dummyMat(1.0f);

        for (const auto& object : scene->GetSceneObjects()) {
            ASSERT_NE(object, nullptr);
            std::cout << "Verifying object: " << object->name << " (ID: " << object->id << ", Type: " << object->GetTypeString() << ")" << std::endl;
            // The Draw call will now use the globally managed renderer.
            object->Draw(*renderer, dummyMat, dummyMat);
        }
    }
    catch (const nlohmann::json::exception& e) {
        FAIL() << "A JSON parsing error occurred during the test: " << e.what();
    }
    catch (const std::exception& e) {
        FAIL() << "An unexpected exception occurred during the test: " << e.what();
    }
    SUCCEED();
}