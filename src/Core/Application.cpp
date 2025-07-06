// Core/Application.cpp
#include "Core/Application.h"
#define GLFW_INCLUDE_NONE

#include <Core/SettingsManager.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Core/Camera.h"
#include "Core/ResourceManager.h"
#include "Core/UI/AppUI.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Grid.h"
#include "Scene/Objects/ObjectTypes.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Objects/Sphere.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"
#include "imgui.h"

Application::Application(int initialWidth, int initialHeight)
    : m_WindowWidth(initialWidth), m_WindowHeight(initialHeight) {
  Initialize();
}

Application::~Application() { Cleanup(); }

void Application::Initialize() {
  // GLFW + window setup
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");
  if (!SettingsManager::Load("settings.json")) {
    Log::Debug("No settings.json → using defaults");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight,
                              "Intuitive Modeler v2.0", nullptr, nullptr);
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

  // Resources & renderer
  ResourceManager::Initialize();
  m_Renderer = std::make_unique<OpenGLRenderer>();
  if (!m_Renderer->Initialize(m_Window))
    throw std::runtime_error("Failed to initialize Renderer");

  // Factory & scene
  m_ObjectFactory = std::make_unique<SceneObjectFactory>();
  RegisterObjectTypes();
  m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
  m_Camera = std::make_unique<Camera>(m_Window);
  m_TransformGizmo = std::make_unique<TransformGizmo>();

  // UI - Now using AppUI
  m_UI = std::make_unique<AppUI>(this);
  m_UI->Initialize(m_Window);
  m_UI->SetExitRequestHandler(
      [this]() { glfwSetWindowShouldClose(m_Window, true); });
  m_UI->SetResetCameraHandler([this]() { m_Camera->ResetToDefault(); });
  m_UI->SetObjectFactory(m_ObjectFactory.get());

  // NEW: Subscribe to scene loaded event from AppUI
  m_UI->SetOnSceneLoadedHandler([this]() {
    // Clear gizmo target to prevent dangling pointer to old scene object
    m_TransformGizmo->SetTarget(nullptr);
    // Clear selection
    m_Scene->SetSelectedObjectByID(0);
    // Re-select first selectable object from new scene if any
    auto const& objs = m_Scene->GetSceneObjects();
    if (!objs.empty() && objs[0]->isSelectable)
      this->SelectObject(objs[0]->id);
    else
      this->SelectObject(0);
  });

  // Initial scene objects
  m_Scene->AddObject(m_ObjectFactory->Create(std::string(ObjectTypes::Grid)));
  m_Scene->AddObject(
      m_ObjectFactory->Create(std::string(ObjectTypes::Pyramid)));
  m_Scene->AddObject(
      m_ObjectFactory->Create(std::string(ObjectTypes::Triangle)));
}

void Application::RegisterObjectTypes() {
  m_ObjectFactory->Register(std::string(ObjectTypes::Grid),
                            []() { return std::make_unique<Grid>(); });  //
  m_ObjectFactory->Register(std::string(ObjectTypes::Triangle),
                            []() { return std::make_unique<Triangle>(); });  //
  m_ObjectFactory->Register(std::string(ObjectTypes::Pyramid),
                            []() { return std::make_unique<Pyramid>(); });  //
  m_ObjectFactory->Register(std::string(ObjectTypes::Sphere),
                            []() { return std::make_unique<Sphere>(); });  //
}

void Application::Run() {
  while (!glfwWindowShouldClose(m_Window)) {
    // 1) Poll & time
    glfwPollEvents();
    float now = static_cast<float>(glfwGetTime());
    m_DeltaTime = now - m_LastFrame;
    m_LastFrame = now;

    // 2) Camera input when hovered
    if (m_UI->IsViewportHovered()) m_Camera->HandleInput(m_DeltaTime, []() {});

    // 3) Handle resize
    glm::vec2 vpSize = m_UI->GetViewportSize();
    if (vpSize.x > 0 && vpSize.y > 0) {
      m_Renderer->OnWindowResize((int)vpSize.x, (int)vpSize.y);  //
      m_Camera->SetAspectRatio(vpSize.x / vpSize.y);             //
    }

    // 4) Render scene to texture
    m_Renderer->BeginSceneFrame();                 //
    m_Renderer->RenderScene(*m_Scene, *m_Camera);  //
    if (auto* sel = m_Scene->GetSelectedObject()) {
      m_Renderer->RenderHighlight(*sel, *m_Camera);  //
      m_TransformGizmo->Draw(*m_Camera);             //
    }
    if (GetShowAnchors()) m_Renderer->RenderAnchors(*m_Scene, *m_Camera);  //
    m_Renderer->EndSceneFrame();                                           //

    // 5) ImGui frame & UI
    m_UI->BeginFrame();                           //
    m_UI->Draw(m_Renderer->GetSceneTextureId());  //
    processKeyboardInput();
    processMouseInput();
    m_UI->EndFrame();  //

    // 6) Composite to screen
    m_Renderer->BeginFrame();  //
    m_Renderer->RenderUI();    //
    m_Renderer->EndFrame();    //
  }
}

