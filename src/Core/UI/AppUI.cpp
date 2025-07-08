// AppUI.cpp
#include "Core/UI/AppUI.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "Core/SettingsManager.h"
#include "Core/UI/UIElements.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Grid.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"

//-----------------------------------------------------------------------------
// Internal helpers: Generic Splitter
//-----------------------------------------------------------------------------
void AppUI::DrawSplitter(const char* id, float& valueToAdjust,
                         bool invertDirection) {
  ImGui::PushID(id);
  ImGui::InvisibleButton("##split", ImVec2(5, -1),
                         ImGuiButtonFlags_MouseButtonLeft);
  ImGui::SetItemAllowOverlap();

  if (ImGui::IsItemHovered()) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  }

  if (ImGui::IsItemActive()) {
    float mouse_delta_x = ImGui::GetIO().MouseDelta.x;
    if (invertDirection) {
      valueToAdjust = std::clamp(valueToAdjust - mouse_delta_x, 100.0f,
                                 ImGui::GetWindowSize().x - 100.0f);
    } else {
      valueToAdjust = std::clamp(valueToAdjust + mouse_delta_x, 100.0f,
                                 ImGui::GetWindowSize().x - 100.0f);
    }

    if (strcmp(id, "split_left") == 0) {
        SettingsManager::Get().leftPaneWidth = valueToAdjust;
    } else if (strcmp(id, "split_right") == 0) {
        SettingsManager::Get().rightPaneWidth = valueToAdjust;
    }
  }

  ImVec2 min_pos = ImGui::GetItemRectMin();
  ImVec2 max_pos = ImGui::GetItemRectMax();
  float line_x = (min_pos.x + max_pos.x) * 0.5f;
  ImGui::GetWindowDrawList()->AddLine({line_x, min_pos.y}, {line_x, max_pos.y},
                                      ImGui::GetColorU32(ImGuiCol_Separator),
                                      2.0f);
  ImGui::PopID();
}

//-----------------------------------------------------------------------------
// AppUI implementation
//-----------------------------------------------------------------------------
AppUI::AppUI(Application* app)
    : m_App(app),
      m_Scene(app->GetScene()),
      m_LeftPaneWidth(SettingsManager::Get().leftPaneWidth > 0
                          ? SettingsManager::Get().leftPaneWidth
                          : 200.0f)
      ,
      m_RightPaneWidth(SettingsManager::Get().rightPaneWidth > 0
                           ? SettingsManager::Get().rightPaneWidth
                           : 300.0f)
      ,
      m_MenuBar(std::make_unique<MenuBar>(app, app->GetScene(),
                                          app->GetObjectFactory())),
      m_ToolsPane(std::make_unique<ToolsPane>()),
      m_ViewportPane(std::make_unique<ViewportPane>()),
      m_HierarchyView(std::make_unique<HierarchyView>(app, app->GetScene())),
      m_InspectorView(std::make_unique<InspectorView>(app->GetScene())),
      m_SettingsWindow(std::make_unique<SettingsWindow>()) {
  // Set up callbacks from UI components to AppUI (or directly to Application)

  // MenuBar Callbacks
  m_MenuBar->OnExitRequested = [this]() {
    if (m_ExitRequestHandler) m_ExitRequestHandler();
  };
  m_MenuBar->OnSaveScene = [this]() { m_Scene->Save("scene.json"); };
  m_MenuBar->OnLoadScene = [this]() {
    m_Scene->Load("scene.json");
    if (m_OnSceneLoadedHandler) m_OnSceneLoadedHandler();
  };
  m_MenuBar->OnShowSettings = [this]() { m_ShowSettingsWindow = true; };
  m_MenuBar->OnShowAnchorsChanged = [this](bool show) {
    m_App->SetShowAnchors(show);
  };
  m_MenuBar->OnDeleteSelectedObject = [this]() {
    if (m_Scene->GetSelectedObject()) {
      if (m_HierarchyView->OnObjectDeleted) {
        m_HierarchyView->OnObjectDeleted(m_Scene->GetSelectedObject()->id);
      }
    }
  };
  m_MenuBar->OnAddObject = [this](const std::string& typeName) {
    m_Scene->AddObject(m_MenuBar->GetFactory()->Create(typeName));
  };

  // ToolsPane Callbacks
  m_ToolsPane->OnResetCamera = [this]() {
    if (m_ResetCameraHandler) m_ResetCameraHandler();
  };

  // HierarchyView Callbacks
  m_HierarchyView->OnObjectSelected = [this](uint32_t id) {
    m_App->SelectObject(id);
  };
  m_HierarchyView->OnObjectDeleted = [this](uint32_t id) {
    if (auto* sel = m_Scene->GetSelectedObject(); sel && sel->id == id) {
      m_App->GetTransformGizmo()->SetTarget(nullptr);
    }
    m_Scene->DeleteObjectByID(id);
    m_App->SelectObject(0);
  };
  m_HierarchyView->OnObjectDuplicated = [this](uint32_t id) {
    m_Scene->DuplicateObject(id);
  };
}

