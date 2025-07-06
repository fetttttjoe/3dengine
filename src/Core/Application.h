#pragma once

#include <memory>
#include <functional>
#include <glm/glm.hpp>

// Forward declarations
class OpenGLRenderer;
class Scene;
class Camera;
class UI;
class SceneObjectFactory;
class TransformGizmo;
class ISceneObject;
struct GLFWwindow;

class Application {
public:
    Application(int initialWidth = 1280, int initialHeight = 720);
    ~Application();
    void Run();

    int GetWindowWidth() const { return m_WindowWidth; }
    int GetWindowHeight() const { return m_WindowHeight; }

private:
    void Initialize();
    void Cleanup();
    void RegisterObjectTypes();

    void processKeyboardInput();
    void processMouseInput();

    static void error_callback(int error, const char* description);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

    GLFWwindow* m_Window = nullptr;
    int m_WindowWidth;
    int m_WindowHeight;
    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    std::unique_ptr<OpenGLRenderer> m_Renderer;
    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<UI> m_UI;
    std::unique_ptr<SceneObjectFactory> m_ObjectFactory;
    std::unique_ptr<TransformGizmo> m_TransformGizmo;

    // Input state
    bool m_IsDraggingObject = false;
    bool m_IsDraggingGizmo = false;
    ISceneObject* m_DraggedObject = nullptr;
    glm::vec2 m_LastMousePos;
    
    bool m_StaticCacheDirty = true;
};