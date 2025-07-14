#include "Core/UI/InspectorView.h"

#include <Interfaces.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/PropertyNames.h"
#include "Core/UI/AppUI.h"
#include "Core/UI/UIElements.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "Sculpting/SubObjectSelection.h"
#include "implot.h"

InspectorView::InspectorView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void InspectorView::Draw() {
    ISceneObject* sel = m_Scene->GetSelectedObject();
    if (!sel) {
        ImGui::TextDisabled("No object selected.");
        return;
    }

    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##Name", &sel->name)) {
        m_App->RequestSceneRender();
    }
    ImGui::PopItemWidth();
    ImGui::Text("Type: %s", sel->GetTypeString().c_str());
    ImGui::Text("ID: %u", sel->id);
    ImGui::Separator();

    DrawTransformControls(sel);
    DrawMaterialControls(sel);
    DrawProperties(sel);
    DrawMeshEditingControls(sel);
}

void InspectorView::DrawTransformControls(ISceneObject* sel) {
  if (ImGui::CollapsingHeader("Transform##Header", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool isLocked = (m_App->GetEditorMode() == EditorMode::SCULPT || m_App->GetEditorMode() == EditorMode::SUB_OBJECT);
    if (isLocked) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      ImGui::BeginDisabled();
    }

    auto pos = sel->GetPosition();
    auto rot = sel->GetRotation();
    auto sca = sel->GetScale();
    auto eul = glm::degrees(glm::eulerAngles(rot));

    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat3(PropertyNames::Position, glm::value_ptr(pos), 0.1f)) sel->SetPosition(pos);
    if (ImGui::DragFloat3(PropertyNames::Rotation, glm::value_ptr(eul), 1.0f)) sel->SetEulerAngles(eul);
    if (ImGui::DragFloat3(PropertyNames::Scale, glm::value_ptr(sca), 0.1f)) sel->SetScale(sca);
    ImGui::PopItemWidth();

    if (isLocked) {
      ImGui::EndDisabled();
      ImGui::PopStyleVar();
    }
  }
}

void InspectorView::DrawMaterialControls(ISceneObject* sel) {
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (auto* colorProp = sel->GetPropertySet().GetProperty(PropertyNames::Color)) {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
            colorProp->DrawEditor();
            ImGui::PopItemWidth();
        }
    }
}

void InspectorView::DrawProperties(ISceneObject* sel) {
  if (ImGui::CollapsingHeader("Object Properties")) {
    const auto& props = sel->GetPropertySet().GetProperties();
    bool hasEditableProps = false;
    
    ImGui::Columns(2, "object_properties_cols", false);
    ImGui::SetColumnWidth(0, 80);

    for (auto& p : props) {
      const auto& n = p->GetName();
      if (n == PropertyNames::Position || n == PropertyNames::Rotation ||
          n == PropertyNames::Scale || n == PropertyNames::Color)
        continue;

      hasEditableProps = true;
      ImGui::Text("%s", n.c_str());
      ImGui::NextColumn();
      ImGui::PushID(n.c_str());
      ImGui::PushItemWidth(-1);
      p->DrawEditor();
      ImGui::PopItemWidth();
      ImGui::PopID();
      ImGui::NextColumn();
    }
    ImGui::Columns(1);

    if (!hasEditableProps) {
      ImGui::TextDisabled("No other editable properties.");
    }
  }
}

void InspectorView::DrawMeshEditingControls(ISceneObject* sel) {
    if (!sel->GetEditableMesh()) {
        return;
    }

    if (ImGui::CollapsingHeader("Mesh Editing", ImGuiTreeNodeFlags_DefaultOpen)) {
        EditorMode currentEditorMode = m_App->GetEditorMode();

        ImGui::Text("Mode:");
        ImGui::BeginGroup();
        if (ImGui::RadioButton("Transform", currentEditorMode == EditorMode::TRANSFORM)) m_App->SetEditorMode(EditorMode::TRANSFORM);
        ImGui::SameLine();
        if (ImGui::RadioButton("Sculpt", currentEditorMode == EditorMode::SCULPT)) m_App->SetEditorMode(EditorMode::SCULPT);
        ImGui::SameLine();
        if (ImGui::RadioButton("Sub-Object", currentEditorMode == EditorMode::SUB_OBJECT)) m_App->SetEditorMode(EditorMode::SUB_OBJECT);
        ImGui::EndGroup();

        if (currentEditorMode == EditorMode::SCULPT) {
            ImGui::SeparatorText("Sculpting Tools");
            DrawBrushSettings();
        } else if (currentEditorMode == EditorMode::SUB_OBJECT) {
            ImGui::SeparatorText("Sub-Object Tools");
            DrawSubObjectSettings();
        }
    }
}

