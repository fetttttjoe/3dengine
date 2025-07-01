// Core/UI/UI.h
#pragma once

#include <string>

// Forward declarations
class Scene;
class SceneObjectFactory;
struct GLFWwindow;

class UI {
public:
    explicit UI(Scene* scene);
    ~UI();

    // Call once on startup
    void Initialize(GLFWwindow* window);
    // Call once before shutdown
    void Shutdown();

    // Per-frame ImGui calls
    void BeginFrame();
    void EndFrame();

    // Draw all UI (menus, panels, etc.)
    void DrawUI();
    // Provide the factory so UI can add new objects
    void SetObjectFactory(SceneObjectFactory* factory);

private:
    void DrawMainMenu();
    void DrawSceneOutliner();
    void DrawPropertiesPanel();

    Scene*              m_Scene;
    SceneObjectFactory* m_Factory;

    // For inline renaming in the outliner
    uint32_t            m_RenameID = 0;
    std::string         m_RenameBuffer;
};
