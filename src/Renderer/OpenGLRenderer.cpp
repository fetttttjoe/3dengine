#include "OpenGLRenderer.h"
#include "Core/Log.h"
#include "Core/ResourceManager.h"
#include "Scene/Scene.h"
#include "Core/Camera.h"
#include "Shader.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "Scene/TransformGizmo.h"
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::Debug("ERROR: GLAD failed to initialize.");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders via ResourceManager
    m_PickingShader = ResourceManager::LoadShader("picking", "shaders/picking.vert", "shaders/picking.frag");
    m_HighlightShader = ResourceManager::LoadShader("highlight", "shaders/highlight.vert", "shaders/highlight.frag");
    m_BlitShader = ResourceManager::LoadShader("blit", "shaders/blit.vert", "shaders/blit.frag");

    glfwGetWindowSize(m_Window, &m_Width, &m_Height);
    createFramebuffers();
    createFullscreenQuad();

    Log::Debug("OpenGLRenderer Initialized successfully.");
    return true;
}

void OpenGLRenderer::cleanupFramebuffers() {
    glDeleteFramebuffers(1, &m_PickingFBO);
    glDeleteTextures(1, &m_PickingTexture);
    glDeleteFramebuffers(1, &m_StaticSceneFBO);
    glDeleteTextures(1, &m_StaticSceneColorTexture);
    glDeleteTextures(1, &m_DepthTexture); 
}

void OpenGLRenderer::Shutdown() {
    Log::Debug("OpenGLRenderer shutdown.");
    cleanupFramebuffers();
    glDeleteVertexArrays(1, &m_FullscreenQuadVAO);
}

void OpenGLRenderer::OnWindowResize(int width, int height) {
    if (width == 0 || height == 0) return;
    m_Width = width;
    m_Height = height;
    Log::Debug("Window resized, recreating framebuffers for new dimensions: ", width, "x", height);
    cleanupFramebuffers();
    createFramebuffers();
}

void OpenGLRenderer::RenderStaticScene(const Scene& scene, const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_StaticSceneFBO);
    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();
    
    // Render only static objects (like the Grid) to this cache
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

    // Render non-static objects
    for (const auto& object : scene.GetSceneObjects()) {
        if (!dynamic_cast<Grid*>(object.get())) {
            object->Draw(view, projection);
        }
    }
}

void OpenGLRenderer::BeginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_Width, m_Height);
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
    glViewport(0, 0, m_Width, m_Height);
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
    // Y is inverted between window coordinates and framebuffer/texture coordinates
    glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &objectID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Log::Debug("Picking processed. Clicked on Object ID: ", objectID);
    return objectID;
}

uint32_t OpenGLRenderer::ProcessGizmoPicking(int x, int y, TransformGizmo& gizmo, const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
    glViewport(0, 0, m_Width, m_Height);
    // Clear only color, gizmo should draw over existing depth values
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    gizmo.DrawForPicking(camera, *m_PickingShader);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    uint32_t objectID = 0;
    glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &objectID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return objectID;
}

void OpenGLRenderer::RenderHighlight(const ISceneObject &object, const Camera &camera) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.5f);
    m_HighlightShader->Bind();
    m_HighlightShader->SetUniformMat4f("u_Model", object.transform);
    m_HighlightShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_HighlightShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    m_HighlightShader->SetUniform4f("u_Color", 1.0f, 0.8f, 0.0f, 1.0f); // Orange highlight
    
    // Use the object's own draw call for its geometry
    object.DrawHighlight(camera.GetViewMatrix(), camera.GetProjectionMatrix());
    
    m_HighlightShader->Unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
}

void OpenGLRenderer::createFramebuffers() {
    // --- Shared Depth Texture ---
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // --- Picking FBO ---
    glGenFramebuffers(1, &m_PickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
    glGenTextures(1, &m_PickingTexture);
    glBindTexture(GL_TEXTURE_2D, m_PickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_Width, m_Height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PickingTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::Debug("ERROR: Picking FBO is not complete!");
    
    // --- Static Scene Cache FBO ---
    glGenFramebuffers(1, &m_StaticSceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_StaticSceneFBO);
    glGenTextures(1, &m_StaticSceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_StaticSceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_StaticSceneColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::Debug("ERROR: Static Scene FBO is not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::createFullscreenQuad() {
    float quadVertices[] = { 
        // positions   // texCoords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    unsigned int vbo;
    glGenVertexArrays(1, &m_FullscreenQuadVAO);
    glGenBuffers(1, &vbo);
    glBindVertexArray(m_FullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}