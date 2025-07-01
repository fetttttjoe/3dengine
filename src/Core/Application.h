// File: src/Core/Application.h
#pragma once

#include <memory>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> // <-- Add this line for glm::vec2 and other core types

// Forward declarations
class OpenGLRenderer;
class Scene;
class Camera;
class UI;
class SceneObjectFactory;
class ISceneObject;

class Application {
public:
    Application();
    ~Application();

    void Run();

private:
    void Initialize();
    void Cleanup();
    void processInput();

    // GLFW Callbacks must be static to be set by glfwSet*Callback
    static void error_callback(int error, const char* description);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    // New: Mouse motion callback for drag and drop
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);


    void RegisterObjectTypes();

    int m_WindowWidth = 1280;
    int m_WindowHeight = 720;
    GLFWwindow* m_Window;

    std::unique_ptr<OpenGLRenderer> m_Renderer;
    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<UI> m_UI;
    std::unique_ptr<SceneObjectFactory> m_ObjectFactory;

    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    // Drag and Drop state
    bool m_IsDraggingObject = false;
    glm::vec2 m_LastMousePos = glm::vec2(0.0f);
    // Keep track of the object being dragged
    ISceneObject* m_DraggedObject = nullptr;
};