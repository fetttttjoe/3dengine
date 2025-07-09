#include "Core/UI/InspectorView.h"

#include <Interfaces.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/PropertyNames.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"

InspectorView::InspectorView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void InspectorView::Draw() {
  ISceneObject* sel = m_Scene->GetSelectedObject();
  if (sel) {
    ImGui::InputText("Name", &sel->name);
    ImGui::Text("Type: %s", sel->GetTypeString().c_str());
    ImGui::Text("ID: %u", sel->id);
    ImGui::Separator();

    DrawTransformControls(sel);
    DrawProperties(sel);

    if (sel->GetSculptableMesh()) {
      DrawSculptControls(sel);
    }

  } else {
    ImGui::TextDisabled("No object selected.");
  }
}

void InspectorView::DrawTransformControls(ISceneObject* sel) {
  // This header's label is "Transform"
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_App->GetEditorMode() == EditorMode::SCULPT) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      ImGui::BeginDisabled();
    }

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

    if (m_App->GetEditorMode() == EditorMode::SCULPT) {
      ImGui::EndDisabled();
      ImGui::PopStyleVar();
    }
  }
}

void InspectorView::DrawProperties(ISceneObject* sel) {
  if (ImGui::CollapsingHeader("Properties")) {
    const auto& props = sel->GetPropertySet().GetProperties();
    bool hasEditableProps = false;
    for (auto& p : props) {
      auto n = p->GetName();
      if (n == PropertyNames::Position || n == PropertyNames::Rotation ||
          n == PropertyNames::Scale)
        continue;

      ImGui::PushID(n.c_str());
      p->DrawEditor();
      ImGui::PopID();
      hasEditableProps = true;
    }
    if (!hasEditableProps) {
      ImGui::TextDisabled("No other editable properties.");
    }
  }
}

void InspectorView::DrawSculptControls(ISceneObject* sel) {
  if (ImGui::CollapsingHeader("Sculpting", ImGuiTreeNodeFlags_DefaultOpen)) {
    EditorMode currentMode = m_App->GetEditorMode();

    // Mode Selection
    ImGui::Text("Mode:");
    ImGui::SameLine();
    // FIX: Added "##Mode" to create a unique ID, resolving the conflict with
    // the "Transform" header.
    if (ImGui::RadioButton("Transform##Mode",
                           currentMode == EditorMode::TRANSFORM)) {
      m_App->SetEditorMode(EditorMode::TRANSFORM, m_App->GetSculptMode());
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Sculpt##Mode", currentMode == EditorMode::SCULPT)) {
      m_App->SetEditorMode(EditorMode::SCULPT, m_App->GetSculptMode());
    }

    if (currentMode == EditorMode::SCULPT) {
      ImGui::Separator();

      // Tool Selection
      ImGui::Text("Tool:");
      ImGui::SameLine();
      SculptMode::Mode sculptMode = m_App->GetSculptMode();
      if (ImGui::RadioButton("Pull", sculptMode == SculptMode::Pull)) {
        m_App->SetEditorMode(EditorMode::SCULPT, SculptMode::Pull);
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Push", sculptMode == SculptMode::Push)) {
        m_App->SetEditorMode(EditorMode::SCULPT, SculptMode::Push);
      }

      ImGui::Separator();
      ImGui::Text("Brush Settings");
      ImGui::DragFloat("Brush Radius##Sculpt", &m_BrushRadius, 0.01f, 0.01f,
                       5.0f);
      ImGui::DragFloat("Strength##Sculpt", &m_BrushStrength, 0.01f, 0.01f,
                       1.0f);
    }
  }
}