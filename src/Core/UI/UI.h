#pragma once

// Forward-declarations
class Scene;
class SceneObjectFactory;
struct GLFWwindow;

class UI {
public:
    UI(Scene* scene);
    ~UI();

    void Initialize(GLFWwindow* window);
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();

    // This is now the single public entry point for drawing.
    void DrawUI();
    void SetObjectFactory(SceneObjectFactory* factory);

private:
    void DrawMainMenu();
    void DrawSceneOutliner();
    void DrawPropertiesPanel();

    Scene* m_Scene;
    SceneObjectFactory* m_Factory;
};