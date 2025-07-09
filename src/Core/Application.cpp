#include "Core/Application.h"
#define GLFW_INCLUDE_NONE

#include <Core/SettingsManager.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <stdexcept>
#include <string>

#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/ResourceManager.h"
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
#include "Scene/Objects/Icosphere.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/Sphere.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"
#include "Sculpting/SculptableMesh.h"
#include "Sculpting/Tools/PushPullTool.h"
#include "imgui.h"

Application::Application(int initialWidth, int initialHeight)
    : m_WindowWidth(initialWidth), m_WindowHeight(initialHeight) {
  Initialize();
}

Application::~Application() { Cleanup(); }

void Application::Initialize() {
  Log::Debug("Application::Initialize - Starting initialization...");

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  if (!SettingsManager::Load("settings.json")) {
    Log::Debug("No settings.json found, using default values.");
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
  Log::Debug("GLFW Window created successfully.");

  ResourceManager::Initialize();
  m_Renderer = std::make_unique<OpenGLRenderer>();
  if (!m_Renderer->Initialize(m_Window)) {
    throw std::runtime_error("Failed to initialize Renderer");
  }
  Log::Debug("OpenGLRenderer initialized.");

  m_ObjectFactory = std::make_unique<SceneObjectFactory>();
  RegisterObjectTypes();
  m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
  m_Camera = std::make_unique<Camera>(m_Window);
  m_TransformGizmo = std::make_unique<TransformGizmo>();
  Log::Debug("Scene and Factory initialized.");

  m_PushPullTool = std::make_unique<PushPullTool>();

  // --- UI Setup ---
  m_UI = std::make_unique<AppUI>(this);
  m_UI->Initialize(m_Window);
  Log::Debug("AppUI initialized.");

  // Register all UI views with the AppUI host
  m_UI->RegisterView<MenuBar>(this);
  m_UI->RegisterView<ViewportPane>(this);
  m_UI->RegisterView<ToolsPane>(this);
  m_UI->RegisterView<HierarchyView>(this);
  m_UI->RegisterView<InspectorView>(this);
  m_UI->RegisterView<SettingsWindow>(this);
  Log::Debug("All UI views registered.");

  // --- Scene Setup ---
  m_Scene->AddObject(m_ObjectFactory->Create(std::string(ObjectTypes::Grid)));
  m_Scene->AddObject(
      m_ObjectFactory->Create(std::string(ObjectTypes::Icosphere)));
  Log::Debug("Default scene created.");
  Log::Debug("Application::Initialize - Initialization complete.");
}

void Application::RegisterObjectTypes() {
  m_ObjectFactory->Register(std::string(ObjectTypes::Grid),
                            []() { return std::make_unique<Grid>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Triangle),
                            []() { return std::make_unique<Triangle>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Pyramid),
                            []() { return std::make_unique<Pyramid>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Sphere),
                            []() { return std::make_unique<Sphere>(); });
  m_ObjectFactory->Register(std::string(ObjectTypes::Icosphere),
                            []() { return std::make_unique<Icosphere>(); });
}

void Application::Run() {
  while (!glfwWindowShouldClose(m_Window)) {
    // 1. Poll events and calculate delta time
    glfwPollEvents();
    float now = static_cast<float>(glfwGetTime());
    m_DeltaTime = now - m_LastFrame;
    m_LastFrame = now;

    // 2. Begin the UI frame
    m_UI->BeginFrame();

    // 3. Draw the UI layout to determine viewport size
    m_UI->Draw();

    // 4. Update renderer and camera based on the new UI layout
    if (auto* vp = m_UI->GetView<ViewportPane>()) {
      glm::vec2 currentSize = vp->GetSize();
      if (currentSize.x > 0 && currentSize.y > 0 &&
          currentSize != m_LastViewportSize) {
        m_Renderer->OnWindowResize((int)currentSize.x, (int)currentSize.y);
        m_Camera->SetAspectRatio(currentSize.x / currentSize.y);
        m_LastViewportSize = currentSize;
      }
    }

    // 5. Process user input
    if (auto* vp = m_UI->GetView<ViewportPane>(); vp && vp->IsHovered()) {
      m_Camera->HandleInput(m_DeltaTime, []() {});
    }
    if (m_IsSculpting) {
      processSculpting();
    }
    processKeyboardInput();
    processMouseInput();

    // 6. Render the 3D scene to the off-screen framebuffer
    m_Renderer->SyncSceneObjects(*m_Scene);
    m_Renderer->BeginSceneFrame();
    m_Renderer->RenderScene(*m_Scene, *m_Camera);

    if (auto* sel = m_Scene->GetSelectedObject()) {
      if (m_EditorMode == EditorMode::TRANSFORM) {
        m_Renderer->RenderHighlight(*sel, *m_Camera);
        m_TransformGizmo->Draw(*m_Camera);
      }
    }

    if (GetShowAnchors()) {
      m_Renderer->RenderAnchors(*m_Scene, *m_Camera);
    }
    m_Renderer->EndSceneFrame();

    // 7. Clear the main window's back buffer before drawing the UI to it
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (m_ShowMetricsWindow) {
      ImGui::ShowMetricsWindow(&m_ShowMetricsWindow);
    }
    // 8. Composite the final UI (which contains the 3D scene as an image) onto
    // the main back buffer
    m_UI->EndFrame();

    // 9. Swap the main window's buffers to display the result
    glfwSwapBuffers(m_Window);
  }
}

void Application::OnSceneLoaded() {
  Log::Debug("Application::OnSceneLoaded - New scene loaded.");
  m_TransformGizmo->SetTarget(nullptr);
  m_Scene->SetSelectedObjectByID(0);
  auto const& objs = m_Scene->GetSceneObjects();
  if (!objs.empty() && objs[0]->isSelectable) {
    SelectObject(objs[0]->id);
  } else {
    SelectObject(0);
  }
}

void Application::SelectObject(uint32_t id) {
  ISceneObject* last = m_Scene->GetSelectedObject();
  m_Scene->SetSelectedObjectByID(id);
  ISceneObject* cur = m_Scene->GetSelectedObject();

  if (last != cur) {
    Log::Debug("Application::SelectObject - Selection changed to object ID: ",
               id);
    if (cur) {
      m_TransformGizmo->SetTarget(cur);
    }
  }

  if (!cur) {
    SetEditorMode(EditorMode::TRANSFORM, SculptMode::Pull);
  }
}

void Application::SetEditorMode(EditorMode newMode,
                                SculptMode::Mode newSculptMode) {
  m_SculptMode = newSculptMode;
  if (m_EditorMode == newMode) return;

  if (newMode == EditorMode::SCULPT && !m_Scene->GetSelectedObject()) {
    Log::Debug(
        "Application::SetEditorMode - Cannot enter sculpt mode: No object "
        "selected.");
    return;
  }

  m_EditorMode = newMode;
  Log::Debug("Application::SetEditorMode - Mode changed to: ",
             (newMode == EditorMode::TRANSFORM ? "TRANSFORM" : "SCULPT"));

  if (m_EditorMode == EditorMode::SCULPT) {
    m_TransformGizmo->SetTarget(nullptr);
  } else {
    m_TransformGizmo->SetTarget(m_Scene->GetSelectedObject());
  }
}

void Application::Cleanup() {
  Log::Debug("Application::Cleanup - Shutting down...");
  SettingsManager::Save("settings.json");
  m_UI->Shutdown();
  m_Renderer->Shutdown();
  ResourceManager::Shutdown();
  if (m_Window) glfwDestroyWindow(m_Window);
  glfwTerminate();
  Log::Debug("Application::Cleanup - Shutdown complete.");
}

void Application::Exit() { glfwSetWindowShouldClose(m_Window, true); }

void Application::processKeyboardInput() {
  if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) Exit();

  ImGuiIO& io = ImGui::GetIO();
  if (io.WantTextInput) return;

  static bool delPressed = false;
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_PRESS && !delPressed) {
    if (m_Scene->GetSelectedObject()) {
      Log::Debug(
          "Application::processKeyboardInput - Delete key pressed, "
          "deleting selected object.");
      m_TransformGizmo->SetTarget(nullptr);
      m_Scene->DeleteSelectedObject();
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
    if (m_EditorMode == EditorMode::TRANSFORM) {
      const auto& vb = vp->GetBounds();
      ImVec2 abs = ImGui::GetMousePos();
      ImVec2 rel{abs.x - vb[0].x, abs.y - vb[0].y};
      int mx = int(rel.x), my = int(rel.y);

      uint32_t gzID =
          m_Renderer->ProcessGizmoPicking(mx, my, *m_TransformGizmo, *m_Camera);
      if (TransformGizmo::IsGizmoID(gzID)) {
        Log::Debug("Application::processMouseInput - Started gizmo drag.");
        m_IsDraggingGizmo = true;
        m_TransformGizmo->SetActiveHandle(gzID);
        return;
      }

      uint32_t objID = m_Renderer->ProcessPicking(mx, my, *m_Scene, *m_Camera);
      SelectObject(objID);

      if (auto* sel = m_Scene->GetSelectedObject(); sel && sel->isSelectable) {
        Log::Debug("Application::processMouseInput - Started object drag.");
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
        Log::Debug("Application::processMouseInput - Started sculpting.");
        m_IsSculpting = true;
      }
    }
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    if (m_IsDraggingGizmo || m_IsDraggingObject || m_IsSculpting) {
      Log::Debug("Application::processMouseInput - Finished drag/sculpt.");
    }
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

  const auto& vb = vp->GetBounds();
  const auto vpSize = vp->GetSize();
  ImVec2 absMousePos = ImGui::GetMousePos();
  ImVec2 relMousePos = {absMousePos.x - vb[0].x, absMousePos.y - vb[0].y};

  glm::vec3 ray_origin = m_Camera->GetPosition();
  glm::vec3 ray_direction = glm::normalize(
      m_Camera->ScreenToWorldPoint({relMousePos.x, relMousePos.y}, -1.0f,
                                   (int)vpSize.x, (int)vpSize.y) -
      ray_origin);

  glm::mat4 invModelMatrix = glm::inverse(selectedObject->GetTransform());
  glm::vec3 ray_origin_local =
      glm::vec3(invModelMatrix * glm::vec4(ray_origin, 1.0f));
  glm::vec3 ray_direction_local = glm::normalize(
      glm::vec3(invModelMatrix * glm::vec4(ray_direction, 0.0f)));

  float closest_dist_sq = std::numeric_limits<float>::max();
  glm::vec3 hit_point_local;
  bool hit = false;

  const auto& vertices = sculptableMesh->GetVertices();
  const auto& indices = sculptableMesh->GetIndices();

  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    const glm::vec3& v0 = vertices[indices[i]];
    const glm::vec3& v1 = vertices[indices[i + 1]];
    const glm::vec3& v2 = vertices[indices[i + 2]];
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(ray_direction_local, edge2);
    float a = glm::dot(edge1, h);
    if (a > -1e-6 && a < 1e-6) continue;
    float f = 1.0f / a;
    glm::vec3 s = ray_origin_local - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) continue;
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray_direction_local, q);
    if (v < 0.0f || u + v > 1.0f) continue;
    float t = f * glm::dot(edge2, q);
    if (t > 1e-6) {
      if (t * t < closest_dist_sq) {
        closest_dist_sq = t * t;
        hit_point_local = ray_origin_local + ray_direction_local * t;
        hit = true;
      }
    }
  }

  if (hit) {
    m_PushPullTool->Apply(*sculptableMesh, hit_point_local,
                          inspector->GetBrushRadius(),
                          inspector->GetBrushStrength(), m_SculptMode);

    sculptableMesh->RecalculateNormals();
    selectedObject->SetMeshDirty(true);
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

  if (app->m_EditorMode == EditorMode::TRANSFORM) {
    if (app->m_IsDraggingGizmo) {
      if (auto* vp = app->m_UI->GetView<ViewportPane>()) {
        app->m_TransformGizmo->Update(*app->m_Camera, delta, true,
                                      (int)vp->GetSize().x,
                                      (int)vp->GetSize().y);
      }
    } else if (app->m_IsDraggingObject && app->m_DraggedObject) {
      if (auto* vp = app->m_UI->GetView<ViewportPane>()) {
        const auto& vb = vp->GetBounds();
        ImVec2 abs = ImGui::GetMousePos();
        ImVec2 rel = {abs.x - vb[0].x, abs.y - vb[0].y};
        glm::vec3 world = app->m_Camera->ScreenToWorldPoint(
            {rel.x, rel.y}, app->m_DragNDCDepth, (int)vp->GetSize().x,
            (int)vp->GetSize().y);
        app->m_DraggedObject->SetPosition(world);
      }
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
    }
  }
}

void Application::framebuffer_size_callback(GLFWwindow* window, int w, int h) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (app) {
    app->m_WindowWidth = w;
    app->m_WindowHeight = h;
  }
}

void Application::error_callback(int error, const char* desc) {
  Log::Debug("GLFW Error [", error, "]: ", desc);
}