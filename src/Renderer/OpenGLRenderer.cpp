#include "OpenGLRenderer.h"
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/ResourceManager.h"
#include "Interfaces.h"
#include "Scene/Grid.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"
#include "Sculpting/SculptableMesh.h"
#include "Shader.h"
#include "imgui_impl_opengl3.h"

OpenGLRenderer::OpenGLRenderer()
    : m_Window(nullptr),
      m_Width(0),
      m_Height(0),
      m_PickingFBO(0),
      m_PickingTexture(0),
      m_SceneFBO(0),
      m_SceneColorTexture(0),
      m_DepthTexture(0),
      m_SceneDepthRBO(0),
      m_AnchorVAO(0),
      m_AnchorVBO(0),
      m_AnchorEBO(0),
      m_AnchorIndexCount(0) {}

OpenGLRenderer::~OpenGLRenderer() { Shutdown(); }

bool OpenGLRenderer::Initialize(void* windowHandle) {
  m_Window = static_cast<GLFWwindow*>(windowHandle);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Log::Debug("ERROR: GLAD failed to initialize.");
    return false;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_PickingShader = ResourceManager::LoadShader(
      "picking", "shaders/picking.vert", "shaders/picking.frag");
  m_HighlightShader = ResourceManager::LoadShader(
      "highlight", "shaders/highlight.vert", "shaders/highlight.frag");
  m_AnchorShader = ResourceManager::LoadShader(
      "anchor_shader", "shaders/default.vert", "shaders/default.frag");
  m_LitShader = ResourceManager::LoadShader("lit_shader", "shaders/lit.vert",
                                            "shaders/lit.frag");

  glfwGetWindowSize(m_Window, &m_Width, &m_Height);
  createFramebuffers();
  createAnchorMesh();

  Log::Debug("OpenGLRenderer Initialized successfully.");
  return true;
}

void OpenGLRenderer::cleanupFramebuffers() {
  glDeleteFramebuffers(1, &m_PickingFBO);
  glDeleteTextures(1, &m_PickingTexture);
  glDeleteFramebuffers(1, &m_SceneFBO);
  glDeleteTextures(1, &m_SceneColorTexture);
  glDeleteTextures(1, &m_DepthTexture);
  glDeleteRenderbuffers(1, &m_SceneDepthRBO);
}

void OpenGLRenderer::Shutdown() {
  Log::Debug("OpenGLRenderer shutdown.");
  cleanupFramebuffers();
  glDeleteVertexArrays(1, &m_AnchorVAO);
  glDeleteBuffers(1, &m_AnchorVBO);
  glDeleteBuffers(1, &m_AnchorEBO);
  for (auto& pair : m_GpuResources) {
    pair.second.Release();
  }
  m_GpuResources.clear();
}

void OpenGLRenderer::OnWindowResize(int width, int height) {
  if (width == 0 || height == 0 || (m_Width == width && m_Height == height))
    return;

  m_Width = width;
  m_Height = height;
  Log::Debug("Recreating scene framebuffer for new dimensions: ", width, "x",
             height);
  cleanupFramebuffers();
  createFramebuffers();
}

void OpenGLRenderer::BeginSceneFrame() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.12f, 0.13f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndSceneFrame() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void OpenGLRenderer::SyncSceneObjects(const Scene& scene) {
  for (auto it = m_GpuResources.begin(); it != m_GpuResources.end();) {
    if (scene.GetObjectByID(it->first) == nullptr) {
      it->second.Release();
      it = m_GpuResources.erase(it);
    } else {
      ++it;
    }
  }

  for (const auto& objectPtr : scene.GetSceneObjects()) {
    if (objectPtr && objectPtr->GetSculptableMesh() && objectPtr->IsMeshDirty()) {
      updateGpuMesh(objectPtr.get());
      objectPtr->SetMeshDirty(false);
    }
  }
}

