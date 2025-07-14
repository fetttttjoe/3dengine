#include "Core/Application.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <stdexcept>
#include <string>

#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/MathHelpers.h"
#include "Core/Raycaster.h"
#include "Core/ResourceManager.h"
#include "Core/SettingsManager.h"
#include "Core/UI/AppUI.h"
#include "Core/UI/HierarchyView.h"
#include "Core/UI/InspectorView.h"
#include "Core/UI/MenuBar.h"
#include "Core/UI/SettingsWindow.h"
#include "Core/UI/ToolsPane.h"
#include "Core/UI/ViewportPane.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Grid.h"
#include "Scene/Objects/CustomMesh.h"
#include "Scene/Objects/Icosphere.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/Sphere.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"
#include "Sculpting/ISculptTool.h"
#include "Sculpting/MeshEditor.h"
#include "Sculpting/SculptableMesh.h"
#include "Sculpting/SubObjectSelection.h"
#include "Sculpting/Tools/GrabTool.h"
#include "Sculpting/Tools/PushPullTool.h"
#include "Sculpting/Tools/SmoothTool.h"
#include "imgui.h"
#include "implot.h"

Application* Application::s_Instance = nullptr;

Application::Application(int initialWidth, int initialHeight)
    : m_WindowWidth(initialWidth), m_WindowHeight(initialHeight) {
  s_Instance = this;
  Initialize();
}

Application::~Application() { Cleanup(); }

Application& Application::Get() { return *s_Instance; }

void Application::Initialize() {
  Log::Debug("Application::Initialize - Starting initialization.");

  if (!SettingsManager::Load("settings.json")) {
    Log::Debug("No settings.json found, using default values.");
  }

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight,
                              "Intuitive Modeler", nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
  glfwSetScrollCallback(m_Window, scroll_callback);
  glfwSetCursorPosCallback(m_Window, cursor_position_callback);

  ResourceManager::Initialize();
  m_Renderer = std::make_unique<OpenGLRenderer>();
  if (!m_Renderer->Initialize(m_Window)) {
    throw std::runtime_error("Failed to initialize Renderer");
  }

  m_ObjectFactory = std::make_unique<SceneObjectFactory>();
  RegisterObjectTypes();
  m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
  m_Camera = std::make_unique<Camera>(m_Window);
  m_TransformGizmo = std::make_unique<TransformGizmo>();

  m_PushPullTool = std::make_unique<PushPullTool>();
  m_SmoothTool = std::make_unique<SmoothTool>();
  m_GrabTool = std::make_unique<GrabTool>();
  m_Selection = std::make_unique<SubObjectSelection>();
  m_MeshEditor = std::make_unique<MeshEditor>();

  m_UI = std::make_unique<AppUI>(this);
  m_UI->Initialize(m_Window);
  ImPlot::CreateContext();
  m_UI->RegisterView<MenuBar>(this);
  m_UI->RegisterView<ViewportPane>(this);
  m_UI->RegisterView<ToolsPane>(this);
  m_UI->RegisterView<HierarchyView>(this);
  m_UI->RegisterView<InspectorView>(this);
  m_UI->RegisterView<SettingsWindow>(this);

  m_Scene->AddObject(m_ObjectFactory->Create("Grid"));
  m_Scene->AddObject(m_ObjectFactory->Create("Icosphere"));
  Log::Debug("Application::Initialize - Initialization complete.");
}

