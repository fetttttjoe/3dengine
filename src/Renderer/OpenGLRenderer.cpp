#include "OpenGLRenderer.h"
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Core/Application.h"
#include "Core/Camera.h"
#include "Core/Log.h"
#include "Core/MathHelpers.h"
#include "Core/ResourceManager.h"
#include "Interfaces.h"
#include "Interfaces/IEditableMesh.h"
#include "Scene/Grid.h"
#include "Scene/Scene.h"
#include "Scene/TransformGizmo.h"
#include "Shader.h"
#include "imgui_impl_opengl3.h"
#include "Core/SettingsManager.h"
const char* GIZMO_VERTEX_SHADER_SRC = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
void main() { gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0); }
)glsl";

const char* GIZMO_FRAGMENT_SHADER_SRC = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 u_Color;
void main() { FragColor = u_Color; }
)glsl";

OpenGLRenderer::OpenGLRenderer() : m_Window(nullptr), m_Width(0), m_Height(0) {}

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
  m_GizmoShader = ResourceManager::LoadShaderFromMemory(
      "gizmo_shader", GIZMO_VERTEX_SHADER_SRC, GIZMO_FRAGMENT_SHADER_SRC);
  m_GridShader = ResourceManager::LoadShader(
      "grid_shader", "shaders/default.vert", "shaders/default.frag");
  m_LitShader = ResourceManager::LoadShader("lit_shader", "shaders/lit.vert",
                                            "shaders/lit.frag");
  m_UnlitShader = ResourceManager::LoadShader("unlit", "shaders/default.vert",
                                              "shaders/unlit.frag");

  glfwGetWindowSize(m_Window, &m_Width, &m_Height);
  createFramebuffers();
  createGizmoResources();
  createAnchorMesh();

  glGenVertexArrays(1, &m_SelectedFacesVAO);
  glGenBuffers(1, &m_SelectedFacesVBO);
  glBindVertexArray(m_SelectedFacesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedFacesVBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  glGenVertexArrays(1, &m_SelectedEdgesVAO);
  glGenBuffers(1, &m_SelectedEdgesVBO);
  glBindVertexArray(m_SelectedEdgesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedEdgesVBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  glGenVertexArrays(1, &m_SelectedVerticesVAO);
  glGenBuffers(1, &m_SelectedVerticesVBO);
  glBindVertexArray(m_SelectedVerticesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedVerticesVBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  glGenVertexArrays(1, &m_HighlightedPathVAO);
  glGenBuffers(1, &m_HighlightedPathVBO);
  glBindVertexArray(m_HighlightedPathVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_HighlightedPathVBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  glBindVertexArray(0);

  Log::Debug("OpenGLRenderer Initialized successfully.");
  return true;
}

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
    if (objectPtr && objectPtr->GetEditableMesh() && objectPtr->IsMeshDirty()) {
      updateGpuMesh(objectPtr.get());
      objectPtr->SetMeshDirty(false);
    }
  }
}

void OpenGLRenderer::updateGpuMesh(ISceneObject* object) {
  if (!object) return;
  auto* meshData = object->GetEditableMesh();
  if (!meshData || meshData->GetVertices().empty()) return;

  GpuMeshResources& res = m_GpuResources[object->id];

  if (res.vao == 0) {
    glGenVertexArrays(1, &res.vao);
    glGenBuffers(1, &res.vboPositions);
    glGenBuffers(1, &res.vboNormals);
    glGenBuffers(1, &res.ebo);
  }

  res.indexCount = static_cast<GLsizei>(meshData->GetIndices().size());
  const auto& vertices = meshData->GetVertices();
  const auto& normals = meshData->GetNormals();
  const auto& indices = meshData->GetIndices();

  glBindVertexArray(res.vao);
  glBindBuffer(GL_ARRAY_BUFFER, res.vboPositions);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  if (!normals.empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, res.vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
                 normals.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void*)0);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);

  Log::Debug("Updated GPU mesh for object ID: ", object->id);
}