AppUI::~AppUI() {
  SettingsManager::Get().leftPaneWidth = m_LeftPaneWidth;
  SettingsManager::Get().rightPaneWidth = m_RightPaneWidth;
}

void AppUI::Initialize(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef ImGuiConfigFlags_DockingEnable
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
#ifdef ImGuiConfigFlags_ViewportsEnable
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
#endif

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void AppUI::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void AppUI::BeginFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void AppUI::EndFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#ifdef ImGuiConfigFlags_ViewportsEnable
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
#endif
}

void AppUI::SetObjectFactory(SceneObjectFactory* factory) {
}

void AppUI::SetExitRequestHandler(std::function<void()> handler) {
  m_ExitRequestHandler = std::move(handler);
}

void AppUI::SetResetCameraHandler(std::function<void()> handler) {
  m_ResetCameraHandler = std::move(handler);
}

void AppUI::SetOnSceneLoadedHandler(
    std::function<void()> handler) {
  m_OnSceneLoadedHandler = std::move(handler);
}

void AppUI::ShowInspector() { m_ActiveRightTab = 1; }

glm::vec2 AppUI::GetViewportSize() const { return m_ViewportPane->GetSize(); }

const std::array<ImVec2, 2>& AppUI::GetViewportBounds() const {
  return m_ViewportPane->GetBounds();
}

bool AppUI::IsViewportFocused() const { return m_ViewportPane->IsFocused(); }
bool AppUI::IsViewportHovered() const { return m_ViewportPane->IsHovered(); }

void AppUI::Draw(uint32_t textureId) {
  ImGuiViewport* vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(vp->WorkPos);
  ImGui::SetNextWindowSize(vp->WorkSize);
  ImGui::Begin("##MainAppUI", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoSavedSettings);

  ImGui::BeginChild("ToolsPane", ImVec2(m_LeftPaneWidth, 0), true,
                    ImGuiWindowFlags_NoBringToFrontOnFocus);
  m_ToolsPane->Draw(m_ShowMetricsWindow);
  ImGui::EndChild();

  ImGui::SameLine();
  DrawSplitter("split_left", m_LeftPaneWidth, false);

  ImGui::SameLine();
  {
    float available_x = ImGui::GetContentRegionAvail().x;
    float viewport_width = available_x - m_RightPaneWidth - 5.0f;
    if (viewport_width < 100.0f) viewport_width = 100.0f;

    ImGui::BeginChild(
        "ViewportPane", ImVec2(viewport_width, 0), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    m_ViewportPane->Draw(textureId);
    ImGui::EndChild();
  }

  ImGui::SameLine();
  DrawSplitter("split_right", m_RightPaneWidth, true);

  ImGui::SameLine();
  ImGui::BeginChild("RightPane", ImVec2(m_RightPaneWidth, 0), true,
                    ImGuiWindowFlags_NoBringToFrontOnFocus);
  if (ImGui::BeginTabBar("##RightTabs")) {
    ImGuiTabItemFlags hierarchyFlags = ImGuiTabItemFlags_None;
    if (m_ActiveRightTab == 0) {
      hierarchyFlags |= ImGuiTabItemFlags_SetSelected;
      m_ActiveRightTab = -1;
    }

    if (ImGui::BeginTabItem("Hierarchy", nullptr, hierarchyFlags)) {
      m_HierarchyView->Draw();
      ImGui::EndTabItem();
    }

    ImGuiTabItemFlags inspectorFlags = ImGuiTabItemFlags_None;
    if (m_ActiveRightTab == 1) {
      inspectorFlags |= ImGuiTabItemFlags_SetSelected;
      m_ActiveRightTab = -1;
    }

    if (ImGui::BeginTabItem("Inspector", nullptr, inspectorFlags)) {
      m_InspectorView->Draw();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
  ImGui::EndChild();

  ImGui::End();

  m_MenuBar->Draw();

#ifndef NDEBUG
  if (m_ShowMetricsWindow) ImGui::ShowMetricsWindow(&m_ShowMetricsWindow);
#endif

  m_SettingsWindow->Draw(m_ShowSettingsWindow);
}