void Application::RegisterObjectTypes() {
  m_ObjectFactory->Register(std::string(ObjectTypes::Triangle),
                            []() { return std::make_unique<Triangle>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Pyramid),
                            []() { return std::make_unique<Pyramid>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Sphere),
                            []() { return std::make_unique<Sphere>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Icosphere),
                            []() { return std::make_unique<Icosphere>(); });

  m_ObjectFactory->Register(std::string(ObjectTypes::Grid),
                            []() { return std::make_unique<Grid>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::CustomMesh),
                            []() { return std::make_unique<CustomMesh>(); });
}
void Application::Run() {
  while (!glfwWindowShouldClose(m_Window)) {
    glfwPollEvents();
    float now = static_cast<float>(glfwGetTime());
    m_DeltaTime = now - m_LastFrame;
    m_LastFrame = now;

    ProcessPendingActions();
    m_Scene->ProcessDeferredDeletions();

    if (m_Scene->GetSelectedObject() == nullptr &&
        m_TransformGizmo->GetTarget() != nullptr) {
      m_TransformGizmo->SetTarget(nullptr);
    }

    m_Renderer->SyncSceneObjects(*m_Scene);
    m_UI->BeginFrame();
    m_UI->Draw();

    auto* vp = m_UI->GetView<ViewportPane>();
    if (vp) {
      glm::vec2 currentSize = vp->GetSize();
      if (currentSize.x > 0 && currentSize.y > 0 &&
          currentSize != m_LastViewportSize) {
        m_Renderer->OnWindowResize((int)currentSize.x, (int)currentSize.y);
        m_Camera->SetAspectRatio(currentSize.x / currentSize.y);
        m_LastViewportSize = currentSize;
        RequestSceneRender();
      }

      // CORRECT: This is the robust way to handle viewport input.
      // We only pass input to the camera if the viewport is hovered.
      if (vp->IsHovered()) {
        m_Camera->HandleInput(m_DeltaTime,
                              [this]() { this->RequestSceneRender(); });
      }
    }

    processGlobalKeyboardShortcuts();
    processMouseActions();

    if (m_SceneRenderRequested) {
      m_Renderer->BeginSceneFrame();

      for (const auto& object : m_Scene->GetSceneObjects()) {
        if (object) {
          object->Draw(*m_Renderer, m_Camera->GetViewMatrix(),
                       m_Camera->GetProjectionMatrix());
        }
      }

      if (auto* sel = m_Scene->GetSelectedObject()) {
        if (m_EditorMode == EditorMode::TRANSFORM) {
          sel->DrawHighlight(m_Camera->GetViewMatrix(),
                             m_Camera->GetProjectionMatrix());
          m_TransformGizmo->Draw(*m_Renderer, *m_Camera);
        }

        if (m_EditorMode == EditorMode::SUB_OBJECT) {
          if (auto* editableMesh = sel->GetEditableMesh()) {
            m_Renderer->RenderVertexHighlights(
                *editableMesh, m_Selection->GetSelectedVertices(),
                sel->GetTransform(), *m_Camera);
            m_Renderer->RenderSelectedFaces(*editableMesh,
                                            m_Selection->GetSelectedFaces(),
                                            sel->GetTransform(), *m_Camera);
          }
        }
      }

      m_Renderer->EndSceneFrame();
      m_SceneRenderRequested = false;
    }

    m_Renderer->BeginFrame();
    if (m_ShowMetricsWindow) {
      ImGui::ShowMetricsWindow(&m_ShowMetricsWindow);
    }
    m_UI->EndFrame();
    m_Renderer->EndFrame();
  }
}

void Application::OnSceneLoaded() {
  m_Scene->Load("scene.json");
  SelectObject(0);
  m_TransformGizmo->SetTarget(nullptr);
  RequestSceneRender();
}

void Application::ImportModel(const std::string& filepath) {
  auto [vertices, indices] = ResourceManager::LoadMesh(filepath);
  if (!vertices.empty() || !indices.empty()) {
    auto newObject = std::make_unique<CustomMesh>(vertices, indices);
    float importScale = SettingsManager::Get().objImportScale;
    newObject->SetScale(glm::vec3(importScale));
    m_Scene->AddObject(std::move(newObject));
  }
}

void Application::SelectObject(uint32_t id) {
  ISceneObject* last = m_Scene->GetSelectedObject();
  m_Scene->SetSelectedObjectByID(id);
  ISceneObject* cur = m_Scene->GetSelectedObject();

  m_Selection->Clear();

  if (last != cur) {
    if (cur) {
      m_TransformGizmo->SetTarget(cur);
    } else {
      m_TransformGizmo->SetTarget(nullptr);
    }
    RequestSceneRender();
  }
  if (!cur) {
    SetEditorMode(EditorMode::TRANSFORM);
  }
}

