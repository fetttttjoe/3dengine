#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "imgui.h"

class Application;
class Scene;
class SceneObjectFactory;
struct GLFWwindow;

#include "Core/UI/HierarchyView.h"
#include "Core/UI/InspectorView.h"
#include "Core/UI/MenuBar.h"
#include "Core/UI/SettingsWindow.h"
#include "Core/UI/ToolsPane.h"
#include "Core/UI/ViewportPane.h"

class AppUI {
 public:
  explicit AppUI(Application* app);
  ~AppUI();

  void Initialize(GLFWwindow* window);
  void Shutdown();
  void BeginFrame();
  void EndFrame();

  void Draw(uint32_t viewportTextureId);

  void SetObjectFactory(SceneObjectFactory* factory);
  void SetExitRequestHandler(std::function<void()> handler);
  void SetResetCameraHandler(std::function<void()> handler);
  void ShowInspector();

  // NEW: Add the declaration for the handler setter
  void SetOnSceneLoadedHandler(std::function<void()> handler);  //

  glm::vec2 GetViewportSize() const;
  const std::array<ImVec2, 2>& GetViewportBounds() const;
  bool IsViewportFocused() const;
  bool IsViewportHovered() const;

#if !defined(NDEBUG)
  bool IsMetricsWindowVisible() const { return m_ShowMetricsWindow; }
#endif

 private:
  static void DrawSplitter(const char* id, float& valueToAdjust,
                           bool invertDirection);

  Application* m_App;
  Scene* m_Scene;

  std::unique_ptr<MenuBar> m_MenuBar;
  std::unique_ptr<ToolsPane> m_ToolsPane;
  std::unique_ptr<ViewportPane> m_ViewportPane;
  std::unique_ptr<HierarchyView> m_HierarchyView;
  std::unique_ptr<InspectorView> m_InspectorView;
  std::unique_ptr<SettingsWindow> m_SettingsWindow;

  float m_LeftPaneWidth;
  float m_RightPaneWidth;

  int m_ActiveRightTab = -1;

  std::function<void()> m_ExitRequestHandler;
  std::function<void()> m_ResetCameraHandler;
  std::function<void()> m_OnSceneLoadedHandler;  //
  bool m_ShowSettingsWindow = false;

#if !defined(NDEBUG)
  bool m_ShowMetricsWindow = false;
#endif
};