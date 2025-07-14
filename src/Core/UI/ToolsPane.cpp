#include "Core/UI/ToolsPane.h"

#include <imgui.h>

#include "Core/Application.h"
#include "Core/Camera.h"

ToolsPane::ToolsPane(Application* app) : m_App(app) {}

void ToolsPane::Draw() {
  ImGui::Text("Tools");
  ImGui::Separator();

  if (ImGui::Button("Reset Cam")) {
    m_App->GetCamera()->ResetToDefault();
  }

#ifndef NDEBUG
  ImGui::Separator();
  bool showMetrics = m_App->GetShowMetricsWindow();
  if (ImGui::Checkbox("Show Metrics", &showMetrics)) {
    m_App->SetShowMetricsWindow(showMetrics);
  }
#endif
}