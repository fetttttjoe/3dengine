#include "Core/UI/MenuBar.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Core/Application.h"
#include "Core/PropertyNames.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"

MenuBar::MenuBar(Application* app, Scene* scene, SceneObjectFactory* factory)
    : m_App(app), m_Scene(scene), m_Factory(factory) {}

void MenuBar::Draw() {
  if (!ImGui::BeginMainMenuBar()) return;
  DrawFileMenu();
  DrawViewMenu();
  DrawSceneMenu();
  ImGui::EndMainMenuBar();
}

void MenuBar::DrawFileMenu() {
  if (!ImGui::BeginMenu("File")) return;
  if (ImGui::MenuItem("Save Scene") && OnSaveScene) OnSaveScene();
  if (ImGui::MenuItem("Load Scene") && OnLoadScene) OnLoadScene();
  ImGui::Separator();
  if (ImGui::MenuItem("Settings…") && OnShowSettings) OnShowSettings();
  ImGui::Separator();
  if (ImGui::MenuItem("Exit") && OnExitRequested) OnExitRequested();
  ImGui::EndMenu();
}

void MenuBar::DrawViewMenu() {
  if (!ImGui::BeginMenu("View")) return;
  bool show = m_App->GetShowAnchors();
  if (ImGui::MenuItem("Show Anchors", nullptr, &show)) {
    if (OnShowAnchorsChanged) OnShowAnchorsChanged(show);
  }
  ImGui::EndMenu();
}

void MenuBar::DrawSceneMenu() {
  if (!ImGui::BeginMenu("Scene")) return;
  DrawAddObjectSubMenu();
  ImGui::Separator();
  bool canDelete = (m_Scene->GetSelectedObject() != nullptr);
  if (ImGui::MenuItem("Delete Selected", nullptr, canDelete)) {
    if (canDelete && OnDeleteSelectedObject) {
      OnDeleteSelectedObject();  // Call the void() callback
    }
  }
  ImGui::EndMenu();
}

void MenuBar::DrawAddObjectSubMenu() {
  if (!m_Factory) return;
  if (!ImGui::BeginMenu("Add")) return;
  for (auto& t : m_Factory->GetRegisteredTypeNames()) {
    if (t == ObjectTypes::Grid) continue;
    if (ImGui::MenuItem(("Add " + t).c_str())) {
      if (OnAddObject) OnAddObject(t);
    }
  }
  ImGui::EndMenu();
}

// Definition of the GetFactory method
SceneObjectFactory* MenuBar::GetFactory() const { return m_Factory; }