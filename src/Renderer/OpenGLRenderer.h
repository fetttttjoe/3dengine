#pragma once
#include <glad/glad.h>

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Scene;
class Camera;
class ISceneObject;
class Shader;
class TransformGizmo;
struct GLFWwindow;
class IEditableMesh;
class Grid;

struct GpuMeshResources {
  GLuint vao = 0;
  GLuint vboPositions = 0;
  GLuint vboNormals = 0;
  GLuint ebo = 0;
  GLsizei indexCount = 0;

  void Release() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vboPositions) glDeleteBuffers(1, &vboPositions);
    if (vboNormals) glDeleteBuffers(1, &vboNormals);
    if (vao) glDeleteVertexArrays(1, &vao);
    vao = vboPositions = vboNormals = ebo = indexCount = 0;
  }
};

class OpenGLRenderer {
 public:
  OpenGLRenderer();
  ~OpenGLRenderer();

  bool Initialize(void* windowHandle);
  void Shutdown();

  void OnWindowResize(int width, int height);

  // --- Frame Lifecycle ---
  void BeginFrame();
  void EndFrame();

  // --- Scene Rendering ---
  void BeginSceneFrame();
  void EndSceneFrame();

  void RenderObject(const ISceneObject& object, const Camera& camera);
  void RenderObjectForPicking(const ISceneObject& object, Shader& pickingShader,
                              const Camera& camera);
  void RenderObjectHighlight(const ISceneObject& object, const Camera& camera);

  // --- Specialized Rendering Methods ---
  void RenderGizmo(const TransformGizmo& gizmo, const Camera& camera);
  void RenderGrid(const Grid& grid, const Camera& camera);

  // --- Sub-Object Rendering ---
  void RenderVertexHighlights(
      const IEditableMesh& mesh,
      const std::unordered_set<uint32_t>& selectedVertexIndices,
      const glm::mat4& modelMatrix, const Camera& camera);
  void RenderSelectedFaces(const IEditableMesh& mesh,
                           const std::unordered_set<uint32_t>& selectedFaces,
                           const glm::mat4& modelMatrix, const Camera& camera);

  // --- Picking ---
  uint32_t ProcessPicking(int x, int y, const Scene& scene,
                          const Camera& camera);
  uint32_t ProcessGizmoPicking(int x, int y, const TransformGizmo& gizmo,
                               const Camera& camera);

  // --- UI Rendering ---
  void RenderUI();

  // --- Resource Management ---
  uint32_t GetSceneTextureId() const { return m_SceneColorTexture; }
  void SyncSceneObjects(const Scene& scene);
  void ClearGpuResources();
  std::unordered_map<uint32_t, GpuMeshResources>& GetGpuResources() {
    return m_GpuResources;
  }

 private:
  void createFramebuffers();
  void cleanupFramebuffers();
  void createGizmoResources();
  void createAnchorMesh();
  void createGridResources(const Grid& grid);
  void updateGpuMesh(ISceneObject* object);

  GLFWwindow* m_Window;
  int m_Width, m_Height;

  // Framebuffers
  GLuint m_PickingFBO = 0, m_PickingTexture = 0;
  GLuint m_SceneFBO = 0, m_SceneColorTexture = 0, m_DepthTexture = 0,
         m_SceneDepthRBO = 0;

  // Shaders
  std::shared_ptr<Shader> m_PickingShader;
  std::shared_ptr<Shader> m_HighlightShader;
  std::shared_ptr<Shader> m_GizmoShader;
  std::shared_ptr<Shader> m_AnchorShader;
  std::shared_ptr<Shader> m_GridShader;
  std::shared_ptr<Shader> m_LitShader;

  // Mesh Data & GPU Buffers
  std::unordered_map<uint32_t, GpuMeshResources> m_GpuResources;

  // Gizmo Resources
  GLuint m_GizmoVAO = 0, m_GizmoVBO = 0, m_GizmoEBO = 0;
  GLsizei m_GizmoIndexCount = 0;

  // Anchor Resources
  GLuint m_AnchorVAO = 0, m_AnchorVBO = 0, m_AnchorEBO = 0;
  GLsizei m_AnchorIndexCount = 0;

  // Grid Resources
  GLuint m_GridVAO = 0, m_GridVBO = 0;

  // Sub-object selection resources
  GLuint m_SelectedFacesVAO = 0, m_SelectedFacesVBO = 0;
};