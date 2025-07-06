#include "Core/Application.h"
#include "Core/ResourceManager.h" 
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Core/UI/UI.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Grid.h"
#include "Interfaces.h"
#include "Scene/TransformGizmo.h"
#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

Application::Application(int initialWidth, int initialHeight)
    : m_WindowWidth(initialWidth), m_WindowHeight(initialHeight)
{
  Initialize();
}

Application::~Application()
{
  Cleanup();
}

void Application::Initialize()
{
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Intuitive Modeler v2.0", NULL, NULL);
  if (!m_Window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwSetWindowUserPointer(m_Window, this);
  glfwMakeContextCurrent(m_Window);
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
  glfwSetScrollCallback(m_Window, scroll_callback);
  glfwSetCursorPosCallback(m_Window, cursor_position_callback);
  glfwSwapInterval(1);

  ResourceManager::Initialize();

  m_Renderer = std::make_unique<OpenGLRenderer>();
  if (!m_Renderer->Initialize(m_Window))
  {
    throw std::runtime_error("Failed to initialize Renderer");
  }

  m_ObjectFactory = std::make_unique<SceneObjectFactory>();
  RegisterObjectTypes();

  m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
  m_Camera = std::make_unique<Camera>(m_Window);
  m_UI = std::make_unique<UI>(m_Scene.get());
  m_TransformGizmo = std::make_unique<TransformGizmo>(); 
  m_UI->Initialize(m_Window);

  m_UI->SetObjectFactory(m_ObjectFactory.get());

  m_Scene->AddObject(m_ObjectFactory->Create("Grid"));
  m_Scene->AddObject(m_ObjectFactory->Create("Pyramid"));
  m_Scene->AddObject(m_ObjectFactory->Create("Triangle"));
}

void Application::RegisterObjectTypes()
{
  m_ObjectFactory->Register("Grid", []()
                            { return std::make_unique<Grid>(); });
  m_ObjectFactory->Register("Triangle", []()
                            { return std::make_unique<Triangle>(); });
  m_ObjectFactory->Register("Pyramid", []()
                            { return std::make_unique<Pyramid>(); });
}

void Application::Run()
{
  while (!glfwWindowShouldClose(m_Window))
  {
    glfwPollEvents();

    float currentFrame = (float)glfwGetTime();
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    m_UI->BeginFrame();

    processKeyboardInput();
    processMouseInput();
    m_Camera->HandleInput(m_DeltaTime, [this]()
                          { m_StaticCacheDirty = true; });

    if (m_StaticCacheDirty)
    {
      m_Renderer->RenderStaticScene(*m_Scene, *m_Camera);
      m_StaticCacheDirty = false;
    }

    m_Renderer->BeginFrame();
    m_Renderer->DrawCachedStaticScene();
    m_Renderer->RenderDynamicScene(*m_Scene, *m_Camera);

    ISceneObject *selectedObject = m_Scene->GetSelectedObject();
    if (selectedObject)
    {
      m_Renderer->RenderHighlight(*selectedObject, *m_Camera);
      m_TransformGizmo->Draw(*m_Camera);
    }

    m_UI->DrawUI();
    m_UI->EndFrame();
    m_Renderer->RenderUI();

    m_Renderer->EndFrame();
  }
}

void Application::Cleanup()
{
  m_UI->Shutdown();
  m_Renderer->Shutdown();
  ResourceManager::Shutdown();
  if (m_Window)
  {
    glfwDestroyWindow(m_Window);
  }
  glfwTerminate();
}

void Application::processKeyboardInput()
{
  if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_Window, true);

  static bool deletePressed = false;
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_PRESS && !deletePressed)
  {
    m_Scene->DeleteSelectedObject();
    m_TransformGizmo->SetTarget(nullptr); // Deselect from gizmo too
    deletePressed = true;
  }
  if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_RELEASE)
  {
    deletePressed = false;
  }
}

