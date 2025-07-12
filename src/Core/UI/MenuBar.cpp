#include "Core/UI/MenuBar.h"
#include <imgui.h>
#include "Core/Application.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Scene.h"
#include "nfd.hpp"

MenuBar::MenuBar(Application* app) : m_App(app) {}

void MenuBar::Draw() {
    if (ImGui::BeginMainMenuBar()) {
        DrawFileMenu();
        DrawViewMenu();
        DrawSceneMenu();
        ImGui::EndMainMenuBar();
    }
}

void MenuBar::DrawFileMenu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save Scene")) {
            m_App->GetScene()->Save("scene.json");
        }
        if (ImGui::MenuItem("Load Scene")) {
            m_App->OnSceneLoaded();
        }
        if (ImGui::MenuItem("Import Model")) {
            NFD::Guard nfdGuard;
            NFD::UniquePath outPath;
            nfdfilteritem_t filterItem[1] = { { "Wavefront OBJ", "obj" } };
            nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1);
            if (result == NFD_OKAY) {
                m_App->ImportModel(outPath.get());
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Settings...")) {
            m_App->SetShowSettings(true);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
            m_App->Exit();
        }
        ImGui::EndMenu();
    }
}

void MenuBar::DrawViewMenu() {
    if (ImGui::BeginMenu("View")) {
        bool show = m_App->GetShowAnchors();
        if (ImGui::MenuItem("Show Anchors", nullptr, &show)) {
            m_App->SetShowAnchors(show);
        }
        ImGui::EndMenu();
    }
}

void MenuBar::DrawSceneMenu() {
    if (ImGui::BeginMenu("Scene")) {
        DrawAddObjectSubMenu();
        ImGui::Separator();
        bool canDelete = (m_App->GetScene()->GetSelectedObject() != nullptr);
        if (ImGui::MenuItem("Delete Selected", "Delete", false, canDelete)) {
            if(auto* selected = m_App->GetScene()->GetSelectedObject()) {
                m_App->RequestObjectDeletion(selected->id);
            }
        }
        ImGui::EndMenu();
    }
}

void MenuBar::DrawAddObjectSubMenu() {
    if (ImGui::BeginMenu("Add")) {
        auto factory = m_App->GetObjectFactory();
        if (factory) {
            // Correctly call the new method to get only user-creatable types
            for (const auto& typeName : factory->GetUserCreatableTypeNames()) {
                if (ImGui::MenuItem(("Add " + typeName).c_str())) {
                    m_App->RequestObjectCreation(typeName);
                }
            }
        }
        ImGui::EndMenu();
    }
}