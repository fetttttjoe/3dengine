#pragma once
#include <memory>

// Forward-declarations
struct GLFWwindow;
class IRenderer;
class Scene;
class Camera;
class UI;

class Application {
public:
    Application();
    ~Application();
    void Run();

private:
    void Initialize();
    void MainLoop();
    void Cleanup();
    void processInput();

    // GLFW Callbacks
    static void error_callback(int error, const char* description);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    GLFWwindow* m_Window = nullptr;
    const unsigned int m_WindowWidth = 1280;
    const unsigned int m_WindowHeight = 720;

    std::unique_ptr<IRenderer> m_Renderer;
    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<UI> m_UI;
};