void OpenGLRenderer::cleanupFramebuffers() {
  glDeleteFramebuffers(1, &m_PickingFBO);
  glDeleteTextures(1, &m_PickingTexture);
  glDeleteFramebuffers(1, &m_SceneFBO);
  glDeleteTextures(1, &m_SceneColorTexture);
  glDeleteTextures(1, &m_DepthTexture);
  glDeleteRenderbuffers(1, &m_SceneDepthRBO);
}

void OpenGLRenderer::ClearAllGpuResources() {
  for (auto& pair : m_GpuResources) {
    pair.second.Release();
  }
  m_GpuResources.clear();
}

void OpenGLRenderer::Shutdown() {
  Log::Debug("OpenGLRenderer shutdown.");
  cleanupFramebuffers();

  ClearAllGpuResources();

  if (m_GizmoVAO != 0) glDeleteVertexArrays(1, &m_GizmoVAO);
  if (m_GizmoVBO != 0) glDeleteBuffers(1, &m_GizmoVBO);
  if (m_GizmoEBO != 0) glDeleteBuffers(1, &m_GizmoEBO);
  m_GizmoVAO = m_GizmoVBO = m_GizmoEBO = 0;

  if (m_AnchorVAO != 0) glDeleteVertexArrays(1, &m_AnchorVAO);
  if (m_AnchorVBO != 0) glDeleteBuffers(1, &m_AnchorVBO);
  if (m_AnchorEBO != 0) glDeleteBuffers(1, &m_AnchorEBO);
  m_AnchorVAO = m_AnchorVBO = m_AnchorEBO = 0;

  if (m_GridVAO != 0) glDeleteVertexArrays(1, &m_GridVAO);
  if (m_GridVBO != 0) glDeleteBuffers(1, &m_GridVBO);
  m_GridVAO = m_GridVBO = 0;

  if (m_SelectedFacesVAO != 0) glDeleteVertexArrays(1, &m_SelectedFacesVAO);
  if (m_SelectedFacesVBO != 0) glDeleteBuffers(1, &m_SelectedFacesVBO);
  m_SelectedFacesVAO = m_SelectedFacesVBO = 0;
  if (m_SelectedEdgesVAO != 0) glDeleteVertexArrays(1, &m_SelectedEdgesVAO);
  if (m_SelectedEdgesVBO != 0) glDeleteBuffers(1, &m_SelectedEdgesVBO);
  m_SelectedEdgesVAO = m_SelectedEdgesVBO = 0;
  if (m_SelectedVerticesVAO != 0)
    glDeleteVertexArrays(1, &m_SelectedVerticesVAO);
  if (m_SelectedVerticesVBO != 0) glDeleteBuffers(1, &m_SelectedVerticesVBO);
  m_SelectedVerticesVAO = m_SelectedVerticesVBO = 0;
  if (m_HighlightedPathVAO != 0) glDeleteVertexArrays(1, &m_HighlightedPathVAO);
  if (m_HighlightedPathVBO != 0) glDeleteBuffers(1, &m_HighlightedPathVBO);
  m_HighlightedPathVAO = m_HighlightedPathVBO = 0;
}

