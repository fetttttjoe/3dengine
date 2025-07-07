#include "Core/UI/SettingsWindow.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>  // For glm::value_ptr

#include "Core/Log.h"              // For logging save success
#include "Core/PropertyNames.h"    // For SettingType enum
#include "Core/SettingsManager.h"  // To access settings data and save

SettingsWindow::SettingsWindow() {}

void SettingsWindow::Draw(bool& showWindow) {
  if (!showWindow) return;  // Only draw if flag is true

  ImGui::Begin("Settings", &showWindow);
  for (auto const& desc : SettingsManager::GetDescriptors()) {
    switch (desc.type) {
      case SettingType::Float: {
        auto ptr = static_cast<float*>(desc.ptr);
        ImGui::DragFloat(desc.label.c_str(), ptr, 1.0f);
      } break;
      case SettingType::Float3: {
        auto ptr = static_cast<glm::vec3*>(desc.ptr);
        ImGui::DragFloat3(desc.label.c_str(), glm::value_ptr(*ptr), 0.05f);
      } break;
        // Add cases for other setting types if they exist (e.g., Bool, Int,
        // String)
    }
  }
  if (ImGui::Button("Save Settings")) {
    if (SettingsManager::Save("settings.json")) {
      Log::Debug("SettingsManager: saved settings.json");
    }
  }
  ImGui::End();
}