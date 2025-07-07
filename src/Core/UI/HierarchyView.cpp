#include "Core/UI/HierarchyView.h"

#include <imgui.h>
#include <imgui_stdlib.h>  // For ImGui::InputText with std::string

#include <algorithm>

#include "Core/Application.h"  // For access to SelectObject, GetTransformGizmo
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"  // For SetTarget(nullptr)

HierarchyView::HierarchyView(Application* app, Scene* scene)
    : m_App(app), m_Scene(scene) {}

void HierarchyView::Draw() {
  if (m_IdToDelete) {
    if (auto* sel = m_Scene->GetSelectedObject();
        sel && sel->id == m_IdToDelete)
      m_App->GetTransformGizmo()->SetTarget(nullptr);  // Deselect gizmo target
    m_Scene->DeleteObjectByID(m_IdToDelete);
    m_IdToDelete = 0;
    m_RenameID = 0;  // Clear rename state if deleted
    if (OnObjectDeleted) OnObjectDeleted(m_IdToDelete);  // Notify
  }

  uint32_t dupID = 0;
  if (ImGui::BeginTable(
          "object_list", 3,
          ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Dup", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("Del", ImGuiTableColumnFlags_WidthFixed, 65);
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
        if (ImGui::Selectable(obj->name.c_str(), sel)) {
          if (OnObjectSelected) OnObjectSelected(oid);
        }
        if (ImGui::IsItemHovered() &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          m_RenameID = oid;
          m_RenameBuffer = obj->name;
          if (OnObjectSelected)
            OnObjectSelected(oid);  // Also select on double-click
        }
      }

      ImGui::TableNextColumn();
      if (ImGui::Button("Dup##dup")) dupID = oid;

      ImGui::TableNextColumn();
      if (ImGui::Button("Del##del")) m_IdToDelete = oid;

      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  if (dupID && OnObjectDuplicated) OnObjectDuplicated(dupID);
}