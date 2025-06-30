
// =======================================================================
// File: src/UI.h (NEW FILE)
// Description: Manages the User Interface using Dear ImGui.
// =======================================================================
#pragma once

// Forward-declarations
class Scene;
struct GLFWwindow;

class UI {
public:
    UI();
    ~UI();

    void Initialize(GLFWwindow* window);
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();

    void DrawPropertiesPanel(Scene& scene);
};