void OpenGLRenderer::RenderSelectedFaces(
    const IEditableMesh& mesh,
    const std::unordered_set<uint32_t>& selectedFaces,
    const glm::mat4& modelMatrix, const Camera& camera) {
  if (selectedFaces.empty() || !m_LitShader || m_SelectedFacesVAO == 0) return;
  std::vector<glm::vec3> faceVertices;
  const auto& meshVertices = mesh.GetVertices();
  const auto& meshIndices = mesh.GetIndices();
  for (uint32_t faceIndex : selectedFaces) {
    size_t baseIdx = faceIndex * 3;
    if (baseIdx + 2 < meshIndices.size()) {
      faceVertices.push_back(meshVertices[meshIndices[baseIdx]]);
      faceVertices.push_back(meshVertices[meshIndices[baseIdx + 1]]);
      faceVertices.push_back(meshVertices[meshIndices[baseIdx + 2]]);
    }
  }
  if (faceVertices.empty()) return;
  m_LitShader->Bind();
  m_LitShader->SetUniformMat4f("u_Model", modelMatrix);
  m_LitShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_LitShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_LitShader->SetUniformVec4("u_Color", SettingsManager::Get().selectedFacesColor);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindVertexArray(m_SelectedFacesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedFacesVBO);
  glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(glm::vec3),
               faceVertices.data(), GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(faceVertices.size()));
  glBindVertexArray(0);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::RenderObjectAsGhost(const ISceneObject& object,
                                         const Camera& camera,
                                         const glm::vec4& color) {
  auto it = m_GpuResources.find(object.id);
  if (it == m_GpuResources.end()) return;
  const GpuMeshResources& res = it->second;
  if (res.vao == 0 || res.indexCount == 0) return;

  m_UnlitShader->Bind();
  m_UnlitShader->SetUniformMat4f("u_Model", object.GetTransform());
  m_UnlitShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_UnlitShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_UnlitShader->SetUniformVec4("u_Color", color);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  glBindVertexArray(res.vao);
  glDrawElements(GL_TRIANGLES, res.indexCount, GL_UNSIGNED_INT, nullptr);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(1.0f);
  m_UnlitShader->SetUniformVec4(
      "u_Color", glm::vec4(color.r, color.g, color.b, color.a * 1.5f));
  glDrawElements(GL_TRIANGLES, res.indexCount, GL_UNSIGNED_INT, nullptr);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glBindVertexArray(0);
}

