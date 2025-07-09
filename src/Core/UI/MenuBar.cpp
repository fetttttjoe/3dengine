#include "Core/UI/MenuBar.h"

#include <imgui.h>

#include "Core/Application.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"

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
      m_App->GetScene()->Load("scene.json");
      m_App->OnSceneLoaded();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Settings…")) {
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
      m_App->GetScene()->DeleteSelectedObject();
    }
    ImGui::EndMenu();
  }
}

void MenuBar::DrawAddObjectSubMenu() {
  if (ImGui::BeginMenu("Add")) {
    auto factory = m_App->GetObjectFactory();
    if (factory) {
      for (auto& typeName : factory->GetRegisteredTypeNames()) {
        if (typeName == ObjectTypes::Grid) continue;
        if (ImGui::MenuItem(("Add " + typeName).c_str())) {
          m_App->GetScene()->AddObject(factory->Create(typeName));
        }
      }
    }
    ImGui::EndMenu();
  }
}
