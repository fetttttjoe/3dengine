// =======================================================================
// File: src/Core/Application.cpp
// =======================================================================
#include "Core/Application.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Core/UI/UI.h"
#include "Factories/SceneObjectFactory.h" //
#include "Scene/Objects/Triangle.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Grid.h"
#include "Interfaces.h" // Include Interfaces for ISceneObject
#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "imgui.h"

// For glm::decompose and glm::unProject
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr

Application::Application() {
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
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    // Register cursor position callback for drag-and-drop
    glfwSetCursorPosCallback(m_Window, cursor_position_callback);
    glfwSwapInterval(1);

    m_Renderer = std::make_unique<OpenGLRenderer>();
    if (!m_Renderer->Initialize(m_Window)) { throw std::runtime_error("Failed to initialize Renderer"); }

    m_ObjectFactory = std::make_unique<SceneObjectFactory>(); // Initialize factory first
    RegisterObjectTypes(); // Register object types with the factory

    // FIX: Pass the m_ObjectFactory.get() to the Scene constructor
    m_Scene = std::make_unique<Scene>(m_ObjectFactory.get());
    m_Camera = std::make_unique<Camera>(m_Window);
    m_UI = std::make_unique<UI>(m_Scene.get());
    m_UI->Initialize(m_Window);

    m_UI->SetObjectFactory(m_ObjectFactory.get()); // UI also needs the factory

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
        float currentFrame = (float)glfwGetTime();
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        glfwPollEvents();
        processInput();
        m_Camera->HandleInput(m_DeltaTime);

        m_Renderer->BeginFrame();
        m_UI->BeginFrame();

        m_Renderer->RenderScene(*m_Scene, *m_Camera);

        // Render highlight for the selected object if one exists
        ISceneObject* selectedObject = m_Scene->GetSelectedObject();
        if (selectedObject) {
            m_Renderer->RenderHighlight(*selectedObject, *m_Camera);
        }

        m_UI->DrawUI();

        m_UI->EndFrame();
        m_Renderer->RenderUI();
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

void Application::processInput() {
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

void Application::error_callback(int error, const char* description) {
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Retrieve the Application instance
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->m_WindowWidth = width;
        app->m_WindowHeight = height;
        // Also update the camera's aspect ratio if needed
        if (app->m_Camera) {
            app->m_Camera->SetAspectRatio((float)width / height);
        }
    }
    glViewport(0, 0, width, height);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();
    if (app && app->m_Camera && !io.WantCaptureMouse) {
        app->m_Camera->ProcessMouseScroll((float)yoffset);
    }
}

void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();

    if (app) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                if (!io.WantCaptureMouse) {
                    uint32_t objectID = app->m_Renderer->ProcessPicking((int)xpos, (int)ypos, *app->m_Scene, *app->m_Camera);
                    ISceneObject* clickedObject = app->m_Scene->GetObjectByID(objectID); // Get object by ID

                    app->m_Scene->SetSelectedObjectByID(objectID); // Set selection in scene

                    // Start drag if an object was clicked
                    if (clickedObject && !dynamic_cast<Grid*>(clickedObject)) { // Don't drag the grid
                        app->m_IsDraggingObject = true;
                        app->m_DraggedObject = clickedObject;
                        app->m_LastMousePos = glm::vec2((float)xpos, (float)ypos);
                        // Hide cursor while dragging might be desired
                        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        // If no object or grid was clicked, deselect
                        app->m_Scene->SetSelectedObjectByID(0); // Assuming 0 is an invalid ID or background
                    }
                }
            } else if (action == GLFW_RELEASE) {
                app->m_IsDraggingObject = false;
                app->m_DraggedObject = nullptr;
                // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }
}

void Application::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();

    if (app && !io.WantCaptureMouse && app->m_IsDraggingObject && app->m_DraggedObject) {
        float dx = (float)xpos - app->m_LastMousePos.x;
        float dy = (float)ypos - app->m_LastMousePos.y;
        app->m_LastMousePos = glm::vec2((float)xpos, (float)ypos);

        // --- Drag and Drop Logic ---
        // This is a simplified approach, a more robust solution would involve
        // projecting mouse coordinates onto a plane (e.g., the XZ plane) or
        // using the object's original depth.

        // Get current object position in world space
        glm::vec3 objectWorldPos = app->m_DraggedObject->GetPosition();

        // Project the object's current world position back to screen space to get its depth
        glm::vec4 screenPos = app->m_Camera->GetProjectionMatrix() * app->m_Camera->GetViewMatrix() * glm::vec4(objectWorldPos, 1.0f);
        screenPos /= screenPos.w; // Perspective division
        // screenPos.z is now in NDC space [-1, 1]. Map it to window Z [0, 1] for glReadPixels or similar.
        float depth = (screenPos.z + 1.0f) / 2.0f;

        // Use the existing depth to unproject new mouse coordinates
        // Create a screen point with the new X/Y and the object's current depth
        glm::vec3 newScreenPoint(
            (float)xpos,
            (app->m_WindowHeight - (float)ypos), // Invert Y for OpenGL's bottom-left origin
            depth
        );

        // Unproject to get new world coordinates
        glm::mat4 invVP = glm::inverse(app->m_Camera->GetProjectionMatrix() * app->m_Camera->GetViewMatrix());
        glm::vec4 newWorldPosHomogeneous = invVP * glm::vec4(
            (newScreenPoint.x / app->m_WindowWidth) * 2.0f - 1.0f,    // Normalize X to NDC [-1, 1]
            (newScreenPoint.y / app->m_WindowHeight) * 2.0f - 1.0f,   // Normalize Y to NDC [-1, 1]
            newScreenPoint.z * 2.0f - 1.0f,                             // Normalize Z to NDC [-1, 1]
            1.0f
        );

        glm::vec3 newWorldPos = glm::vec3(newWorldPosHomogeneous) / newWorldPosHomogeneous.w;

        // For simple planar dragging (e.g., along XZ plane), you might force Y to 0 or object's original Y.
        // newWorldPos.y = objectWorldPos.y; // Keep on the same Y level as the object started

        app->m_DraggedObject->SetPosition(newWorldPos);
    }
}