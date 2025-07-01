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
struct GLFWwindow;

enum class RenderingMode {
    OnDemand,
    Continuous
};

class Application {
public:
    Application(int initialWidth = 1280, int initialHeight = 720);
    ~Application();
    void Run();
    void RequestRedraw();

private:
    void Initialize();
    void Cleanup();
    void RegisterObjectTypes();
    void RunContinuous();
    void RunOnDemand();
    
    // REMOVED: The RenderFrame helper is no longer used.
    // void RenderFrame();

    void processKeyboardInput();
    void processMouseInput();

    // ... other members and callbacks remain the same ...
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

    bool m_IsDraggingObject = false;
    class ISceneObject* m_DraggedObject = nullptr;
    glm::vec2 m_LastMousePos;
    
    RenderingMode m_RenderingMode = RenderingMode::OnDemand;
    bool m_RedrawNeeded = true; 
    bool m_StaticCacheDirty = true;
};