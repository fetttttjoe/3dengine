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
#include "Sculpting/SculptableMesh.h"
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
  Log::Debug("Application::Initialize - Starting initialization...");

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

    if (auto* vp = m_UI->GetView<ViewportPane>()) {
      glm::vec2 currentSize = vp->GetSize();
      if (currentSize.x > 0 && currentSize.y > 0 &&
          currentSize != m_LastViewportSize) {
        m_Renderer->OnWindowResize((int)currentSize.x, (int)currentSize.y);
        m_Camera->SetAspectRatio(currentSize.x / currentSize.y);
        m_LastViewportSize = currentSize;
        RequestSceneRender();
      }
    }

    if (auto* vp = m_UI->GetView<ViewportPane>(); vp && vp->IsHovered()) {
      m_Camera->HandleInput(m_DeltaTime,
                            [this]() { this->RequestSceneRender(); });
    }

    processKeyboardInput();
    processMouseInput();
    if (m_IsSculpting) {
      processSculpting();
    }

    if (m_SceneRenderRequested) {
      m_Renderer->BeginSceneFrame();
      m_Renderer->RenderScene(*m_Scene, *m_Camera);

      if (auto* sel = m_Scene->GetSelectedObject()) {
        // Draw highlight in either Transform or Sculpt mode
        if (m_EditorMode == EditorMode::TRANSFORM ||
            m_EditorMode == EditorMode::SCULPT) {
          m_Renderer->RenderHighlight(*sel, *m_Camera);
        }

        // Only draw the transform gizmo in transform mode
        if (m_EditorMode == EditorMode::TRANSFORM) {
          m_TransformGizmo->Draw(*m_Camera);
        }
      }

      if (GetShowAnchors()) {
        m_Renderer->RenderAnchors(*m_Scene, *m_Camera);
      }
      m_Renderer->EndSceneFrame();
      m_SceneRenderRequested = false;
    }

    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_ShowMetricsWindow) {
      ImGui::ShowMetricsWindow(&m_ShowMetricsWindow);
    }

    m_UI->EndFrame();
    glfwSwapBuffers(m_Window);
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

  if (last != cur) {
    if (cur) {
      m_TransformGizmo->SetTarget(cur);
    } else {
      m_TransformGizmo->SetTarget(nullptr);
    }
    RequestSceneRender();
  }
  if (!cur) {
    SetEditorMode(EditorMode::TRANSFORM, SculptMode::Pull);
  }
}

