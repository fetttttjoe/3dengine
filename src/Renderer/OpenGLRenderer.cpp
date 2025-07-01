#include "OpenGLRenderer.h"
#include "Core/Log.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Shader.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

OpenGLRenderer::OpenGLRenderer() {}
OpenGLRenderer::~OpenGLRenderer() {
    Shutdown();
}

bool OpenGLRenderer::Initialize(void* windowHandle) {
    m_Window = static_cast<GLFWwindow*>(windowHandle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { Log::Debug("ERROR: GLAD failed to initialize."); return false; }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_PickingShader = std::make_unique<Shader>("shaders/picking.vert", "shaders/picking.frag");
    m_HighlightShader = std::make_unique<Shader>("shaders/highlight.vert", "shaders/highlight.frag");
    m_BlitShader = std::make_unique<Shader>("shaders/blit.vert", "shaders/blit.frag");
    createPickingFramebuffer();
    createStaticSceneCache();
    createFullscreenQuad();
    Log::Debug("OpenGLRenderer Initialized successfully.");
    return true;
}

// NEW: Helper function to clean up all framebuffer-related resources.
void OpenGLRenderer::cleanupFramebuffers() {
    glDeleteFramebuffers(1, &m_PickingFBO);
    glDeleteTextures(1, &m_PickingTexture);
    glDeleteFramebuffers(1, &m_StaticSceneFBO);
    glDeleteTextures(1, &m_StaticSceneColorTexture);
    glDeleteTextures(1, &m_DepthTexture); // Delete the shared depth texture
}

void OpenGLRenderer::Shutdown() {
    Log::Debug("OpenGLRenderer shutdown.");
    cleanupFramebuffers(); // Use the helper here
    glDeleteVertexArrays(1, &m_FullscreenQuadVAO);
}

// NEW: This function is called by the application when the window resizes.
void OpenGLRenderer::OnWindowResize(int width, int height) {
    Log::Debug("Window resized, recreating framebuffers for new dimensions: ", width, "x", height);
    
    // First, delete the old framebuffer resources
    cleanupFramebuffers();
    
    // Then, create them again with the new window size
    createPickingFramebuffer();
    createStaticSceneCache();
}

void OpenGLRenderer::RenderStaticScene(const Scene& scene, const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_StaticSceneFBO);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();
    for (const auto& object : scene.GetSceneObjects()) {
        if (dynamic_cast<Grid*>(object.get())) {
            object->Draw(view, projection);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::DrawCachedStaticScene() {
    glDisable(GL_DEPTH_TEST);
    m_BlitShader->Bind();
    glBindVertexArray(m_FullscreenQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_StaticSceneColorTexture);
    m_BlitShader->SetUniform1i("screenTexture", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::RenderDynamicScene(const Scene& scene, const Camera& camera) {
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();
    for (const auto& object : scene.GetSceneObjects()) {
        if (!dynamic_cast<Grid*>(object.get())) {
            object->Draw(view, projection);
        }
    }
}

void OpenGLRenderer::BeginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndFrame() {
    glfwSwapBuffers(m_Window);
}

void OpenGLRenderer::RenderUI() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

uint32_t OpenGLRenderer::ProcessPicking(int x, int y, const Scene& scene, const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();
    for (const auto& object : scene.GetSceneObjects()) {
        if (!dynamic_cast<Grid*>(object.get())) {
            object->DrawForPicking(*m_PickingShader, view, projection);
        }
    }
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    uint32_t objectID = 0;
    int height;
    glfwGetWindowSize(m_Window, nullptr, &height);
    glReadPixels(x, height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &objectID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Log::Debug("Picking processed. Clicked on Object ID: ", objectID);
    return objectID;
}

void OpenGLRenderer::RenderHighlight(const ISceneObject &object, const Camera &camera) {
    Log::Debug("Rendering wireframe highlight for object '", object.name, "' (ID: ", object.id, ")");
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    m_HighlightShader->Bind();
    m_HighlightShader->SetUniformMat4f("u_Model", object.transform);
    m_HighlightShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_HighlightShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    m_HighlightShader->SetUniform4f("u_Color", 1.0f, 1.0f, 0.0f, 1.0f);
    object.DrawHighlight(camera.GetViewMatrix(), camera.GetProjectionMatrix());
    m_HighlightShader->Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
}

void OpenGLRenderer::createPickingFramebuffer() {
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    glGenFramebuffers(1, &m_PickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
    glGenTextures(1, &m_PickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_PickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PickingTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::Debug("ERROR: Picking FBO is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::createStaticSceneCache() {
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    glGenFramebuffers(1, &m_StaticSceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_StaticSceneFBO);
    glGenTextures(1, &m_StaticSceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_StaticSceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_StaticSceneColorTexture, 0);
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::Debug("ERROR: Static Scene FBO is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::createFullscreenQuad() {
    float quadVertices[] = { -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f };
    unsigned int VBO;
    glGenVertexArrays(1, &m_FullscreenQuadVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(m_FullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}