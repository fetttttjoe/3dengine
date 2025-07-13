#include "Core/UI/InspectorView.h"

#include <Interfaces.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/PropertyNames.h"
#include "Core/UI/AppUI.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "Sculpting/SubObjectSelection.h"
#include "implot.h"

InspectorView::InspectorView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void InspectorView::Draw() {
  ISceneObject* sel = m_Scene->GetSelectedObject();
  if (sel) {
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##Name", &sel->name)) {
      m_App->RequestSceneRender();
    }
    ImGui::PopItemWidth();
    ImGui::Text("Type: %s", sel->GetTypeString().c_str());
    ImGui::Text("ID: %u", sel->id);
    ImGui::Separator();

    DrawTransformControls(sel);
    DrawProperties(sel);

    if (ImGui::CollapsingHeader("Mesh Editing", ImGuiTreeNodeFlags_DefaultOpen)) {
        EditorMode currentEditorMode = m_App->GetEditorMode();

        ImGui::BeginGroup();
        if (ImGui::RadioButton("Transform", currentEditorMode == EditorMode::TRANSFORM)) {
            m_App->SetEditorMode(EditorMode::TRANSFORM);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sculpt", currentEditorMode == EditorMode::SCULPT)) {
            m_App->SetEditorMode(EditorMode::SCULPT);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sub-Object", currentEditorMode == EditorMode::SUB_OBJECT)) {
            m_App->SetEditorMode(EditorMode::SUB_OBJECT);
        }
        ImGui::EndGroup();

        if (sel->GetEditableMesh()) {
            if (currentEditorMode == EditorMode::SCULPT) {
                ImGui::Separator();
                DrawBrushSettings();
            } else if (currentEditorMode == EditorMode::SUB_OBJECT) {
                ImGui::Separator();
                DrawSubObjectSettings();
            }
        } else {
            if (currentEditorMode == EditorMode::SCULPT || currentEditorMode == EditorMode::SUB_OBJECT) {
                ImGui::TextDisabled("Selected object is not editable.");
                if (currentEditorMode != EditorMode::TRANSFORM) {
                    m_App->SetEditorMode(EditorMode::TRANSFORM);
                }
            }
        }
    }

  } else {
    ImGui::TextDisabled("No object selected.");
  }
}

void InspectorView::DrawTransformControls(ISceneObject* sel) {
  if (ImGui::CollapsingHeader("Transform##Header", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_App->GetEditorMode() == EditorMode::SCULPT || m_App->GetEditorMode() == EditorMode::SUB_OBJECT) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      ImGui::BeginDisabled();
    }

    auto pos = sel->GetPosition();
    auto rot = sel->GetRotation();
    auto sca = sel->GetScale();
    auto eul = glm::degrees(glm::eulerAngles(rot));

    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat3(PropertyNames::Position, glm::value_ptr(pos), 0.1f))
      sel->SetPosition(pos);
    if (ImGui::DragFloat3(PropertyNames::Rotation, glm::value_ptr(eul), 1.0f))
      sel->SetEulerAngles(eul);
    if (ImGui::DragFloat3(PropertyNames::Scale, glm::value_ptr(sca), 0.1f))
      sel->SetScale(sca);
    ImGui::PopItemWidth();

    if (m_App->GetEditorMode() == EditorMode::SCULPT || m_App->GetEditorMode() == EditorMode::SUB_OBJECT) {
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
      ImGui::PushItemWidth(-1);
      p->DrawEditor();
      ImGui::PopItemWidth();
      ImGui::PopID();
      hasEditableProps = true;
    }
    if (!hasEditableProps) {
      ImGui::TextDisabled("No other editable properties.");
    }
  }
}

