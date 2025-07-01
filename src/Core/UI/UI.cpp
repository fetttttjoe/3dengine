// Core/UI/UI.cpp
#include "Core/UI/UI.h"
#include "Core/Log.h"
#include "Core/UI/UIElements.h"

#include "Scene/Scene.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"           // for ISceneObject
#include "Scene/Grid.h"           // to skip in outliner
#include "Scene/Objects/BaseObject.h" // for color API

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
            // TODO: actually close window
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
        });

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
        ImGui::TableSetupColumn("Dup",        ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Del",        ImGuiTableColumnFlags_WidthFixed, 50.0f);

        const auto& objects = m_Scene->GetSceneObjects();
        for (const auto& objPtr : objects) {
            if (dynamic_cast<Grid*>(objPtr.get())) continue;

            uint32_t oid = objPtr->id;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            // Inline rename vs selectable
            if (oid == m_RenameID) {
                ImGui::PushItemWidth(-1);
                char buf[256];
                std::strncpy(buf, m_RenameBuffer.c_str(), sizeof(buf));
                buf[sizeof(buf)-1] = '\0';
                if (ImGui::InputText("##rename", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    objPtr->name = buf;
                    m_RenameBuffer = buf;
                    m_RenameID = 0;
                }
                ImGui::PopItemWidth();
            } else {
                bool isSelected = (objPtr.get() == m_Scene->GetSelectedObject());
                if (ImGui::Selectable(objPtr->name.c_str(), isSelected)) {
                    m_Scene->SetSelectedObjectByID(oid);
                }
            }

            // Rename button or OK/Cancel
            ImGui::TableNextColumn();
            if (oid != m_RenameID) {
                if (UIElements::SmallButton("Edit")) {
                    m_RenameID = oid;
                    m_RenameBuffer = objPtr->name;
                }
            } else {
                if (UIElements::SmallButton("OK")) {
                    objPtr->name = m_RenameBuffer;
                    m_RenameID = 0;
                }
                ImGui::SameLine();
                if (UIElements::SmallButton("Cancel")) {
                    m_RenameID = 0;
                }
            }

            // Duplicate button
            ImGui::TableNextColumn();
            if (UIElements::SmallButton("Dup")) {
                idToDuplicate = oid;
            }

            // Delete button
            ImGui::TableNextColumn();
            if (UIElements::SmallButton("Del")) {
                idToDelete = oid;
            }
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

    // Inline rename in properties
    ImGui::PushItemWidth(-1);
    {
        char nameBuf[256];
        std::strncpy(nameBuf, selected->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf)-1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            selected->name = nameBuf;
        }
    }
    ImGui::PopItemWidth();

    ImGui::Text("ID: %u", selected->id);
    ImGui::Separator();

    // Transform
    glm::vec3 position, scale, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(selected->transform, scale, rotation, position, skew, perspective);
    glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

    bool changed = false;
    changed |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
    changed |= ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 1.0f);
    changed |= ImGui::DragFloat3("Scale",    glm::value_ptr(scale), 0.1f);

    if (changed) {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
        t *= glm::mat4_cast(glm::quat(glm::radians(euler)));
        t = glm::scale(t, scale);
        selected->transform = t;
    }

    // Color picker
    if (auto* base = dynamic_cast<BaseObject*>(selected)) {
        glm::vec4 col = base->GetColor();
        if (ImGui::ColorEdit4("Color", glm::value_ptr(col))) {
            base->SetColor(col);
        }
        ImGui::Separator();
    }

    // Shape properties
    auto props = selected->GetProperties();
    if (!props.empty()) {
        ImGui::Text("Shape Properties");
        bool propChanged = false;
        for (auto& p : props) {
            if (p.value) {
                propChanged |= ImGui::DragFloat(p.name.c_str(), p.value, 0.05f);
            }
        }
        if (propChanged) {
            selected->RebuildMesh();
        }
    }

    ImGui::End();
}

