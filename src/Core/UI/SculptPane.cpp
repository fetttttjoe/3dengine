#include "Core/UI/SculptPane.h"

#include <imgui.h>

SculptPane::SculptPane() {}

void SculptPane::Draw() {
  ImGui::Text("Sculpting");
  ImGui::Separator();

  // Sculpt Mode Toggle
  static bool sculpt_mode_active = false;
  if (ImGui::Checkbox("Enable Sculpt Mode", &sculpt_mode_active)) {
    if (OnToggleSculptMode) {
      OnToggleSculptMode(sculpt_mode_active);
    }
  }

  ImGui::Separator();
  ImGui::Text("Brush Settings");
  ImGui::DragFloat("Radius", &m_BrushRadius, 0.01f, 0.01f, 5.0f);
  ImGui::DragFloat("Strength", &m_BrushStrength, 0.01f, 0.01f, 1.0f);

  ImGui::Separator();
  ImGui::Text("Tool");

  if (ImGui::RadioButton("Pull", m_CurrentMode == SculptMode::Pull)) {
    m_CurrentMode = SculptMode::Pull;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Push", m_CurrentMode == SculptMode::Push)) {
    m_CurrentMode = SculptMode::Push;
  }
}
