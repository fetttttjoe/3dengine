#include "Core/UI/HierarchyView.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Core/Application.h"
#include "Scene/Scene.h"

HierarchyView::HierarchyView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void HierarchyView::Draw() {
  if (ImGui::BeginTable(
          "object_list", 3,
          ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Dup##Mode", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Del##Mode", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableHeadersRow();

    for (auto& obj : m_Scene->GetSceneObjects()) {
      if (!obj || !obj->isSelectable) continue;
      uint32_t oid = obj->id;
      ImGui::PushID(int(oid));
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      // ... (Rename logic remains the same) ...
      if (oid == m_RenameID) {
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##rename", &m_RenameBuffer,
                             ImGuiInputTextFlags_EnterReturnsTrue |
                                 ImGuiInputTextFlags_AutoSelectAll)) {
          obj->name = m_RenameBuffer;
          m_RenameID = 0;
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
          obj->name = m_RenameBuffer;
          m_RenameID = 0;
        }
        ImGui::PopItemWidth();
      } else {
        bool sel = (obj.get() == m_Scene->GetSelectedObject());
        if (ImGui::Selectable(obj->name.c_str(), sel)) {
          m_App->SelectObject(oid);
        }
        if (ImGui::IsItemHovered() &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          m_RenameID = oid;
          m_RenameBuffer = obj->name;
          m_App->SelectObject(oid);
        }
      }

      ImGui::TableNextColumn();
      if (ImGui::Button("Dup")) {
        // 1. Request duplication instead of doing it directly
        m_App->RequestObjectDuplication(oid);
      }

      ImGui::TableNextColumn();
      if (ImGui::Button("Del")) {
        // 2. Request deletion instead of queueing it directly in the scene
        m_App->RequestObjectDeletion(oid);
      }

      ImGui::PopID();
    }
    ImGui::EndTable();
  }
}