// Core/Application.h
#pragma once

#include <Interfaces.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
// Forward declarations
struct GLFWwindow;
class Camera;
class OpenGLRenderer;
class Scene;
class SceneObjectFactory;
class TransformGizmo;
// class UI; // Old forward declaration
class AppUI;  // New forward declaration
class Application {
 public:
  Application(int initialWidth, int initialHeight);
  ~Application();

  void Run();

  // Accessors for UI, Scene, etc. used by other parts of the system
  Scene* GetScene() const { return m_Scene.get(); }
  TransformGizmo* GetTransformGizmo() const { return m_TransformGizmo.get(); }
  SceneObjectFactory* GetObjectFactory() const {
    return m_ObjectFactory.get();
  }  // Added this getter
  bool GetShowAnchors() const { return m_ShowAnchors; }
  void SetShowAnchors(bool show) { m_ShowAnchors = show; }

  void SelectObject(uint32_t id);  // Method to select an object, called by UI

 private:
  void Initialize();
  void Cleanup();
  void RegisterObjectTypes();  // Registers object types with m_ObjectFactory

  void processKeyboardInput();
  void processMouseInput();

  // GLFW callbacks
  static void framebuffer_size_callback(GLFWwindow* window, int w, int h);
  static void scroll_callback(GLFWwindow* window, double xoffset,
                              double yoffset);
  static void cursor_position_callback(GLFWwindow* window, double xpos,
                                       double ypos);
  static void error_callback(int error, const char* desc);

  GLFWwindow* m_Window = nullptr;
  int m_WindowWidth;
  int m_WindowHeight;

  std::unique_ptr<Camera> m_Camera;
  std::unique_ptr<OpenGLRenderer> m_Renderer;
  std::unique_ptr<Scene> m_Scene;
  std::unique_ptr<SceneObjectFactory> m_ObjectFactory;
  std::unique_ptr<TransformGizmo> m_TransformGizmo;
  std::unique_ptr<AppUI> m_UI;  // Changed type from UI to AppUI

  float m_LastFrame = 0.0f;
  float m_DeltaTime = 0.0f;

  bool m_ShowAnchors = true;  // State for showing/hiding scene anchors

  // Mouse picking/dragging state
  glm::vec2 m_LastMousePos = {0, 0};
  bool m_IsDraggingObject = false;
  ISceneObject* m_DraggedObject = nullptr;
  float m_DragNDCDepth = 0.0f;
  bool m_IsDraggingGizmo = false;
};