void Application::SetEditorMode(EditorMode newMode,
                                SculptMode::Mode newSculptMode) {
  m_SculptMode = newSculptMode;
  if (m_EditorMode == newMode && m_SculptMode == newSculptMode) return;

  m_EditorMode = newMode;
  if (newMode == EditorMode::SCULPT && !m_Scene->GetSelectedObject()) {
    return;
  }

  if (m_EditorMode == EditorMode::SCULPT) {
    m_TransformGizmo->SetTarget(nullptr);
  } else {
    m_TransformGizmo->SetTarget(m_Scene->GetSelectedObject());
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
}

void Application::processKeyboardInput() {
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

void Application::processMouseInput() {
  auto* vp = m_UI->GetView<ViewportPane>();
  if (!vp || !vp->IsHovered()) {
    m_IsDraggingObject = m_IsDraggingGizmo = m_IsSculpting = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
    return;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    const auto& vb = vp->GetBounds();
    ImVec2 abs = ImGui::GetMousePos();
    ImVec2 rel{abs.x - vb[0].x, abs.y - vb[0].y};
    int mx = int(rel.x), my = int(rel.y);

    if (m_EditorMode == EditorMode::TRANSFORM) {
      uint32_t gzID =
          m_Renderer->ProcessGizmoPicking(mx, my, *m_TransformGizmo, *m_Camera);
      if (TransformGizmo::IsGizmoID(gzID)) {
        m_IsDraggingGizmo = true;
        m_DraggedObject = nullptr;
        m_TransformGizmo->SetActiveHandle(gzID);
        return;
      }

      uint32_t objID = m_Renderer->ProcessPicking(mx, my, *m_Scene, *m_Camera);
      SelectObject(objID);

      if (auto* sel = m_Scene->GetSelectedObject(); sel && sel->isSelectable) {
        m_IsDraggingObject = true;
        m_DraggedObject = sel;
        glm::mat4 viewProj =
            m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
        glm::vec4 clipPos = viewProj * glm::vec4(sel->GetPosition(), 1.0f);
        m_DragNDCDepth = (clipPos.w != 0.0f) ? (clipPos.z / clipPos.w) : 0.0f;
      } else {
        m_IsDraggingObject = false;
        m_DraggedObject = nullptr;
      }
    } else if (m_EditorMode == EditorMode::SCULPT) {
      if (m_Scene->GetSelectedObject()) {
        m_IsSculpting = true;
      }
    }
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    m_IsDraggingObject = m_IsDraggingGizmo = m_IsSculpting = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
  }
}

void Application::processSculpting() {
  auto* selectedObject = m_Scene->GetSelectedObject();
  if (!selectedObject || !m_UI) return;

  auto* inspector = m_UI->GetView<InspectorView>();
  if (!inspector) return;

  auto* sculptableMesh = selectedObject->GetSculptableMesh();
  if (!sculptableMesh) return;

  auto* vp = m_UI->GetView<ViewportPane>();
  if (!vp) return;

  const auto vpSize = vp->GetSize();
  ImVec2 relMousePos = {ImGui::GetMousePos().x - vp->GetBounds()[0].x,
                        ImGui::GetMousePos().y - vp->GetBounds()[0].y};

  glm::vec3 ray_origin = m_Camera->GetPosition();
  glm::vec3 ray_direction = m_Camera->ScreenToWorldRay(
      MathHelpers::ToGlm(relMousePos), (int)vpSize.x, (int)vpSize.y);

  Raycaster::RaycastResult result;
  if (Raycaster::IntersectMesh(ray_origin, ray_direction, *sculptableMesh,
                               selectedObject->GetTransform(), result)) {
    ISculptTool* tool = nullptr;
    switch (inspector->GetBrushSettings().mode) {
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

      glm::mat4 model = selectedObject->GetTransform();
      glm::vec3 hitPointWorld =
          glm::vec3(model * glm::vec4(result.hitPoint, 1.0f));

      tool->Apply(*sculptableMesh, hitPointWorld, ray_direction,
                  MathHelpers::ToGlm(ImGui::GetIO().MouseDelta),
                  inspector->GetBrushSettings(), m_Camera->GetViewMatrix(),
                  m_Camera->GetProjectionMatrix(), (int)vpSize.x,
                  (int)vpSize.y);

      sculptableMesh->RecalculateNormals();
      selectedObject->SetMeshDirty(true);
      RequestSceneRender();
    }
  }
}

void Application::cursor_position_callback(GLFWwindow* window, double xpos,
                                           double ypos) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (!app) return;
  glm::vec2 cur{float(xpos), float(ypos)};
  glm::vec2 delta = cur - app->m_LastMousePos;
  app->m_LastMousePos = cur;
  ImGuiIO& io = ImGui::GetIO();
  if (auto* vp = app->m_UI->GetView<ViewportPane>();
      io.WantCaptureMouse && (!vp || !vp->IsHovered()))
    return;

  if (app->m_IsDraggingGizmo) {
    if (auto* vp = app->m_UI->GetView<ViewportPane>()) {
      app->m_TransformGizmo->Update(*app->m_Camera, delta, true,
                                    (int)vp->GetSize().x, (int)vp->GetSize().y);
      app->RequestSceneRender();
    }
  } else if (app->m_IsDraggingObject && app->m_DraggedObject) {
    if (auto* vp = app->m_UI->GetView<ViewportPane>()) {
      const auto& vb = vp->GetBounds();
      ImVec2 abs = ImGui::GetMousePos();
      glm::vec2 rel = MathHelpers::ToGlm({abs.x - vb[0].x, abs.y - vb[0].y});
      glm::mat4 invViewProj =
          glm::inverse(app->m_Camera->GetProjectionMatrix() *
                       app->m_Camera->GetViewMatrix());
      glm::vec3 world = Camera::ScreenToWorldPoint(
          rel, app->m_DragNDCDepth, invViewProj, (int)vp->GetSize().x,
          (int)vp->GetSize().y);
      app->m_DraggedObject->SetPosition(world);
      app->RequestSceneRender();
    }
  }
}

void Application::scroll_callback(GLFWwindow* window, double xoffset,
                                  double yoffset) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  ImGuiIO& io = ImGui::GetIO();
  if (app && app->m_Camera) {
    if (auto* vp = app->m_UI->GetView<ViewportPane>();
        vp && vp->IsHovered() && !io.WantCaptureMouse) {
      app->m_Camera->ProcessMouseScroll(float(yoffset));
      app->RequestSceneRender();
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