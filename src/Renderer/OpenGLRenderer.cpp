#include "OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

OpenGLRenderer::OpenGLRenderer() {}
OpenGLRenderer::~OpenGLRenderer() {}

bool OpenGLRenderer::Initialize(void* windowHandle) {
    m_Window = static_cast<GLFWwindow*>(windowHandle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    return true;
}

void OpenGLRenderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::RenderScene(const Scene& scene, const Camera& camera) {
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();

    for (const auto& object : scene.GetSceneObjects()) {
        object->Draw(view, projection);
    }
}

void OpenGLRenderer::RenderUI() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLRenderer::EndFrame() {
    glfwSwapBuffers(m_Window);
}

void OpenGLRenderer::Shutdown() {
    std::cout << "Shutdown" << std::endl;
}