#pragma once

#include <Interfaces.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Sculpting/ISculptTool.h"

// Forward declarations
struct GLFWwindow;
class Camera;
class OpenGLRenderer;
class Scene;
class SceneObjectFactory;
class TransformGizmo;
class AppUI;
class PushPullTool;
class SmoothTool;
class GrabTool;
class SubObjectSelection;
class MeshEditor;

enum class EditorMode { TRANSFORM, SCULPT, SUB_OBJECT };
enum class SubObjectMode { VERTEX, EDGE, FACE };

class Application {
 public:
  Application(int initialWidth, int initialHeight);
  ~Application();

  void Run();

  // --- Core System Accessors ---
  Scene* GetScene() const { return m_Scene.get(); }
  TransformGizmo* GetTransformGizmo() const { return m_TransformGizmo.get(); }
  SceneObjectFactory* GetObjectFactory() const { return m_ObjectFactory.get(); }
  AppUI* GetUI() const { return m_UI.get(); }
  OpenGLRenderer* GetRenderer() const { return m_Renderer.get(); }
  Camera* GetCamera() const { return m_Camera.get(); }
  GLFWwindow* GetWindow() { return m_Window; }
  SubObjectSelection* GetSelection() { return m_Selection.get(); }

  // --- State Management ---
  void SelectObject(uint32_t id);
  void SetEditorMode(EditorMode newMode,
                     SculptMode::Mode newSculptMode = SculptMode::Pull,
                     SubObjectMode newSubObjectMode = SubObjectMode::VERTEX);
  EditorMode GetEditorMode() const { return m_EditorMode; }
  SculptMode::Mode GetSculptMode() const { return m_SculptMode; }
  SubObjectMode GetSubObjectMode() const { return m_SubObjectMode; }
  bool GetShowAnchors() const { return m_ShowAnchors; }
  void SetShowAnchors(bool show) {
    m_ShowAnchors = show;
    RequestSceneRender();
  }
  void SetShowSettings(bool show) { m_ShowSettingsWindow = show; }
  bool GetShowSettings() const { return m_ShowSettingsWindow; }
  void SetShowMetricsWindow(bool show) { m_ShowMetricsWindow = show; }
  bool GetShowMetricsWindow() const { return m_ShowMetricsWindow; }

  // --- Scene Render Request ---
  void RequestSceneRender() { m_SceneRenderRequested = true; }

  // --- Actions ---
  void OnSceneLoaded();
  void ImportModel(const std::string& filepath);
  void Exit();
  void RequestObjectDuplication(uint32_t objectID);
  void RequestObjectDeletion(uint32_t objectID);
  void RequestObjectCreation(const std::string& typeName);
  void RequestExtrude(float distance);
  void RequestWeld();
  void RequestMoveSelection(float distance);

  // --- Singleton Accessor ---
  static Application& Get();

 private:
  void Initialize();
  void Cleanup();
  void RegisterObjectTypes();

  void ProcessPendingActions();
  void processGlobalKeyboardShortcuts();
  void processMouseActions();
  void processSculpting();

  static void framebuffer_size_callback(GLFWwindow* window, int w, int h);
  static void scroll_callback(GLFWwindow* window, double xoffset,
                              double yoffset);
  static void cursor_position_callback(GLFWwindow* window, double xpos,
                                       double ypos);
  static void error_callback(int error, const char* desc);

  // --- Singleton Instance ---
  static Application* s_Instance;

  GLFWwindow* m_Window = nullptr;
  int m_WindowWidth;
  int m_WindowHeight;

  // --- Core Systems ---
  std::unique_ptr<SceneObjectFactory> m_ObjectFactory;
  std::unique_ptr<OpenGLRenderer> m_Renderer;
  std::unique_ptr<AppUI> m_UI;
  std::unique_ptr<Scene> m_Scene;
  std::unique_ptr<Camera> m_Camera;
  std::unique_ptr<TransformGizmo> m_TransformGizmo;
  std::unique_ptr<PushPullTool> m_PushPullTool;
  std::unique_ptr<SmoothTool> m_SmoothTool;
  std::unique_ptr<GrabTool> m_GrabTool;
  std::unique_ptr<SubObjectSelection> m_Selection;
  std::unique_ptr<MeshEditor> m_MeshEditor;

  // --- State ---
  EditorMode m_EditorMode = EditorMode::TRANSFORM;
  SculptMode::Mode m_SculptMode = SculptMode::Pull;
  SubObjectMode m_SubObjectMode = SubObjectMode::VERTEX;
  float m_LastFrame = 0.0f;
  float m_DeltaTime = 0.0f;
  bool m_ShowAnchors = true;
  bool m_ShowSettingsWindow = false;
  bool m_ShowMetricsWindow = false;

  // --- Dirty Flag for Rendering ---
  bool m_SceneRenderRequested = true;

  // --- Input State ---
  glm::vec2 m_LastViewportSize = {0, 0};
  bool m_IsDraggingGizmo = false;
  bool m_IsSculpting = false;

  // --- Pending Action Queues ---
  std::vector<std::string> m_RequestedCreationTypeNames;
  uint32_t m_RequestedDuplicateID = 0;
  std::vector<uint32_t> m_RequestedDeletionIDs;
  bool m_ExtrudeRequested = false;
  float m_ExtrudeDistance = 0.1f;
  bool m_WeldRequested = false;
  bool m_MoveSelectionRequested = false;
  float m_MoveSelectionDistance = 0.1f;
};