void OpenGLRenderer::RenderVertexHighlights(
    const IEditableMesh& mesh,
    const std::unordered_set<uint32_t>& selectedVertexIndices,
    const glm::mat4& modelMatrix, const Camera& camera) {
  if (selectedVertexIndices.empty() || !m_LitShader) return;

  glPointSize(10.0f);
  glDisable(GL_DEPTH_TEST);
  m_LitShader->Bind();
  m_LitShader->SetUniformMat4f("u_Model", modelMatrix);
  m_LitShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_LitShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_LitShader->SetUniformVec4("u_Color", SettingsManager::Get().vertexHighlightColor);

  std::vector<glm::vec3> points;
  const auto& vertices = mesh.GetVertices();
  for (uint32_t index : selectedVertexIndices) {
    if (index < vertices.size()) {
      points.push_back(vertices[index]);
    }
  }

  if (points.empty()) {
    glEnable(GL_DEPTH_TEST);
    return;
  }

  glBindVertexArray(m_SelectedVerticesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedVerticesVBO);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3),
               points.data(), GL_DYNAMIC_DRAW);
  glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));
  glBindVertexArray(0);
  glPointSize(1.0f);
  glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::RenderSelectedEdges(
    const IEditableMesh& mesh,
    const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>&
        selectedEdges,
    const glm::mat4& modelMatrix, const Camera& camera) {
  if (selectedEdges.empty() || !m_LitShader) return;

  glLineWidth(4.0f);
  glDisable(GL_DEPTH_TEST);
  m_LitShader->Bind();
  m_LitShader->SetUniformMat4f("u_Model", modelMatrix);
  m_LitShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_LitShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_LitShader->SetUniformVec4("u_Color", SettingsManager::Get().edgeHighlightColor);

  std::vector<glm::vec3> lines;
  const auto& vertices = mesh.GetVertices();
  for (const auto& edge : selectedEdges) {
    if (edge.first < vertices.size() && edge.second < vertices.size()) {
      lines.push_back(vertices[edge.first]);
      lines.push_back(vertices[edge.second]);
    }
  }

  if (lines.empty()) {
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    return;
  }

  glBindVertexArray(m_SelectedEdgesVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_SelectedEdgesVBO);
  glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(),
               GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
  glBindVertexArray(0);
  glLineWidth(1.0f);
  glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::RenderHighlightedPath(
    const IEditableMesh& mesh,
    const std::vector<std::pair<uint32_t, uint32_t>>& path,
    const glm::mat4& modelMatrix, const Camera& camera) {
  if (path.empty() || !m_LitShader) return;

  glLineWidth(6.0f);
  glDisable(GL_DEPTH_TEST);
  m_LitShader->Bind();
  m_LitShader->SetUniformMat4f("u_Model", modelMatrix);
  m_LitShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_LitShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_LitShader->SetUniformVec4("u_Color", SettingsManager::Get().pathHighlightColor);

  std::vector<glm::vec3> lines;
  const auto& vertices = mesh.GetVertices();
  for (const auto& edge : path) {
    if (edge.first < vertices.size() && edge.second < vertices.size()) {
      lines.push_back(vertices[edge.first]);
      lines.push_back(vertices[edge.second]);
    }
  }

  if (lines.empty()) {
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    return;
  }

  glBindVertexArray(m_HighlightedPathVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_HighlightedPathVBO);
  glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(),
               GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
  glBindVertexArray(0);
  glLineWidth(1.0f);
  glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::OnWindowResize(int width, int height) {
  if (width == 0 || height == 0 || (m_Width == width && m_Height == height))
    return;
  m_Width = width;
  m_Height = height;
  cleanupFramebuffers();
  createFramebuffers();
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

void OpenGLRenderer::BeginSceneFrame() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.12f, 0.13f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndSceneFrame() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void OpenGLRenderer::RenderObject(const ISceneObject& object,
                                  const Camera& camera) {
  auto shader = object.GetShader();
  if (!shader) return;

  auto it = m_GpuResources.find(object.id);
  if (it == m_GpuResources.end()) return;

  const GpuMeshResources& res = it->second;
  if (res.vao == 0 || res.indexCount == 0) return;

  shader->Bind();
  shader->SetUniformMat4f("u_Model", object.GetTransform());
  shader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  shader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  shader->SetUniformVec4("u_Color",
                         object.GetPropertySet().GetValue<glm::vec4>("Color"));
  shader->SetUniformVec3("u_ViewPos", camera.GetPosition());

  glBindVertexArray(res.vao);
  glDrawElements(GL_TRIANGLES, res.indexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void OpenGLRenderer::RenderObjectHighlight(const ISceneObject& object,
                                           const Camera& camera) {
  auto it = m_GpuResources.find(object.id);
  if (it == m_GpuResources.end()) return;
  const GpuMeshResources& res = it->second;
  if (res.vao == 0 || res.indexCount == 0) return;

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(2.5f);
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonOffset(-1.0, -1.0);

  m_HighlightShader->Bind();
  m_HighlightShader->SetUniformMat4f("u_Model", object.GetTransform());
  m_HighlightShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_HighlightShader->SetUniformMat4f("u_Projection",
                                     camera.GetProjectionMatrix());
  m_HighlightShader->SetUniform4f("u_Color", SettingsManager::Get().vertexHighlightColor);

  glBindVertexArray(res.vao);
  glDrawElements(GL_TRIANGLES, res.indexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  m_HighlightShader->Unbind();
  glDisable(GL_POLYGON_OFFSET_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1.0f);
}

void OpenGLRenderer::RenderObjectForPicking(const ISceneObject& object,
                                            Shader& pickingShader,
                                            const Camera& camera) {
  auto it = m_GpuResources.find(object.id);
  if (it == m_GpuResources.end()) return;
  const GpuMeshResources& res = it->second;
  if (res.vao == 0 || res.indexCount == 0) return;

  pickingShader.Bind();
  pickingShader.SetUniformMat4f("u_Model", object.GetTransform());
  pickingShader.SetUniformMat4f("u_View", camera.GetViewMatrix());
  pickingShader.SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  pickingShader.SetUniform1ui("u_ObjectID", object.id);

  glBindVertexArray(res.vao);
  glDrawElements(GL_TRIANGLES, res.indexCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void OpenGLRenderer::RenderGizmo(const TransformGizmo& gizmo,
                                 const Camera& camera) {
  if (!gizmo.GetTarget() || gizmo.GetHandles().empty()) return;

  m_GizmoShader->Bind();
  m_GizmoShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_GizmoShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());

  float distance =
      glm::length(camera.GetPosition() - gizmo.GetTarget()->GetPosition());
  float viz_scale = distance * 0.02f;

  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(m_GizmoVAO);

  for (const auto& handle : gizmo.GetHandles()) {
    m_GizmoShader->SetUniformVec4("u_Color", handle.color);
    glm::mat4 handleModelMatrix =
        gizmo.CalculateHandleModelMatrix(handle, camera, viz_scale);
    m_GizmoShader->SetUniformMat4f("u_Model", handleModelMatrix);
    glDrawElements(GL_TRIANGLES, m_GizmoIndexCount, GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);
}
void OpenGLRenderer::RenderGrid(const Grid& grid, const Camera& camera) {
  if (!m_GridShader) return;

  if (grid.IsMeshDirty()) {
    createGridResources(grid);
  }
  if (m_GridVAO == 0) return;

  m_GridShader->Bind();
  m_GridShader->SetUniformMat4f("u_Model", grid.GetTransform());
  m_GridShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_GridShader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
  m_GridShader->SetUniform4f("u_Color", 0.3f, 0.3f, 0.3f, 1.0f);

  glBindVertexArray(m_GridVAO);
  glDrawArrays(GL_LINES, 0, grid.GetVertexCount());
  glBindVertexArray(0);
}

// --- Picking Implementations ---
uint32_t OpenGLRenderer::ProcessPicking(int x, int y, const Scene& scene,
                                        const Camera& camera) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (const auto& object : scene.GetSceneObjects()) {
    if (object->isSelectable) {
      object->DrawForPicking(*m_PickingShader, camera.GetViewMatrix(),
                             camera.GetProjectionMatrix());
    }
  }

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  uint32_t objectID = 0;
  glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
               &objectID);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return objectID;
}

uint32_t OpenGLRenderer::ProcessGizmoPicking(int x, int y,
                                             const TransformGizmo& gizmo,
                                             const Camera& camera) {
  if (!gizmo.GetTarget()) return 0;

  glBindFramebuffer(GL_FRAMEBUFFER, m_PickingFBO);
  glViewport(0, 0, m_Width, m_Height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_PickingShader->Bind();
  m_PickingShader->SetUniformMat4f("u_View", camera.GetViewMatrix());
  m_PickingShader->SetUniformMat4f("u_Projection",
                                   camera.GetProjectionMatrix());

  float distance =
      glm::length(camera.GetPosition() - gizmo.GetTarget()->GetPosition());
  float viz_scale = distance * 0.02f;

  glBindVertexArray(m_GizmoVAO);
  for (const auto& handle : gizmo.GetHandles()) {
    m_PickingShader->SetUniform1ui("u_ObjectID", handle.id);
    glm::mat4 handleModelMatrix =
        gizmo.CalculateHandleModelMatrix(handle, camera, viz_scale);
    m_PickingShader->SetUniformMat4f("u_Model", handleModelMatrix);
    glDrawElements(GL_TRIANGLES, m_GizmoIndexCount, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  uint32_t objectID = 0;
  glReadPixels(x, m_Height - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
               &objectID);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return objectID;
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
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width,
                        m_Height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_SceneDepthRBO);

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

void OpenGLRenderer::createGizmoResources() {
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f,
                      0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f};
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};
  m_GizmoIndexCount = 6;
  glGenVertexArrays(1, &m_GizmoVAO);
  glGenBuffers(1, &m_GizmoVBO);
  glGenBuffers(1, &m_GizmoEBO);
  glBindVertexArray(m_GizmoVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_GizmoVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GizmoEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void OpenGLRenderer::createGridResources(const Grid& grid) {
  if (m_GridVAO == 0) {
    glGenVertexArrays(1, &m_GridVAO);
    glGenBuffers(1, &m_GridVBO);
  }

  glBindVertexArray(m_GridVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
  const auto& vertices = grid.GetVertices();
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  const_cast<Grid&>(grid).SetMeshDirty(false);
}