void Application::SetEditorMode(EditorMode newMode,
                                SculptMode::Mode newSculptMode,
                                SubObjectMode newSubObjectMode) {
  Log::Debug("Application::SetEditorMode called. newMode: ", static_cast<int>(newMode),
             ", newSculptMode: ", static_cast<int>(newSculptMode),
             ", newSubObjectMode: ", static_cast<int>(newSubObjectMode));

  if (m_EditorMode == newMode && m_SculptMode == newSculptMode &&
      m_SubObjectMode == newSubObjectMode)
    return;

  m_EditorMode = newMode;
  m_SculptMode = newSculptMode;
  m_SubObjectMode = newSubObjectMode;

  m_Selection->Clear();

  ISceneObject* selectedObject = m_Scene->GetSelectedObject();

  if (m_EditorMode == EditorMode::TRANSFORM) {
    m_TransformGizmo->SetTarget(selectedObject);
  } else {
    m_TransformGizmo->SetTarget(nullptr);

    if (!selectedObject || !selectedObject->GetEditableMesh()) {
      Log::Debug(
          "Cannot enter Sculpt or Sub-Object mode: No editable mesh on "
          "selected object. Switching to Transform mode.");
      m_EditorMode = EditorMode::TRANSFORM;
      m_TransformGizmo->SetTarget(selectedObject);
    }
  }
  RequestSceneRender();
}

void Application::Cleanup() {
  Log::Debug("Application::Cleanup - Shutting down...");
  ImPlot::DestroyContext();
  m_UI->Shutdown();
  m_Renderer->Shutdown();
  ResourceManager::Shutdown();
  if (m_Window) glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void Application::Exit() { glfwSetWindowShouldClose(m_Window, true); }

void Application::RequestObjectCreation(const std::string& typeName) {
  m_RequestedCreationTypeNames.push_back(typeName);
}

void Application::RequestObjectDuplication(uint32_t objectID) {
  m_RequestedDuplicateID = objectID;
}

void Application::RequestObjectDeletion(uint32_t objectID) {
  m_RequestedDeletionIDs.push_back(objectID);
}

void Application::RequestExtrude(float distance) {
  m_ExtrudeRequested = true;
  m_ExtrudeDistance = distance;
}

void Application::RequestWeld() { m_WeldRequested = true; }

void Application::RequestMoveSelection(float distance) {
  m_MoveSelectionRequested = true;
  m_MoveSelectionDistance = distance;
}

void Application::ProcessPendingActions() {
  if (!m_RequestedCreationTypeNames.empty()) {
    for (const auto& typeName : m_RequestedCreationTypeNames) {
      if (auto obj = m_ObjectFactory->Create(typeName)) {
        m_Scene->AddObject(std::move(obj));
      }
    }
    m_RequestedCreationTypeNames.clear();
  }

  if (m_RequestedDuplicateID != 0) {
    m_Scene->DuplicateObject(m_RequestedDuplicateID);
    m_RequestedDuplicateID = 0;
  }

  if (!m_RequestedDeletionIDs.empty()) {
    for (uint32_t id : m_RequestedDeletionIDs) {
      m_Scene->QueueForDeletion(id);
    }
    m_RequestedDeletionIDs.clear();
  }

  if (m_ExtrudeRequested) {
    if (auto* sel = m_Scene->GetSelectedObject()) {
      if (auto* mesh = sel->GetEditableMesh()) {
        m_MeshEditor->Extrude(*mesh, *m_Selection, m_ExtrudeDistance);
        sel->SetMeshDirty(true);
      }
    }
    m_ExtrudeRequested = false;
  }

  if (m_WeldRequested) {
    if (auto* sel = m_Scene->GetSelectedObject()) {
      if (auto* mesh = sel->GetEditableMesh()) {
        m_MeshEditor->Weld(*mesh, *m_Selection);
        sel->SetMeshDirty(true);
      }
    }
    m_WeldRequested = false;
  }
  if (m_MoveSelectionRequested) {
    if (auto* sel = m_Scene->GetSelectedObject()) {
      if (auto* mesh = sel->GetEditableMesh()) {
        m_MeshEditor->MoveAlongNormal(*mesh, *m_Selection,
                                      m_MoveSelectionDistance);
        sel->SetMeshDirty(true);
      }
    }
    m_MoveSelectionRequested = false;
  }
}

void Application::processGlobalKeyboardShortcuts() {
  if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) Exit();

  ImGuiIO& io = ImGui::GetIO();
  if (io.WantTextInput) return;

  static bool delPressed = false;
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_PRESS && !delPressed) {
    if (auto* selected = m_Scene->GetSelectedObject()) {
      RequestObjectDeletion(selected->id);
    }
    delPressed = true;
  }
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_RELEASE) {
    delPressed = false;
  }
}

