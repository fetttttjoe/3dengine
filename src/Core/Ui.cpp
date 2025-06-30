
// =======================================================================
// File: src/UI.cpp (NEW FILE)
// =======================================================================
#include "UI.h"
#include "Scene/Scene.h"
#include "Interfaces.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>

UI::UI() {}
UI::~UI() {}

void UI::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
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

void UI::DrawPropertiesPanel(Scene& scene) {
    ImGui::Begin("Properties");

    ISceneObject* selected = scene.GetSelectedObject();
    if (selected) {
        ImGui::Text("Selected Object: %s", selected->name.c_str());
        ImGui::Separator();
        ImGui::DragFloat3("Position", glm::value_ptr(selected->transform[3]), 0.1f);
        // Add more properties here (rotation, scale, etc.)
    } else {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}
