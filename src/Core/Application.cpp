#include "Core/Application.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Core/UI/UI.h"
#include "Factories/SceneObjectFactory.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Grid.h"
#include "Interfaces.h"
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

Application::~Application() {
    Cleanup();
}

void Application::Initialize() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) { throw std::runtime_error("Failed to initialize GLFW"); }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Intuitive Modeler v1.0", NULL, NULL);
    if (!m_Window) { glfwTerminate(); throw std::runtime_error("Failed to create GLFW window"); }

    glfwSetWindowUserPointer(m_Window, this);
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetCursorPosCallback(m_Window, cursor_position_callback);
    glfwSwapInterval(1);

    m_Renderer = std::make_unique<OpenGLRenderer>();
    if (!m_Renderer->Initialize(m_Window)) { throw std::runtime_error("Failed to initialize Renderer"); }

    m_ObjectFactory = std::make_unique<SceneObjectFactory>();
    RegisterObjectTypes();

    m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
    m_Camera = std::make_unique<Camera>(m_Window);
    m_UI = std::make_unique<UI>(m_Scene.get());
    m_UI->Initialize(m_Window);

    m_UI->SetObjectFactory(m_ObjectFactory.get());

    m_Scene->AddObject(m_ObjectFactory->Create("Grid"));
    m_Scene->AddObject(m_ObjectFactory->Create("Triangle"));
    m_Scene->AddObject(m_ObjectFactory->Create("Pyramid"));
}

void Application::RegisterObjectTypes() {
    m_ObjectFactory->Register("Grid", []() { return std::make_unique<Grid>(); });
    m_ObjectFactory->Register("Triangle", []() { return std::make_unique<Triangle>(); });
    m_ObjectFactory->Register("Pyramid",  []() { return std::make_unique<Pyramid>(); });
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_Window)) {
        // --- CORRECTED LOOP ORDER ---
        glfwPollEvents();

        // 1. Update timing every frame
        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        // 2. Begin UI Frame (this processes ImGui inputs)
        m_UI->BeginFrame();

        // 3. Process application logic and input
        processKeyboardInput();
        processMouseInput();
        // The camera now tells us if the static cache needs to be updated
        m_Camera->HandleInput(m_DeltaTime, [this]() { m_StaticCacheDirty = true; });

        // 4. Render the entire frame
        // Re-render the static grid texture only if the camera moved
        if (m_StaticCacheDirty) {
            m_Renderer->RenderStaticScene(*m_Scene, *m_Camera);
            m_StaticCacheDirty = false;
        }
        
        // Render the main scene
        m_Renderer->BeginFrame();
        m_Renderer->DrawCachedStaticScene();
        m_Renderer->RenderDynamicScene(*m_Scene, *m_Camera);
        ISceneObject* selectedObject = m_Scene->GetSelectedObject();
        if (selectedObject) {
            m_Renderer->RenderHighlight(*selectedObject, *m_Camera);
        }
        
        // Render the UI
        m_UI->DrawUI();
        m_UI->EndFrame();
        m_Renderer->RenderUI();
        
        // Swap buffers
        m_Renderer->EndFrame();
    }
}

void Application::Cleanup() {
    m_UI->Shutdown();
    m_Renderer->Shutdown();
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}

void Application::processKeyboardInput() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    static bool deletePressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_PRESS && !deletePressed) {
        m_Scene->DeleteSelectedObject();
        deletePressed = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_DELETE) == GLFW_RELEASE) {
        deletePressed = false;
    }
}

void Application::processMouseInput() {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            double xpos, ypos;
            glfwGetCursorPos(m_Window, &xpos, &ypos);

            uint32_t objectID = m_Renderer->ProcessPicking((int)xpos, (int)ypos, *m_Scene, *m_Camera);
            ISceneObject* clickedObject = m_Scene->GetObjectByID(objectID);
            m_Scene->SetSelectedObjectByID(objectID);

            if (clickedObject && !dynamic_cast<Grid*>(clickedObject)) {
                m_IsDraggingObject = true;
                m_DraggedObject = clickedObject;
                m_LastMousePos = glm::vec2((float)xpos, (float)ypos);
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_IsDraggingObject = false;
            m_DraggedObject = nullptr;
        }
    }
}

void Application::error_callback(int error, const char* description) {
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->m_WindowWidth = width;
        app->m_WindowHeight = height;
        if (app->m_Camera) {
            app->m_Camera->SetAspectRatio((float)width / height);
        }
        if (app->m_Renderer) {
            app->m_Renderer->OnWindowResize(width, height);
        }
        app->m_StaticCacheDirty = true;
    }
    glViewport(0, 0, width, height);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();
    if (app && app->m_Camera && !io.WantCaptureMouse) {
        app->m_Camera->ProcessMouseScroll((float)yoffset);
        app->m_StaticCacheDirty = true;
    }
}

void Application::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();
    if (app && !io.WantCaptureMouse) {
        if (app->m_IsDraggingObject && app->m_DraggedObject) {
            glm::vec3 objectWorldPos = app->m_DraggedObject->GetPosition();
            glm::vec4 screenPos = app->m_Camera->GetProjectionMatrix() * app->m_Camera->GetViewMatrix() * glm::vec4(objectWorldPos, 1.0f);
            screenPos /= screenPos.w;
            float depth = (screenPos.z + 1.0f) / 2.0f;
            glm::vec3 newScreenPoint((float)xpos, (app->m_WindowHeight - (float)ypos), depth);
            glm::mat4 invVP = glm::inverse(app->m_Camera->GetProjectionMatrix() * app->m_Camera->GetViewMatrix());
            glm::vec4 newWorldPosHomogeneous = invVP * glm::vec4(
                (newScreenPoint.x / app->m_WindowWidth) * 2.0f - 1.0f,
                (newScreenPoint.y / app->m_WindowHeight) * 2.0f - 1.0f,
                newScreenPoint.z * 2.0f - 1.0f,
                1.0f
            );
            glm::vec3 newWorldPos = glm::vec3(newWorldPosHomogeneous) / newWorldPosHomogeneous.w;
            app->m_DraggedObject->SetPosition(newWorldPos);
        } 
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            // Camera movement itself is handled in Camera::HandleInput, 
            // but we can mark the cache dirty here from the raw input.
            app->m_StaticCacheDirty = true;
        }
    }
}