void Application::processMouseActions() {
  auto* vp = m_UI->GetView<ViewportPane>();
  if (!vp || !vp->IsHovered()) {
    if (m_IsDraggingGizmo) {
      m_IsDraggingGizmo = false;
      m_TransformGizmo->SetActiveHandle(0);
    }
    if (m_IsSculpting) {
      m_IsSculpting = false;
    }
    return;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    const auto& vb = vp->GetBounds();
    ImVec2 absMousePosImGui = ImGui::GetMousePos();
    glm::vec2 mouseScreenPosRelToViewport = MathHelpers::ToGlm(
        {absMousePosImGui.x - vb[0].x, absMousePosImGui.y - vb[0].y});
    int mx = static_cast<int>(mouseScreenPosRelToViewport.x);
    int my = static_cast<int>(mouseScreenPosRelToViewport.y);
    bool isShiftPressed = ImGui::GetIO().KeyShift;

    if (m_EditorMode == EditorMode::TRANSFORM) {
      uint32_t gizmoID =
          m_Renderer->ProcessGizmoPicking(mx, my, *m_TransformGizmo, *m_Camera);
      if (TransformGizmo::IsGizmoID(gizmoID)) {
        m_IsDraggingGizmo = true;
        m_TransformGizmo->SetActiveHandle(gizmoID);
      } else {
        uint32_t objectID =
            m_Renderer->ProcessPicking(mx, my, *m_Scene, *m_Camera);
        SelectObject(objectID);
      }
    } else if (m_EditorMode == EditorMode::SUB_OBJECT) {
      auto* sel = m_Scene->GetSelectedObject();
      if (sel && sel->GetEditableMesh()) {
        glm::vec3 ray_origin = m_Camera->GetPosition();
        glm::vec3 ray_direction = m_Camera->ScreenToWorldRay(
            mouseScreenPosRelToViewport, (int)vp->GetSize().x,
            (int)vp->GetSize().y);
        m_Selection->OnMouseDown(
            *sel->GetEditableMesh(), ray_origin, ray_direction,
            sel->GetTransform(), mouseScreenPosRelToViewport,
            m_Camera->GetViewMatrix(), m_Camera->GetProjectionMatrix(),
            (int)vp->GetSize().x, (int)vp->GetSize().y, isShiftPressed,
            m_SubObjectMode);
        RequestSceneRender();
      }
    } else if (m_EditorMode == EditorMode::SCULPT) {
      m_IsSculpting = true;
      processSculpting();
    }
  }

  if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    if (m_IsSculpting) {
      processSculpting();
    }
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    m_IsDraggingGizmo = false;
    m_TransformGizmo->SetActiveHandle(0);
    m_IsSculpting = false;

    if (m_EditorMode == EditorMode::SUB_OBJECT) {
      auto* sel = m_Scene->GetSelectedObject();
      if (sel && sel->GetEditableMesh()) {
        m_Selection->OnMouseRelease(*sel->GetEditableMesh());
        RequestSceneRender();
      }
    }
  }
}

