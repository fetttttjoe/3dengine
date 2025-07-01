#include "Core/UI.h"
#include "Scene/Scene.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp> // For glm::decompose
#include <iostream> // For debug output

UI::UI(Scene* scene) : m_Scene(scene), m_Factory(nullptr) {}
UI::~UI() {}

void UI::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
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
            if (ImGui::MenuItem("Exit")) { /* Should post an exit event or set a flag */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            if (m_Factory) {
                if (ImGui::MenuItem("Add Triangle")) {
                    m_Scene->AddObject(m_Factory->Create("Triangle"));
                }
                if (ImGui::MenuItem("Add Pyramid")) {
                    m_Scene->AddObject(m_Factory->Create("Pyramid"));
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Selected", "Del")) {
                m_Scene->DeleteSelectedObject();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UI::DrawSceneOutliner() {
    ImGui::Begin("Scene Outliner");

    const auto& objects = m_Scene->GetSceneObjects();
    // Use a temporary vector to avoid issues with modifying m_Objects while iterating
    // (though deletion/duplication might require specific scene logic to handle indices)
    // For simplicity, direct iteration and action on m_Scene is shown.
    // In a production app, these actions would queue up to be processed after the UI pass.

    for (int i = 0; i < objects.size(); ++i) {
        // Skip grid from the outliner if it's not meant to be selectable like other objects
        if (dynamic_cast<Grid*>(objects[i].get())) {
            continue;
        }

        bool isSelected = (objects[i].get() == m_Scene->GetSelectedObject());
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (isSelected) {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        // Display the object's name. Use PushID to ensure unique ImGui IDs for context menus.
        ImGui::PushID(objects[i]->id);
        ImGui::TreeNodeEx((void*)(intptr_t)objects[i]->id, nodeFlags, "%s", objects[i]->name.c_str());

        // Handle selection on click
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_Scene->SetSelectedObjectByID(objects[i]->id);
        }

        // Right-click context menu for each item
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Duplicate")) {
                // Implement duplication logic in Scene.h/cpp
                m_Scene->DuplicateObject(objects[i]->id);
            }
            if (ImGui::MenuItem("Delete")) {
                m_Scene->DeleteObjectByID(objects[i]->id);
                // After deleting, ensure no object is selected if the deleted one was selected
                if (isSelected) {
                    m_Scene->SetSelectedObjectByID(0); // Deselect
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::End();
}

void UI::DrawPropertiesPanel() {
    // Flag to track if the properties panel is closed by the user
    bool propertiesPanelOpen = true; // Assume open by default
    ImGui::Begin("Properties", &propertiesPanelOpen); // Pass pointer to control visibility

    ISceneObject* selected = m_Scene->GetSelectedObject();

    if (!propertiesPanelOpen) {
        // If the window was closed by the user (by clicking the 'X' button)
        if (selected) {
            m_Scene->SetSelectedObjectByID(0); // Deselect the item
        }
        ImGui::End();
        return; // Exit early as the window is closed
    }

    if (selected) {
        ImGui::Text("Selected: %s (ID: %u)", selected->name.c_str(), selected->id); // Show ID for debugging
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
            // Reconstruct the transform matrix in the correct order: scale, rotate, translate
            // Note: glm::translate multiplies from the left, so translate * rotation * scale
            glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), position);
            newTransform *= glm::mat4_cast(glm::quat(glm::radians(eulerRotation))); // Convert Euler back to Quaternion then to Matrix
            newTransform = glm::scale(newTransform, scale); // Scale applies to the local axes before rotation/translation
            selected->transform = newTransform;
        }

        auto properties = selected->GetProperties();
        if (!properties.empty()) {
            ImGui::Separator();
            ImGui::Text("Shape Properties");
            bool propertyChanged = false;
            for (auto& prop : properties) {
                // Ensure the value pointer is valid before attempting to use it
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