#include "Core/UI/ToolsPane.h"

#include <imgui.h>

ToolsPane::ToolsPane() {}

void ToolsPane::Draw(bool& showMetricsWindow) {
  ImGui::Text("Tools");
  if (ImGui::Button("Reset Cam") && OnResetCamera) {
    OnResetCamera();
  }
#ifndef NDEBUG
  ImGui::Separator();
  const char* lbl = showMetricsWindow ? "Hide Metrics" : "Show Metrics";
  if (ImGui::Button(lbl)) {
    showMetricsWindow = !showMetricsWindow;
  }
#endif
}