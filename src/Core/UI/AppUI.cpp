#include "Core/UI/AppUI.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <algorithm>

#include "Core/Application.h"
#include "Core/SettingsManager.h"
#include "Core/UI/HierarchyView.h"
#include "Core/UI/InspectorView.h"
#include "Core/UI/MenuBar.h"
#include "Core/UI/SettingsWindow.h"
#include "Core/UI/ToolsPane.h"
#include "Core/UI/ViewportPane.h"

AppUI::AppUI(Application* app)
    : m_App(app),
      m_LeftPaneWidth(SettingsManager::Get().leftPaneWidth > 0
                          ? SettingsManager::Get().leftPaneWidth
                          : 200.0f),
      m_RightPaneWidth(SettingsManager::Get().rightPaneWidth > 0
                           ? SettingsManager::Get().rightPaneWidth
                           : 300.0f) {}

AppUI::~AppUI() {
  // Values are now updated live, but we still save them here as a final
  // measure.
  SettingsManager::Get().leftPaneWidth = m_LeftPaneWidth;
  SettingsManager::Get().rightPaneWidth = m_RightPaneWidth;
}

void AppUI::Initialize(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void AppUI::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void AppUI::BeginFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void AppUI::EndFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppUI::DrawSplitter(const char* id, float& valueToAdjust,
                         bool invertDirection) {
  ImGui::PushID(id);
  ImGui::InvisibleButton("##split", ImVec2(5, -1),
                         ImGuiButtonFlags_MouseButtonLeft);
  ImGui::SetItemAllowOverlap();

  if (ImGui::IsItemHovered()) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  }

  if (ImGui::IsItemActive()) {
    float mouse_delta_x = ImGui::GetIO().MouseDelta.x;
    if (invertDirection) {
      valueToAdjust = std::clamp(valueToAdjust - mouse_delta_x, 100.0f,
                                 ImGui::GetWindowSize().x - 100.0f);
    } else {
      valueToAdjust = std::clamp(valueToAdjust + mouse_delta_x, 100.0f,
                                 ImGui::GetWindowSize().x - 100.0f);
    }
  }

  ImVec2 min_pos = ImGui::GetItemRectMin();
  ImVec2 max_pos = ImGui::GetItemRectMax();
  float line_x = (min_pos.x + max_pos.x) * 0.5f;
  ImGui::GetWindowDrawList()->AddLine({line_x, min_pos.y}, {line_x, max_pos.y},
                                      ImGui::GetColorU32(ImGuiCol_Separator),
                                      2.0f);
  ImGui::PopID();
}

void AppUI::Draw() {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("MainAppWindow", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);
  ImGui::PopStyleVar();

  if (auto* menuBar = GetView<MenuBar>()) {
    menuBar->Draw();
  }

  // Left Pane
  ImGui::BeginChild("LeftPane", ImVec2(m_LeftPaneWidth, 0), true);
  if (auto* toolsPane = GetView<ToolsPane>()) {
    toolsPane->Draw();
  }
  ImGui::EndChild();
  ImGui::SameLine();
  DrawSplitter("split_left", m_LeftPaneWidth, false);
  ImGui::SameLine();

  // Center (Viewport)
  float available_x = ImGui::GetContentRegionAvail().x;
  float viewport_width = available_x - m_RightPaneWidth - 5.0f;
  if (viewport_width < 100.0f) viewport_width = 100.0f;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::BeginChild(
      "ViewportPane", ImVec2(viewport_width, 0), false,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  if (auto* viewportPane = GetView<ViewportPane>()) {
    viewportPane->Draw();
  }
  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::SameLine();
  DrawSplitter("split_right", m_RightPaneWidth, true);
  ImGui::SameLine();

  // Right Pane
  ImGui::BeginChild("RightPane", ImVec2(m_RightPaneWidth, 0), true);
  if (ImGui::BeginTabBar("RightTabs")) {
    if (ImGui::BeginTabItem("Hierarchy")) {
      if (auto* hierarchyView = GetView<HierarchyView>()) {
        hierarchyView->Draw();
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Inspector")) {
      if (auto* inspectorView = GetView<InspectorView>()) {
        inspectorView->Draw();
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::EndChild();

  SettingsManager::Get().leftPaneWidth = m_LeftPaneWidth;
  SettingsManager::Get().rightPaneWidth = m_RightPaneWidth;

  ImGui::End();

  // Draw any other top-level windows, like the settings window itself
  if (auto* settingsWindow = GetView<SettingsWindow>()) {
    settingsWindow->Draw();
  }
}