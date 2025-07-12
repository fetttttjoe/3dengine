#pragma once
#include <glad/glad.h>

#include <cstdint>
#include <memory>
#include <unordered_map>

class Scene;
class Camera;
class ISceneObject;
class Shader;
class TransformGizmo;
struct GLFWwindow;

// Struct to hold GPU resources for a mesh
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

  void BeginSceneFrame();
  void RenderScene(const Scene& scene, const Camera& camera);
  void RenderHighlight(const ISceneObject& object, const Camera& camera);
  void RenderAnchors(const Scene& scene, const Camera& camera);
  void EndSceneFrame();

  void BeginFrame();
  void RenderUI();
  void EndFrame();

  uint32_t ProcessPicking(int x, int y, const Scene& scene,
                          const Camera& camera);
  // This declaration must match the implementation to fix the linker error
  uint32_t ProcessGizmoPicking(int x, int y, TransformGizmo& gizmo,
                               const Camera& camera);

  uint32_t GetSceneTextureId() const { return m_SceneColorTexture; }

  void SyncSceneObjects(const Scene& scene);
  void ClearGpuResources();

  std::unordered_map<uint32_t, GpuMeshResources>& GetGpuResources() {
    return m_GpuResources;
  }

 private:
  void createFramebuffers();
  void cleanupFramebuffers();
  void createAnchorMesh();

  void updateGpuMesh(ISceneObject* object);

  GLFWwindow* m_Window;
  int m_Width, m_Height;

  unsigned int m_PickingFBO;
  unsigned int m_PickingTexture;
  unsigned int m_SceneFBO;
  unsigned int m_SceneColorTexture;
  unsigned int m_DepthTexture;
  unsigned int m_SceneDepthRBO;

  std::shared_ptr<Shader> m_PickingShader;
  std::shared_ptr<Shader> m_HighlightShader;
  std::shared_ptr<Shader> m_AnchorShader;
  std::shared_ptr<Shader> m_LitShader;

  unsigned int m_AnchorVAO;
  unsigned int m_AnchorVBO;
  unsigned int m_AnchorEBO;
  GLsizei m_AnchorIndexCount;

  std::unordered_map<uint32_t, GpuMeshResources> m_GpuResources;
};