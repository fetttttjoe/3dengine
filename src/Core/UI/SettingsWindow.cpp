#include "Core/UI/SettingsWindow.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/SettingsManager.h"
#include "Core/UI/AppUI.h"
#include "Scene/Grid.h"
#include "Scene/Scene.h"

SettingsWindow::SettingsWindow(Application* app)
    : m_App(app), m_AppUI(app->GetUI()) {}

void SettingsWindow::RevertToSavedSettings() {
  AppSettings& savedSettings = SettingsManager::Get();
  m_TempLeftPaneWidth = savedSettings.leftPaneWidth;
  m_TempRightPaneWidth = savedSettings.rightPaneWidth;
}

void SettingsWindow::Draw() {
  m_IsVisible = m_App->GetShowSettings();
  if (!m_IsVisible) return;

  ImGui::Begin("Settings", &m_IsVisible,
               ImGuiWindowFlags_AlwaysAutoResize); 

  if (ImGui::IsWindowAppearing()) {
    RevertToSavedSettings();
  }

  ImGui::Text("UI Settings");
  ImGui::DragFloat("Left Pane Width", &m_TempLeftPaneWidth, 1.0f, 100.0f,
                   500.0f);
  ImGui::DragFloat("Right Pane Width", &m_TempRightPaneWidth, 1.0f, 100.0f,
                   500.0f);
  ImGui::Separator();

  ImGui::Text("Highlight Colors");
  for (const auto& desc : SettingsManager::GetDescriptors()) {
      if (desc.type == SettingType::Color4) {
          ImGui::ColorEdit4(desc.label.c_str(), (float*)desc.ptr);
      }
  }


  ImGui::Separator();

  ImGui::Text("World Settings");
  ImGui::DragInt("Grid Size", &SettingsManager::Get().gridSize, 1, 10, 200);
  ImGui::DragInt("Grid Divisions", &SettingsManager::Get().gridDivisions, 1, 10,
                 200);
  if (ImGui::Button("Apply Grid Settings")) {
    for (const auto& obj : m_App->GetScene()->GetSceneObjects()) {
      if (auto* grid = dynamic_cast<Grid*>(obj.get())) {
        grid->SetConfiguration(SettingsManager::Get().gridSize,
                               SettingsManager::Get().gridDivisions);
      }
    }
  }
  ImGui::Separator();

  if (ImGui::Button("Save and Close")) {
    SettingsManager::Get().leftPaneWidth = m_TempLeftPaneWidth;
    SettingsManager::Get().rightPaneWidth = m_TempRightPaneWidth;

    if (SettingsManager::Save("settings.json")) {
      Log::Debug("SettingsManager: saved settings.json");
    }
    m_App->SetShowSettings(false);
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) {
    m_App->SetShowSettings(false);
  }

  ImGui::End();

  if (!m_IsVisible) {
    m_App->SetShowSettings(false);
  }
}