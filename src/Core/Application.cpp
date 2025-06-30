#include "Core/Application.h"
#include "Renderer/OpenGLRenderer.h"
#include "Scene/Objects/Triangle.h"
#include "Scene/Objects/Pyramid.h"
#include "Scene/Grid.h"
#include "Core/Camera.h"
#include "Core/UI.h"
#include "Scene/Scene.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

Application::Application() {}
Application::~Application() { Cleanup(); }

void Application::Initialize() {
    std::cout << "[DEBUG] Initializing Application..." << std::endl;
    
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        throw std::runtime_error("FATAL: glfwInit() failed!");
    }
    std::cout << "[DEBUG] GLFW Initialized." << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    std::cout << "[DEBUG] Creating GLFW window..." << std::endl;
    m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Intuitive Modeler v0.9.2 - Diagnostics", NULL, NULL);
    
    if (!m_Window) {
        glfwTerminate();
        throw std::runtime_error("FATAL: glfwCreateWindow() returned NULL. Check GLFW error callback output above.");
    }
    std::cout << "[DEBUG] GLFW Window created successfully." << std::endl;
    
    glfwSetWindowUserPointer(m_Window, this);
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSwapInterval(1);

    m_Renderer = std::make_unique<OpenGLRenderer>();
    if (!m_Renderer->Initialize(m_Window)) { throw std::runtime_error("Failed to initialize Renderer"); }
    
    m_Scene = std::make_unique<Scene>();
    m_Camera = std::make_unique<Camera>(m_Window);
    m_UI = std::make_unique<UI>();
    m_UI->Initialize(m_Window);

    m_Scene->AddObject(std::make_unique<Grid>());
    m_Scene->AddObject(std::make_unique<Triangle>());
    m_Scene->AddObject(std::make_unique<Pyramid>());

    auto& objects = m_Scene->GetSceneObjects();
    objects[1]->transform = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.5f, 0.0f));
    objects[2]->transform = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 0.0f));
    std::cout << "[DEBUG] Initialization complete." << std::endl;
}

void Application::Run() {
    std::cout << "[DEBUG] Entering main loop..." << std::endl;
    while (m_Window && !glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();
        processInput();
        m_Camera->HandleInput();

        m_Renderer->BeginFrame();
        m_UI->BeginFrame();

        m_Renderer->RenderScene(*m_Scene, *m_Camera);
        m_UI->DrawPropertiesPanel(*m_Scene);

        m_UI->EndFrame();
        m_Renderer->RenderUI();
        m_Renderer->EndFrame();
    }
    std::cout << "[DEBUG] Exited main loop." << std::endl;
}

void Application::Cleanup() {
    std::cout << "[DEBUG] Cleaning up..." << std::endl;
    if (m_UI) m_UI->Shutdown();
    if (m_Renderer) m_Renderer->Shutdown();
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
    std::cout << "[DEBUG] Cleanup complete." << std::endl;
}

void Application::processInput() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    static bool tabPressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        m_Scene->SelectNextObject();
        tabPressed = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabPressed = false;
    }

    ISceneObject* selected = m_Scene->GetSelectedObject();
    if (selected) {
        float moveSpeed = 0.05f;
        if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS)
            selected->transform = glm::translate(selected->transform, glm::vec3(0, 0, -moveSpeed));
        if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS)
            selected->transform = glm::translate(selected->transform, glm::vec3(0, 0, moveSpeed));
        if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS)
            selected->transform = glm::translate(selected->transform, glm::vec3(-moveSpeed, 0, 0));
        if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            selected->transform = glm::translate(selected->transform, glm::vec3(moveSpeed, 0, 0));
    }
}

void Application::error_callback(int error, const char* description) {
    std::cerr << "!!! GLFW Error [" << error << "]: " << description << " !!!" << std::endl;
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && app->m_Camera) {
        app->m_Camera->ProcessMouseScroll((float)yoffset);
    }
}