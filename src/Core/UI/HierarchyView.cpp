#include "Core/UI/HierarchyView.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "Core/Application.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"

HierarchyView::HierarchyView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void HierarchyView::Draw() {
  uint32_t idToDelete = 0;
  uint32_t idToDuplicate = 0;

  if (ImGui::BeginTable(
          "object_list", 3,
          ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Dup##Mode", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Del##Mode", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableHeadersRow();

    for (auto& obj : m_Scene->GetSceneObjects()) {
      if (!obj->isSelectable) continue;
      uint32_t oid = obj->id;
      ImGui::PushID(int(oid));
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      if (oid == m_RenameID) {
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##rename", &m_RenameBuffer,
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
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
        // FIX: Removed ImGuiSelectableFlags_SpanAllColumns to prevent text from
        // overflowing onto the buttons in the other columns.
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
        idToDuplicate = oid;
      }

      ImGui::TableNextColumn();
      if (ImGui::Button("Del")) {
        idToDelete = oid;
      }

      ImGui::PopID();
    }
    ImGui::EndTable();
  }

  if (idToDuplicate != 0) {
    m_Scene->DuplicateObject(idToDuplicate);
  }
  if (idToDelete != 0) {
    m_Scene->DeleteObjectByID(idToDelete);
  }
}