bool InspectorView::CanWeld() const {
    return m_App->GetSelection()->GetSelectedVertices().size() >= 2;
}

bool InspectorView::CanExtrude() const {
    return !m_App->GetSelection()->GetSelectedFaces().empty();
}

bool InspectorView::CanBevel() const {
    return !m_App->GetSelection()->GetSelectedEdges().empty();
}

void InspectorView::DrawSubObjectSettings() {
    SubObjectMode currentSubObjectMode = m_App->GetSubObjectMode();

    ImGui::Text("Selection:");
    ImGui::BeginGroup();
    if (ImGui::RadioButton("Vertices", currentSubObjectMode == SubObjectMode::VERTEX)) m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::VERTEX);
    ImGui::SameLine(0, 10);
    if (ImGui::RadioButton("Edges", currentSubObjectMode == SubObjectMode::EDGE)) m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::EDGE);
    ImGui::SameLine(0, 10);
    if (ImGui::RadioButton("Faces", currentSubObjectMode == SubObjectMode::FACE)) m_App->SetEditorMode(EditorMode::SUB_OBJECT, SculptMode::Pull, SubObjectMode::FACE);
    ImGui::EndGroup();
    
    bool ignoreBackfaces = m_App->GetSelection()->GetIgnoreBackfaces();
    if (ImGui::Checkbox("Select Visible Only", &ignoreBackfaces)) {
        m_App->GetSelection()->SetIgnoreBackfaces(ignoreBackfaces);
    }
    
    ImGui::Separator();
    
    if (currentSubObjectMode == SubObjectMode::VERTEX) {
        ImGui::Text("Vertex Tools");
        if (UIElements::Button("Weld Vertices", CanWeld())) m_App->RequestWeld();
        if (!CanWeld()) ImGui::TextDisabled("Select >= 2 vertices to weld.");
    }

    if (currentSubObjectMode == SubObjectMode::EDGE) {
        ImGui::Text("Edge Tools");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        ImGui::DragFloat("##BevelAmount", &m_BevelAmount, 0.01f, 0.01f, 1.0f, "Bevel: %.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (UIElements::Button("Apply Bevel", CanBevel())) m_App->RequestBevelEdge(m_BevelAmount);
    }

    if (currentSubObjectMode == SubObjectMode::FACE) {
        ImGui::Text("Face Tools");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        ImGui::DragFloat("##ExtrudeDist", &m_ExtrudeDistance, 0.01f, 0.0f, 10.0f, "Extrude: %.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (UIElements::Button("Apply Extrude", CanExtrude())) m_App->RequestExtrude(m_ExtrudeDistance);
    }

    ImGui::Separator();
    ImGui::TextDisabled("Shift+Click to multi-select.");
    ImGui::TextDisabled("Select 2+ vertices for path highlight.");
}

void InspectorView::DrawBrushSettings() {
  bool settingsChanged = false;
  ImGui::Text("Brush Tool:");
  ImGui::BeginGroup();
  if (ImGui::RadioButton("Pull", m_BrushSettings.mode == SculptMode::Pull)) { m_BrushSettings.mode = SculptMode::Pull; settingsChanged = true; }
  ImGui::SameLine();
  if (ImGui::RadioButton("Push", m_BrushSettings.mode == SculptMode::Push)) { m_BrushSettings.mode = SculptMode::Push; settingsChanged = true; }
  ImGui::SameLine();
  if (ImGui::RadioButton("Smooth", m_BrushSettings.mode == SculptMode::Smooth)) { m_BrushSettings.mode = SculptMode::Smooth; settingsChanged = true; }
  ImGui::SameLine();
  if (ImGui::RadioButton("Grab", m_BrushSettings.mode == SculptMode::Grab)) { m_BrushSettings.mode = SculptMode::Grab; settingsChanged = true; }
  ImGui::EndGroup();

  ImGui::Separator();
  ImGui::Text("Brush Settings");
  ImGui::PushItemWidth(-1);
  if (ImGui::DragFloat("Radius##Sculpt", &m_BrushSettings.radius, 0.01f, 0.01f, 5.0f)) settingsChanged = true;
  if (ImGui::DragFloat("Strength##Sculpt", &m_BrushSettings.strength, 0.01f, 0.01f, 1.0f)) settingsChanged = true;
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