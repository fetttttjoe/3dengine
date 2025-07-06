#include "Core/UI/InspectorView.h"

#include <Interfaces.h>
#include <imgui.h>
#include <imgui_stdlib.h>  // For ImGui::InputText with std::string

#include <glm/gtc/type_ptr.hpp>  // For glm::value_ptr

#include "Core/PropertyNames.h"  // For PropertyNames::Position etc.
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"

InspectorView::InspectorView(Scene* scene) : m_Scene(scene) {}

void InspectorView::Draw() {
  if (auto* sel = m_Scene->GetSelectedObject()) {
    ImGui::InputText("Name", &sel->name);
    ImGui::Text("Type: %s", sel->GetTypeString().c_str());
    ImGui::Text("ID: %u", sel->id);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto pos = sel->GetPosition();
      auto rot = sel->GetRotation();
      auto sca = sel->GetScale();
      auto eul = glm::degrees(glm::eulerAngles(rot));

      if (ImGui::DragFloat3(PropertyNames::Position, glm::value_ptr(pos), 0.1f))
        sel->SetPosition(pos);
      if (ImGui::DragFloat3(PropertyNames::Rotation, glm::value_ptr(eul), 1.0f))
        sel->SetEulerAngles(eul);
      if (ImGui::DragFloat3(PropertyNames::Scale, glm::value_ptr(sca), 0.1f))
        sel->SetScale(sca);
    }

    if (ImGui::CollapsingHeader("Other Properties",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      const auto& props = sel->GetPropertySet().GetProperties();
      if (props.empty()) {
        ImGui::Text("No editable properties.");
      } else {
        for (auto& p : props) {
          auto n = p->GetName();
          if (n == PropertyNames::Position || n == PropertyNames::Rotation ||
              n == PropertyNames::Scale)
            continue;
          p->DrawEditor();
        }
      }
    }
  } else {
    ImGui::TextDisabled("No object selected.");
  }
}