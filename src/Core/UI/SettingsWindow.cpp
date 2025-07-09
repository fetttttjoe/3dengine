#include "Core/UI/SettingsWindow.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/SettingsManager.h"

SettingsWindow::SettingsWindow(Application* app) : m_App(app) {}

void SettingsWindow::Draw() {
  bool show = m_App->GetShowSettings();
  if (!show) return;

  ImGui::Begin("Settings", &show);
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
    }
  }
  if (ImGui::Button("Save Settings")) {
    if (SettingsManager::Save("settings.json")) {
      Log::Debug("SettingsManager: saved settings.json");
    }
  }
  ImGui::End();

  if (!show) {
    m_App->SetShowSettings(false);
  }
}