void InspectorView::DrawBrushSettings() {
  bool settingsChanged = false;
  ImGui::Text("Brush Tool:");
  ImGui::BeginGroup();
  if (ImGui::RadioButton("Pull", m_BrushSettings.mode == SculptMode::Pull)) {
    m_BrushSettings.mode = SculptMode::Pull;
    settingsChanged = true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Push", m_BrushSettings.mode == SculptMode::Push)) {
    m_BrushSettings.mode = SculptMode::Push;
    settingsChanged = true;
  }
    ImGui::SameLine();
  if (ImGui::RadioButton("Smooth", m_BrushSettings.mode == SculptMode::Smooth)) {
    m_BrushSettings.mode = SculptMode::Smooth;
    settingsChanged = true;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Grab", m_BrushSettings.mode == SculptMode::Grab)) {
    m_BrushSettings.mode = SculptMode::Grab;
    settingsChanged = true;
  }
  ImGui::EndGroup();

  ImGui::Separator();
  ImGui::Text("Brush Settings");
  ImGui::PushItemWidth(-1);
  if (ImGui::DragFloat("Radius##Sculpt", &m_BrushSettings.radius, 0.01f, 0.01f, 5.0f))
    settingsChanged = true;
  if (ImGui::DragFloat("Strength##Sculpt", &m_BrushSettings.strength, 0.01f, 0.01f, 1.0f))
    settingsChanged = true;
  ImGui::PopItemWidth();

  ImGui::Separator();
  ImGui::Text("Brush Falloff");

  if (ImPlot::BeginPlot("##FalloffCurve", ImVec2(-1, 150), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoTitle)) {
    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
    ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);

    auto& points = m_BrushSettings.falloff.GetPoints();
    std::vector<double> xs, ys;
    for (const auto& p : points) {
      xs.push_back(p.pos.x);
      ys.push_back(p.pos.y);
    }

    ImPlot::PlotLine("Falloff", xs.data(), ys.data(), xs.size());

    for (size_t i = 0; i < points.size(); ++i) {
      double x = points[i].pos.x;
      double y = points[i].pos.y;
      if (ImPlot::DragPoint((int)i, &x, &y, ImVec4(0, 0.9f, 0, 1), 4)) {
        points[i].pos.x = glm::clamp((float)x, 0.0f, 1.0f);
        points[i].pos.y = glm::clamp((float)y, 0.0f, 1.0f);
        m_BrushSettings.falloff.SortPoints();
        settingsChanged = true;
      }
    }
    ImPlot::EndPlot();
  }

  if (settingsChanged) {
    m_App->SetEditorMode(EditorMode::SCULPT, m_BrushSettings.mode);
  }
}

void InspectorView::DrawSubObjectSettings() {
    SubObjectMode currentSubObjectMode = m_App->GetSubObjectMode();

    ImGui::Text("Selection Mode:");
    ImGui::BeginGroup();
    if (ImGui::RadioButton("Vertices", currentSubObjectMode == SubObjectMode::VERTEX)) {
        m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::VERTEX);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Edges", currentSubObjectMode == SubObjectMode::EDGE)) {
        m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::EDGE);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Faces", currentSubObjectMode == SubObjectMode::FACE)) {
        m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::FACE);
    }
    ImGui::EndGroup();
    
    ImGui::Separator();
    ImGui::Text("Operations");
    
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float dragWidth = availableWidth * 0.4f; // Allocate 40% of space to the drag float
    float buttonWidth = availableWidth - dragWidth - ImGui::GetStyle().ItemSpacing.x;
    if (buttonWidth < 50) { // If space is too tight, stack them
        dragWidth = -1;
        buttonWidth = -1;
    }

    if (currentSubObjectMode == SubObjectMode::VERTEX) {
        ImGui::PushItemWidth(dragWidth);
        ImGui::DragFloat("##MoveDist", &m_MoveDistance, 0.01f, -1.0f, 1.0f, "%.2f");
        ImGui::PopItemWidth();
        if (dragWidth != -1) ImGui::SameLine();
        if (ImGui::Button("Move Along Normal", ImVec2(buttonWidth, 0))) {
            m_App->RequestMoveSelection(m_MoveDistance);
        }
        if (ImGui::Button("Weld Vertices", ImVec2(-1, 0))) {
            m_App->RequestWeld();
        }
    } else if (currentSubObjectMode == SubObjectMode::FACE) {
        ImGui::PushItemWidth(dragWidth);
        ImGui::DragFloat("##ExtrudeDist", &m_ExtrudeDistance, 0.01f, 0.0f, 10.0f, "%.2f");
        ImGui::PopItemWidth();
        if (dragWidth != -1) ImGui::SameLine();
        if (ImGui::Button("Extrude Face", ImVec2(buttonWidth, 0))) {
            m_App->RequestExtrude(m_ExtrudeDistance);
        }
    } else if (currentSubObjectMode == SubObjectMode::EDGE) {
        ImGui::TextDisabled("Edge editing is not yet implemented.");
    }

    ImGui::Separator();
    ImGui::TextDisabled("Shift+Click to multi-select.");
    ImGui::TextDisabled("Click empty space to deselect all.");
}