#include "Renderer/OpenGLRenderer.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Shader.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

OpenGLRenderer::OpenGLRenderer() {}
OpenGLRenderer::~OpenGLRenderer() {}

bool OpenGLRenderer::Initialize(void* windowHandle) {
    m_Window = static_cast<GLFWwindow*>(windowHandle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return false; }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // FIX: Load the correct PICKING shaders for the picking system.
    m_PickingShader = std::make_unique<Shader>("shaders/picking.vert", "shaders/picking.frag");

    // FIX: Load the correct HIGHLIGHT shaders for the highlighting system.
    m_HighlightShader = std::make_unique<Shader>("shaders/highlight.vert", "shaders/highlight.frag");
    
    createPickingFramebuffer();
    
    return true;
}

void OpenGLRenderer::createPickingFramebuffer() {
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);

    glGenFramebuffers(1, &m_PickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);

    // This texture is configured to store single integer IDs.
    glGenTextures(1, &m_PickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_PickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PickingTexture, 0);

    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t OpenGLRenderer::ProcessPicking(int x, int y, const Scene& scene, const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
    
    // Clear with 0, as it represents "no object"
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();

    for (const auto& object : scene.GetSceneObjects()) {
        object->DrawForPicking(*m_PickingShader, view, projection);
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    uint32_t objectID = 0;
    int height;
    glfwGetWindowSize(m_Window, nullptr, &height);
    
    // This function reads the integer ID from the framebuffer.
    glReadPixels(x, height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &objectID);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return objectID;
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

void OpenGLRenderer::RenderHighlight(const ISceneObject &object, const Camera &camera)
{
    // Bind the special shader used for highlighting
    m_HighlightShader->Bind();

    // Set its uniforms
    m_HighlightShader->SetUniformMat4f("u_Model", object.transform); // Use the object's transform
    m_HighlightShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_HighlightShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    
    // Set a bright, noticeable color for the highlight
    m_HighlightShader->SetUniform4f("u_Color", 1.0f, 1.0f, 0.0f, 1.0f); // Bright Yellow

    // Now, tell the object to draw its mesh. It will be drawn using the
    // currently bound highlight shader.
    object.DrawHighlight(camera.GetViewMatrix(), camera.GetProjectionMatrix());

    // Unbind the shader to be safe
    m_HighlightShader->Unbind();
}

void OpenGLRenderer::Shutdown() {
    // FIX: Added a shutdown message as requested.
    std::cout << "OpenGLRenderer shutdown." << std::endl;
}