void Application::SelectObject(uint32_t id) {
  ISceneObject* last = m_Scene->GetSelectedObject();
  m_Scene->SetSelectedObjectByID(id);
  ISceneObject* cur = m_Scene->GetSelectedObject();
  if (last != cur && cur) {
    m_TransformGizmo->SetTarget(cur);
    m_UI->ShowInspector();
  }
}

void Application::Cleanup() {
  SettingsManager::Save(
      "settings.json");         // Changed from "config.json" for consistency
  m_UI->Shutdown();             //
  m_Renderer->Shutdown();       //
  ResourceManager::Shutdown();  //
  if (m_Window) glfwDestroyWindow(m_Window);  //
  glfwTerminate();                            //
}

void Application::processKeyboardInput() {
  if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_Window, true);

  ImGuiIO& io = ImGui::GetIO();
  if (io.WantTextInput) return;

  static bool delPressed = false;
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_PRESS && !delPressed) {
    if (m_Scene->GetSelectedObject()) {
      m_TransformGizmo->SetTarget(nullptr);
      m_Scene->DeleteSelectedObject();
      m_UI->ShowInspector();
    }
    delPressed = true;
  }
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_RELEASE) {
    delPressed = false;
  }
}

void Application::processMouseInput() {
  if (!m_UI->IsViewportHovered()) {
    m_IsDraggingObject = m_IsDraggingGizmo = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
    return;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    const auto& vb = m_UI->GetViewportBounds();  //
    ImVec2 abs = ImGui::GetMousePos();
    ImVec2 rel{abs.x - vb[0].x, abs.y - vb[0].y};
    int mx = int(rel.x), my = int(rel.y);

    uint32_t gzID = m_Renderer->ProcessGizmoPicking(mx, my, *m_TransformGizmo,
                                                    *m_Camera);  //
    if (TransformGizmo::IsGizmoID(gzID)) {                       //
      m_IsDraggingGizmo = true;
      m_TransformGizmo->SetActiveHandle(gzID);                          //
      Log::Debug("processMouseInput: started gizmo drag id={}", gzID);  //
      return;
    }

    uint32_t objID =
        m_Renderer->ProcessPicking(mx, my, *m_Scene, *m_Camera);  //
    Log::Debug("processMouseInput: picked objID={}", objID);      //
    SelectObject(objID);

    if (auto* sel = m_Scene->GetSelectedObject();
        sel && sel->isSelectable) {  //
      m_IsDraggingObject = true;
      m_DraggedObject = sel;
      glm::mat4 vp =
          m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();     //
      glm::vec4 cp = vp * glm::vec4(sel->GetPosition(), 1.0f);             //
      m_DragNDCDepth = (cp.w != 0.0f) ? (cp.z / cp.w) : 0.0f;              //
      Log::Debug("processMouseInput: started object drag id={} depth={}",  //
                 sel->id, m_DragNDCDepth);                                 //
    } else {
      m_IsDraggingObject = false;
      m_DraggedObject = nullptr;
    }
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    if (m_IsDraggingObject || m_IsDraggingGizmo) {
      Log::Debug("processMouseInput: ended drag");
    }
    m_IsDraggingObject = m_IsDraggingGizmo = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
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
  if (io.WantCaptureMouse && !app->m_UI->IsViewportHovered()) return;

  if (app->m_IsDraggingGizmo) {
    app->m_TransformGizmo->Update(*app->m_Camera, delta, true,
                                  int(app->m_UI->GetViewportSize().x),   //
                                  int(app->m_UI->GetViewportSize().y));  //
  } else if (app->m_IsDraggingObject && app->m_DraggedObject) {
    const auto& vb = app->m_UI->GetViewportBounds();  //
    ImVec2 abs = ImGui::GetMousePos();
    ImVec2 rel = {abs.x - vb[0].x, abs.y - vb[0].y};
    glm::vec3 world = app->m_Camera->ScreenToWorldPoint(
        {rel.x, rel.y}, app->m_DragNDCDepth,
        int(app->m_UI->GetViewportSize().x),   //
        int(app->m_UI->GetViewportSize().y));  //
    app->m_DraggedObject->SetPosition(world);
  }
}

void Application::scroll_callback(GLFWwindow* window, double xoffset,
                                  double yoffset) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  ImGuiIO& io = ImGui::GetIO();
  if (app && app->m_Camera && app->m_UI->IsViewportHovered() &&
      !io.WantCaptureMouse) {
    app->m_Camera->ProcessMouseScroll(float(yoffset));
  }
}

void Application::framebuffer_size_callback(GLFWwindow* window, int w, int h) {
  Application* app =
      static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (app) {
    app->m_WindowWidth = w;
    app->m_WindowHeight = h;
  }
  glViewport(0, 0, w, h);
}

void Application::error_callback(int error, const char* desc) {
  std::cerr << "GLFW Error [" << error << "]: " << desc << std::endl;
}