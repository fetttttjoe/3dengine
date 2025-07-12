#include "Core/UI/InspectorView.h"
#include <Interfaces.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Application.h"
#include "Core/PropertyNames.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "implot.h"
#include "Core/UI/AppUI.h"

InspectorView::InspectorView(Application* app)
    : m_App(app), m_Scene(app->GetScene()) {}

void InspectorView::Draw() {
    ISceneObject* sel = m_Scene->GetSelectedObject();
    if (sel) {
        // Renaming the object requests a render.
        if(ImGui::InputText("Name", &sel->name)) {
            m_App->RequestSceneRender();
        }
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
    if (ImGui::CollapsingHeader("Transform##Header", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (m_App->GetEditorMode() == EditorMode::SCULPT) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            ImGui::BeginDisabled();
        }

        auto pos = sel->GetPosition();
        auto rot = sel->GetRotation();
        auto sca = sel->GetScale();
        auto eul = glm::degrees(glm::eulerAngles(rot));

        // Each of these setters will trigger the onChange callback in BaseObject,
        // which correctly calls RequestSceneRender().
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
            // The DrawEditor method for each property also uses the onChange callback.
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
        
        ImGui::Text("Mode:");
        ImGui::SameLine();
        // SetEditorMode correctly requests a re-render.
        if (ImGui::RadioButton("Transform##Mode", currentMode == EditorMode::TRANSFORM)) {
            m_App->SetEditorMode(EditorMode::TRANSFORM, m_BrushSettings.mode);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sculpt##Mode", currentMode == EditorMode::SCULPT)) {
            m_App->SetEditorMode(EditorMode::SCULPT, m_BrushSettings.mode);
        }

        if (currentMode == EditorMode::SCULPT) {
            ImGui::Separator();
            DrawBrushSettings();
        }
    }
}

void InspectorView::DrawBrushSettings() {
    bool settingsChanged = false;
    ImGui::Text("Tool:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Pull", m_BrushSettings.mode == SculptMode::Pull)) { m_BrushSettings.mode = SculptMode::Pull; settingsChanged = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Push", m_BrushSettings.mode == SculptMode::Push)) { m_BrushSettings.mode = SculptMode::Push; settingsChanged = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Smooth", m_BrushSettings.mode == SculptMode::Smooth)) { m_BrushSettings.mode = SculptMode::Smooth; settingsChanged = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Grab", m_BrushSettings.mode == SculptMode::Grab)) { m_BrushSettings.mode = SculptMode::Grab; settingsChanged = true; }
    
    ImGui::Separator();
    ImGui::Text("Brush Settings");
    if(ImGui::DragFloat("Radius##Sculpt", &m_BrushSettings.radius, 0.01f, 0.01f, 5.0f)) settingsChanged = true;
    if(ImGui::DragFloat("Strength##Sculpt", &m_BrushSettings.strength, 0.01f, 0.01f, 1.0f)) settingsChanged = true;

    ImGui::Separator();
    ImGui::Text("Brush Falloff");

    if (ImPlot::BeginPlot("##FalloffCurve", ImVec2(-1, 150), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoTitle)) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
        ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);

        auto& points = m_BrushSettings.falloff.GetPoints();
        std::vector<double> xs, ys;
        for(const auto& p : points) {
            xs.push_back(p.pos.x);
            ys.push_back(p.pos.y);
        }

        ImPlot::PlotLine("Falloff", xs.data(), ys.data(), xs.size());
        
        for (size_t i = 0; i < points.size(); ++i) {
            double x = points[i].pos.x;
            double y = points[i].pos.y;
            if (ImPlot::DragPoint((int)i, &x, &y, ImVec4(0,0.9f,0,1), 4)) {
                points[i].pos.x = glm::clamp((float)x, 0.0f, 1.0f);
                points[i].pos.y = glm::clamp((float)y, 0.0f, 1.0f);
                m_BrushSettings.falloff.SortPoints();
                settingsChanged = true;
            }
        }

        ImPlot::EndPlot();
    }
    
    // One single check at the end to request a render if any brush setting was changed.
    if (settingsChanged) {
        m_App->RequestSceneRender();
    }
}