void Application::processMouseInput()
{
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse)
  {
    m_IsDraggingObject = false;
    m_IsDraggingGizmo = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
    return;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  {
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    m_LastMousePos = glm::vec2((float)xpos, (float)ypos);

    // --- GIZMO PICKING PASS ---
    uint32_t gizmoID = m_Renderer->ProcessGizmoPicking((int)xpos, (int)ypos, *m_TransformGizmo, *m_Camera);
    if (TransformGizmo::IsGizmoID(gizmoID))
    {
      m_IsDraggingGizmo = true;
      m_TransformGizmo->SetActiveHandle(gizmoID);
      return; 
    }

    // --- OBJECT PICKING PASS ---
    uint32_t objectID = m_Renderer->ProcessPicking((int)xpos, (int)ypos, *m_Scene, *m_Camera);
    ISceneObject *lastSelected = m_Scene->GetSelectedObject();
    m_Scene->SetSelectedObjectByID(objectID);
    ISceneObject *currentSelected = m_Scene->GetSelectedObject();

    if (lastSelected != currentSelected)
    {
      m_TransformGizmo->SetTarget(currentSelected);
    }

    if (currentSelected && currentSelected->isSelectable)
    {
      m_IsDraggingObject = true;
      m_DraggedObject = currentSelected;

      // FIX: Calculate the object's depth in NDC space and store it.
      // This ensures we move the object on a plane parallel to the screen,
      // preventing it from jumping towards the camera.
      glm::mat4 viewProjection = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
      glm::vec4 clipPos = viewProjection * glm::vec4(m_DraggedObject->GetPosition(), 1.0f);
      if (clipPos.w != 0.0f) {
          m_DragNDCDepth = clipPos.z / clipPos.w;
      } else {
          m_DragNDCDepth = 0.0f; // Fallback
      }

    } else {
      m_IsDraggingObject = false;
      m_DraggedObject = nullptr;
    }
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
  {
    m_IsDraggingObject = false;
    m_IsDraggingGizmo = false;
    m_DraggedObject = nullptr;
    m_TransformGizmo->SetActiveHandle(0);
  }
}

void Application::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
  Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app) return;

  glm::vec2 currentMousePos((float)xpos, (float)ypos);
  glm::vec2 mouseDelta = currentMousePos - app->m_LastMousePos;
  app->m_LastMousePos = currentMousePos;

  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse) return;

  // --- GIZMO DRAGGING LOGIC ---
  if (app->m_IsDraggingGizmo)
  {
    app->m_TransformGizmo->Update(*app->m_Camera, mouseDelta, true, app->m_WindowWidth, app->m_WindowHeight);
  }
  // --- OBJECT DRAGGING LOGIC ---
  else if (app->m_IsDraggingObject && app->m_DraggedObject)
  {
    // FIX: Use the constant NDC depth calculated when the drag started.
    // This correctly unprojects the mouse cursor onto the plane the object was on,
    // resulting in smooth, intuitive movement.
    glm::vec3 newWorldPos = app->m_Camera->ScreenToWorldPoint(currentMousePos, app->m_DragNDCDepth, app->m_WindowWidth, app->m_WindowHeight);
    
    app->m_DraggedObject->SetPosition(newWorldPos);
  }
  else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
  {
    app->m_StaticCacheDirty = true;
  }
}

void Application::error_callback(int error, const char *description)
{
  std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

void Application::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app)
  {
    app->m_WindowWidth = width;
    app->m_WindowHeight = height;
    if (app->m_Camera)
    {
      app->m_Camera->SetAspectRatio((float)width / (height > 0 ? height : 1));
    }
    if (app->m_Renderer)
    {
      app->m_Renderer->OnWindowResize(width, height);
    }
    app->m_StaticCacheDirty = true;
  }
  glViewport(0, 0, width, height);
}

void Application::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  ImGuiIO &io = ImGui::GetIO();
  if (app && app->m_Camera && !io.WantCaptureMouse)
  {
    app->m_Camera->ProcessMouseScroll((float)yoffset);
    app->m_StaticCacheDirty = true;
  }
}
