#include "Core/UI/UI.h"
#include "Core/Log.h"                  // Include the logger
#include "Core/UI/UIElements.h"           // Include the new generic UI elements
#include "Scene/Scene.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>

UI::UI(Scene* scene) : m_Scene(scene), m_Factory(nullptr) {}
UI::~UI() {}

void UI::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame() {
    ImGui::Render();
}

void UI::SetObjectFactory(SceneObjectFactory* factory) {
    m_Factory = factory;
}

void UI::DrawUI() {
    DrawMainMenu();
    DrawSceneOutliner();
    DrawPropertiesPanel();
}

void UI::DrawMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            UIElements::ActionMenuItem("Exit", []() { 
                Log::Debug("Menu Action: Exit clicked.");
                //glfwSetWindowShouldClose(window, true); // Future implementation
            });
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            UIElements::ActionMenuItem("Add Triangle", [this]() {
                Log::Debug("Menu Action: Add Triangle.");
                m_Scene->AddObject(m_Factory->Create("Triangle"));
            }, m_Factory != nullptr);

            UIElements::ActionMenuItem("Add Pyramid", [this]() {
                Log::Debug("Menu Action: Add Pyramid.");
                m_Scene->AddObject(m_Factory->Create("Pyramid"));
            }, m_Factory != nullptr);
            
            ImGui::Separator();
            
            UIElements::ActionMenuItem("Delete Selected", [this]() {
                Log::Debug("Menu Action: Delete Selected.");
                m_Scene->DeleteSelectedObject();
            });
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UI::DrawSceneOutliner() {
    ImGui::Begin("Scene Outliner");

    uint32_t idToDelete = 0;
    uint32_t idToDuplicate = 0;

    if (ImGui::BeginTable("object_list", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##Duplicate", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_WidthFixed, 70.0f);

        const auto& objects = m_Scene->GetSceneObjects();
        for (const auto& object : objects) {
            if (dynamic_cast<Grid*>(object.get())) { continue; }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            
            bool isSelected = (object.get() == m_Scene->GetSelectedObject());
            // FIX: Removed ImGuiSelectableFlags_SpanAllColumns to fix button clicking
            if (ImGui::Selectable(object->name.c_str(), isSelected, ImGuiSelectableFlags_None)) {
                m_Scene->SetSelectedObjectByID(object->id);
            }

            ImGui::PushID(object->id);
            ImGui::TableNextColumn();

            if (UIElements::SmallButton("Duplicate")) {
                idToDuplicate = object->id;
                Log::Debug("UI Action: Queued duplication for object ID ", idToDuplicate);
            }

            ImGui::TableNextColumn();

            if (UIElements::SmallButton("Delete")) {
                idToDelete = object->id;
                Log::Debug("UI Action: Queued deletion for object ID ", idToDelete);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (idToDuplicate != 0) { m_Scene->DuplicateObject(idToDuplicate); }
    if (idToDelete != 0) { m_Scene->DeleteObjectByID(idToDelete); }

    ImGui::End();
}

void UI::DrawPropertiesPanel() {
    bool propertiesPanelOpen = true; 
    ImGui::Begin("Properties", &propertiesPanelOpen);

    ISceneObject* selected = m_Scene->GetSelectedObject();

    if (!propertiesPanelOpen && selected) {
        m_Scene->SetSelectedObjectByID(0);
        ImGui::End();
        return;
    }

    if (selected) {
        ImGui::Text("Selected: %s (ID: %u)", selected->name.c_str(), selected->id);
        ImGui::Separator();

        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(selected->transform, scale, rotation, position, skew, perspective);
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(rotation));

        bool changed = false;
        changed |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
        changed |= ImGui::DragFloat3("Rotation", glm::value_ptr(eulerRotation), 1.0f);
        changed |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f);

        if (changed) {
            glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), position);
            newTransform *= glm::mat4_cast(glm::quat(glm::radians(eulerRotation)));
            newTransform = glm::scale(newTransform, scale);
            selected->transform = newTransform;
        }

        auto properties = selected->GetProperties();
        if (!properties.empty()) {
            ImGui::Separator();
            ImGui::Text("Shape Properties");
            bool propertyChanged = false;
            for (auto& prop : properties) {
                if (prop.value) {
                    propertyChanged |= ImGui::DragFloat(prop.name.c_str(), prop.value, 0.05f);
                } else {
                    ImGui::Text("Error: Property '%s' has null value pointer!", prop.name.c_str());
                }
            }
            if (propertyChanged) {
                selected->RebuildMesh();
            }
        }
    } else {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}