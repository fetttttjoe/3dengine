#include "Core/UI/SettingsWindow.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/SettingsManager.h"
#include "Scene/Scene.h"
#include "Scene/Grid.h"

SettingsWindow::SettingsWindow(Application* app) : m_App(app) {}

void SettingsWindow::Draw() {
    bool show = m_App->GetShowSettings();
    if (!show) return;

    ImGui::Begin("Settings", &show);

    bool settingsChanged = false;
    for (auto const& desc : SettingsManager::GetDescriptors()) {
        switch (desc.type) {
            case SettingType::Float: {
                auto ptr = static_cast<float*>(desc.ptr);
                if (ImGui::DragFloat(desc.label.c_str(), ptr, 0.1f)) {
                    settingsChanged = true;
                }
            } break;
            case SettingType::Float3: {
                auto ptr = static_cast<glm::vec3*>(desc.ptr);
                if(ImGui::DragFloat3(desc.label.c_str(), glm::value_ptr(*ptr), 0.05f)) {
                    settingsChanged = true;
                }
            } break;
            case SettingType::Int: {
                auto ptr = static_cast<int*>(desc.ptr);
                if (ImGui::DragInt(desc.label.c_str(), ptr, 1.0f, 1)) {
                    settingsChanged = true;
                }
            } break;
        }
    }
    
    ImGui::Separator();

    if (ImGui::Button("Apply Grid Settings")) {
        for(const auto& obj : m_App->GetScene()->GetSceneObjects()) {
            if (auto* grid = dynamic_cast<Grid*>(obj.get())) {
                grid->SetConfiguration(SettingsManager::Get().gridSize, SettingsManager::Get().gridDivisions);
            }
        }
    }

    if (ImGui::Button("Save All Settings")) {
        if (SettingsManager::Save("settings.json")) {
            Log::Debug("SettingsManager: saved settings.json");
        }
    }

    ImGui::End();

    if (!show) {
        m_App->SetShowSettings(false);
    }
}