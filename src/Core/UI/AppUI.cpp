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

AppUI::AppUI(Application* app) : m_App(app) {}

AppUI::~AppUI() {}

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
  ImGui::InvisibleButton(
      "##split", ImVec2(5, -1),  // Height -1 means "fill remaining height"
      ImGuiButtonFlags_MouseButtonLeft);
  ImGui::SetItemAllowOverlap();

  if (ImGui::IsItemHovered()) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  }

  if (ImGui::IsItemActive()) {
    float mouse_delta_x = ImGui::GetIO().MouseDelta.x;
    if (invertDirection) {
      // Clamp to prevent panes from becoming too small or overlapping
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

  auto* settingsWindow = GetView<SettingsWindow>();
  float leftPaneWidth, rightPaneWidth;

  if (settingsWindow && settingsWindow->IsVisible()) {
    leftPaneWidth = settingsWindow->GetLeftPaneWidth();
    rightPaneWidth = settingsWindow->GetRightPaneWidth();
  } else {
    leftPaneWidth = SettingsManager::Get().leftPaneWidth;
    rightPaneWidth = SettingsManager::Get().rightPaneWidth;
  }

  // Calculate the available content height AFTER the menu bar is drawn.
  // This is crucial for correctly sizing the child windows vertically.
  float content_height = ImGui::GetContentRegionAvail().y;

  // Left Pane
  ImGui::BeginChild("LeftPane", ImVec2(leftPaneWidth, content_height),
                    true);  // Use calculated content_height
  if (auto* toolsPane = GetView<ToolsPane>()) {
    toolsPane->Draw();
  }
  ImGui::EndChild();
  ImGui::SameLine();

  // Splitter between Left Pane and Viewport
  if (settingsWindow && settingsWindow->IsVisible()) {
    DrawSplitter("split_left", settingsWindow->GetLeftPaneWidth(), false);
  } else {
    DrawSplitter("split_left", SettingsManager::Get().leftPaneWidth, false);
  }
  ImGui::SameLine();

  // Center (Viewport)
  float available_x =
      ImGui::GetContentRegionAvail()
          .x;  // Available width after left pane and its splitter
  float viewport_width = available_x - rightPaneWidth;
  if (viewport_width < 100.0f) viewport_width = 100.0f;  // Ensure minimum width

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::BeginChild(
      "ViewportPane", ImVec2(viewport_width, content_height),
      false,  // Use calculated content_height
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  if (auto* viewportPane = GetView<ViewportPane>()) {
    viewportPane->Draw();
  }
  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::SameLine();

  // Splitter between Viewport and Right Pane
  if (settingsWindow && settingsWindow->IsVisible()) {
    DrawSplitter("split_right", settingsWindow->GetRightPaneWidth(), true);
  } else {
    DrawSplitter("split_right", SettingsManager::Get().rightPaneWidth, true);
  }
  ImGui::SameLine();

  // Right Pane
  ImGui::BeginChild("RightPane", ImVec2(rightPaneWidth, content_height),
                    true);  // Use calculated content_height
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

  ImGui::End();  // End MainAppWindow

  // Draw settings window outside the main window layout to allow it to float
  if (settingsWindow) {
    settingsWindow->Draw();
  }
}