void Application::processSculpting() {
  auto* selectedObject = m_Scene->GetSelectedObject();
  if (!selectedObject || !selectedObject->GetEditableMesh()) {
    if (m_EditorMode == EditorMode::SCULPT)
      SetEditorMode(EditorMode::TRANSFORM);
    return;
  }

  auto* editableMesh = selectedObject->GetEditableMesh();
  auto* inspector = m_UI->GetView<InspectorView>();
  if (!inspector) return;

  auto* vp = m_UI->GetView<ViewportPane>();
  if (!vp || !vp->IsHovered()) {
    return;
  }

  if (m_IsSculpting) {
    const auto vpSize = vp->GetSize();
    ImVec2 absMousePosImGui = ImGui::GetMousePos();
    const auto& vb = vp->GetBounds();
    glm::vec2 mouseScreenPosRelToViewport = MathHelpers::ToGlm(
        {absMousePosImGui.x - vb[0].x, absMousePosImGui.y - vb[0].y});

    glm::vec3 ray_origin = m_Camera->GetPosition();
    glm::vec3 ray_direction = m_Camera->ScreenToWorldRay(
        mouseScreenPosRelToViewport, (int)vpSize.x, (int)vpSize.y);

    Raycaster::RaycastResult result;
    if (Raycaster::IntersectMesh(ray_origin, ray_direction, *editableMesh,
                                 selectedObject->GetTransform(), result)) {
      SculptMode::Mode currentSculptMode = inspector->GetBrushSettings().mode;
      ISculptTool* tool = nullptr;
      switch (currentSculptMode) {
        case SculptMode::Push:
        case SculptMode::Pull:
          tool = m_PushPullTool.get();
          break;
        case SculptMode::Smooth:
          tool = m_SmoothTool.get();
          break;
        case SculptMode::Grab:
          tool = m_GrabTool.get();
          break;
      }
      if (tool) {
        selectedObject->isPristine = false;
        tool->Apply(*editableMesh, result.hitPoint, ray_direction,
                    MathHelpers::ToGlm(ImGui::GetIO().MouseDelta),
                    inspector->GetBrushSettings(), m_Camera->GetViewMatrix(),
                    m_Camera->GetProjectionMatrix(), (int)vpSize.x,
                    (int)vpSize.y);
        editableMesh->RecalculateNormals();
        selectedObject->SetMeshDirty(true);
        RequestSceneRender();
      }
    }
  }
}

void Application::cursor_position_callback(GLFWwindow* window, double xpos,
                                           double ypos) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (!app) return;

  if (glfwGetMouseButton(app->m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    if (app->m_IsDraggingGizmo) {
      app->m_TransformGizmo->Update(
          *app->m_Camera, MathHelpers::ToGlm(ImGui::GetIO().MouseDelta), true,
          app->m_LastViewportSize.x, app->m_LastViewportSize.y);
      app->RequestSceneRender();
    } else if (app->m_EditorMode == EditorMode::SUB_OBJECT) {
      auto* sel = app->m_Scene->GetSelectedObject();
      if (sel && sel->GetEditableMesh() && app->m_Selection->IsDragging()) {
        app->m_Selection->OnMouseDrag(
            MathHelpers::ToGlm(ImGui::GetIO().MouseDelta));
        app->m_Selection->ApplyDrag(
            *sel->GetEditableMesh(), app->m_Camera->GetViewMatrix(),
            app->m_Camera->GetProjectionMatrix(),
            (int)app->m_LastViewportSize.x, (int)app->m_LastViewportSize.y);
        sel->SetMeshDirty(true);
        app->RequestSceneRender();
      }
    }
  }
}

void Application::scroll_callback(GLFWwindow* window, double xoffset,
                                  double yoffset) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (app && app->m_Camera) {
    if (auto* vp = app->m_UI->GetView<ViewportPane>(); vp && vp->IsHovered()) {
      app->m_Camera->ProcessMouseScroll(float(yoffset));
    }
  }
}

void Application::framebuffer_size_callback(GLFWwindow* window, int w, int h) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (app) {
    app->m_WindowWidth = w;
    app->m_WindowHeight = h;
    app->RequestSceneRender();
  }
}

void Application::error_callback(int error, const char* desc) {
  Log::Debug("GLFW Error [", error, "]: ", desc);
}