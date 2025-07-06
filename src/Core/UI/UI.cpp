#include "Core/UI/UI.h"
#include "Core/Log.h"
#include "Core/UI/UIElements.h"
#include "Scene/Scene.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "Scene/Objects/BaseObject.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h" // This header provides the std::string overload for ImGui::InputText
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstring>

UI::UI(Scene* scene)
  : m_Scene(scene)
  , m_Factory(nullptr)
  , m_RenameID(0)
  , m_RenameBuffer()
{}

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
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        UIElements::ActionMenuItem("Exit", []() {
            Log::Debug("Menu Action: Exit clicked.");
            // This needs to be hooked up to the Application to close the window.
        });
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scene")) {
        UIElements::ActionMenuItem("Add Triangle", [this]() {
            Log::Debug("Menu Action: Add Triangle.");
            if (m_Factory) m_Scene->AddObject(m_Factory->Create("Triangle"));
        }, m_Factory != nullptr);

        UIElements::ActionMenuItem("Add Pyramid", [this]() {
            Log::Debug("Menu Action: Add Pyramid.");
            if (m_Factory) m_Scene->AddObject(m_Factory->Create("Pyramid"));
        }, m_Factory != nullptr);

        ImGui::Separator();

        UIElements::ActionMenuItem("Delete Selected", [this]() {
            Log::Debug("Menu Action: Delete Selected.");
            m_Scene->DeleteSelectedObject();
        }, m_Scene->GetSelectedObject() != nullptr);

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::DrawSceneOutliner() {
    ImGui::Begin("Scene Outliner");

    uint32_t idToDelete = 0;
    uint32_t idToDuplicate = 0;

    if (ImGui::BeginTable("object_list", 4,
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Name",       ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Rename",     ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Duplicate",  ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Delete",     ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableHeadersRow();

        const auto& objects = m_Scene->GetSceneObjects();
        for (const auto& objPtr : objects) {
            if (!objPtr->isSelectable) continue;

            uint32_t oid = objPtr->id;
            ImGui::PushID(oid);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (oid == m_RenameID) {
                ImGui::PushItemWidth(-1);
                
                if (ImGui::InputText("##rename", &m_RenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    objPtr->name = m_RenameBuffer;
                    m_RenameID = 0;
                }
                ImGui::PopItemWidth();
            } else {
                bool isSelected = (objPtr.get() == m_Scene->GetSelectedObject());
                if (ImGui::Selectable(objPtr->name.c_str(), isSelected)) {
                    m_Scene->SetSelectedObjectByID(oid);
                }
            }
            
            std::string rename_label = "Rename##" + std::to_string(oid);
            std::string ok_label = "OK##" + std::to_string(oid);
            std::string cancel_label = "Cancel##" + std::to_string(oid);
            std::string duplicate_label = "Duplicate##" + std::to_string(oid);
            std::string delete_label = "Delete##" + std::to_string(oid);

            ImGui::TableNextColumn();
            if (oid != m_RenameID) {
                if (ImGui::Button(rename_label.c_str())) {
                    m_RenameID = oid;
                    m_RenameBuffer = objPtr->name;
                }
            } else {
                if (ImGui::Button(ok_label.c_str())) {
                    objPtr->name = m_RenameBuffer;
                    m_RenameID = 0;
                }
                ImGui::SameLine();
                if (ImGui::Button(cancel_label.c_str())) {
                    m_RenameID = 0;
                }
            }

            ImGui::TableNextColumn();
            if (ImGui::Button(duplicate_label.c_str())) {
                idToDuplicate = oid;
            }

            ImGui::TableNextColumn();
            if (ImGui::Button(delete_label.c_str())) {
                idToDelete = oid;
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    if (idToDuplicate) { m_Scene->DuplicateObject(idToDuplicate); }
    if (idToDelete)    { m_Scene->DeleteObjectByID(idToDelete); }

    ImGui::End();
}

void UI::DrawPropertiesPanel() {
    bool open = true;
    ImGui::Begin("Properties", &open);

    ISceneObject* selected = m_Scene->GetSelectedObject();
    if (!open || !selected) {
        if (!open) m_Scene->SetSelectedObjectByID(0);
        ImGui::Text("No object selected.");
        ImGui::End();
        return;
    }

    // --- Object Name and ID ---
    char nameBuf[256];
    std::strncpy(nameBuf, selected->name.c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf)-1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
        selected->name = nameBuf;
    }
    ImGui::Text("ID: %u", selected->id);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        // --- Transform Properties (handled first as they are fundamental) ---
        glm::vec3 position, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(selected->transform, scale, rotation, position, skew, perspective);
        glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

        bool transformChanged = false;
        transformChanged |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
        transformChanged |= ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 1.0f);
        transformChanged |= ImGui::DragFloat3("Scale",    glm::value_ptr(scale), 0.1f);

        if (transformChanged) {
            glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
            t *= glm::mat4_cast(glm::quat(glm::radians(euler)));
            t = glm::scale(t, scale);
            selected->transform = t;
        }

        // --- Other Dynamic Properties (seamlessly drawn after transform) ---
        const auto& props = selected->GetProperties();
        if (!props.empty()) {
            ImGui::Separator(); 
            bool propChanged = false;
            for (const auto& p : props) {
                switch (p.type) {
                    case PropertyType::Float:
                        propChanged |= ImGui::DragFloat(p.name.c_str(), static_cast<float*>(p.value_ptr), 0.05f);
                        break;
                    case PropertyType::Vec3:
                        propChanged |= ImGui::DragFloat3(p.name.c_str(), glm::value_ptr(*static_cast<glm::vec3*>(p.value_ptr)), 0.05f);
                        break;
                    case PropertyType::Color_Vec4:
                        propChanged |= ImGui::ColorEdit4(p.name.c_str(), glm::value_ptr(*static_cast<glm::vec4*>(p.value_ptr)));
                        break;
                }
            }
            if (propChanged) {
                selected->RebuildMesh();
            }
        }
    }
    
    ImGui::End();
}