void OpenGLRenderer::updateGpuMesh(ISceneObject* object) {
  if (!object) return;
  auto* meshData = object->GetSculptableMesh();
  if (!meshData || meshData->GetVertices().empty()) return;

  GpuMeshResources& res = m_GpuResources[object->id];
  
  if (res.vao == 0) {
    glGenVertexArrays(1, &res.vao);
    glGenBuffers(1, &res.vboPositions);
    glGenBuffers(1, &res.vboNormals);
    glGenBuffers(1, &res.ebo);
  }

  res.indexCount = static_cast<GLsizei>(meshData->GetIndexCount());
  const auto& vertices = meshData->GetVertices();
  const auto& normals = meshData->GetNormals();
  const auto& indices = meshData->GetIndices();

  glBindVertexArray(res.vao);
  glBindBuffer(GL_ARRAY_BUFFER, res.vboPositions);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  if (!normals.empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, res.vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
  
  Log::Debug("Updated GPU mesh for object ID: ", object->id);
}

void OpenGLRenderer::ClearGpuResources() {
    for (auto& pair : m_GpuResources) {
        pair.second.Release();
    }
    m_GpuResources.clear();
    Log::Debug("OpenGLRenderer: Cleared all GPU mesh resources.");
}
void OpenGLRenderer::RenderScene(const Scene& scene, const Camera& camera) {
    const auto& view = camera.GetViewMatrix();
    const auto& projection = camera.GetProjectionMatrix();

    for (const auto& object : scene.GetSceneObjects()) {
        object->Draw(*this, view, projection);
    }
}

void OpenGLRenderer::RenderAnchors(const Scene& scene, const Camera& camera) {
    if (!m_AnchorShader || m_AnchorVAO == 0) return;

    m_AnchorShader->Bind();
    m_AnchorShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_AnchorShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    m_AnchorShader->SetUniform4f("u_Color", 0.0f, 1.0f, 0.0f, 1.0f);

    glBindVertexArray(m_AnchorVAO);
    for (const auto& object : scene.GetSceneObjects()) {
        if (!object->isSelectable) {
            continue;
        }
        glm::vec4 anchor_local(0.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 objectTransform = object->GetTransform();
        glm::vec3 anchor_world = objectTransform * anchor_local;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), anchor_world);
        model = glm::scale(model, glm::vec3(0.05f));
        m_AnchorShader->SetUniformMat4f("u_Model", model);
        glDrawElements(GL_TRIANGLES, m_AnchorIndexCount, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}

void OpenGLRenderer::BeginFrame() {
  int display_w, display_h;
  glfwGetFramebufferSize(m_Window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::EndFrame() { glfwSwapBuffers(m_Window); }

void OpenGLRenderer::RenderUI() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

uint32_t OpenGLRenderer::ProcessPicking(int x, int y, const Scene& scene,
                                        const Camera& camera) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (const auto& object : scene.GetSceneObjects()) {
      if (object->isSelectable) {
          object->DrawForPicking(*m_PickingShader, camera.GetViewMatrix(), camera.GetProjectionMatrix());
      }
  }

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  uint32_t objectID = 0;
  glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
               &objectID);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return objectID;
}

// FIX: This function implementation is now present, which will resolve the linker error.
uint32_t OpenGLRenderer::ProcessGizmoPicking(int x, int y,
                                             TransformGizmo& gizmo,
                                             const Camera& camera) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gizmo.DrawForPicking(camera, *m_PickingShader);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  uint32_t objectID = 0;
  glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
               &objectID);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return objectID;
}

void OpenGLRenderer::RenderHighlight(const ISceneObject& object,
                                     const Camera& camera) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(2.5f);
  m_HighlightShader->Bind();
  m_HighlightShader->SetUniformMat4f("u_Model", object.GetTransform());
  m_HighlightShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_HighlightShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_HighlightShader->SetUniform4f("u_Color", 1.0f, 0.8f, 0.0f, 1.0f);
  
  object.DrawHighlight(camera.GetViewMatrix(), camera.GetProjectionMatrix());

  m_HighlightShader->Unbind();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1.0f);
}

void OpenGLRenderer::createFramebuffers() {
  glGenTextures(1, &m_DepthTexture);
  glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenFramebuffers(1, &m_PickingFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
  glGenTextures(1, &m_PickingTexture);
  glBindTexture(GL_TEXTURE_2D, m_PickingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, m_Width, m_Height, 0, GL_RED_INTEGER,
               GL_UNSIGNED_INT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_PickingTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_DepthTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    Log::Debug("ERROR: Picking FBO is not complete!");

  glGenFramebuffers(1, &m_SceneFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO);
  glGenTextures(1, &m_SceneColorTexture);
  glBindTexture(GL_TEXTURE_2D, m_SceneColorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_SceneColorTexture, 0);
                         
  glGenRenderbuffers(1, &m_SceneDepthRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_SceneDepthRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_SceneDepthRBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    Log::Debug("ERROR: Scene FBO is not complete!");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::createAnchorMesh() {
  float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,  0.5f,  0.5f,
                      0.5f,  -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
                      0.5f,  -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,  -0.5f, -0.5f};
  unsigned int indices[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
                            3, 2, 6, 6, 5, 3, 0, 1, 7, 7, 4, 0,
                            1, 7, 6, 6, 2, 1, 0, 4, 5, 5, 3, 0};
  m_AnchorIndexCount = 36;

  glGenVertexArrays(1, &m_AnchorVAO);
  glGenBuffers(1, &m_AnchorVBO);
  glGenBuffers(1, &m_AnchorEBO);

  glBindVertexArray(m_AnchorVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_AnchorVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_AnchorEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}