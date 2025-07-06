#pragma once
#include <glad/glad.h>

#include <cstdint>
#include <memory>

class Scene;
class Camera;
class ISceneObject;
class Shader;
class TransformGizmo;
struct GLFWwindow;

class OpenGLRenderer {
 public:
  OpenGLRenderer();
  ~OpenGLRenderer();

  bool Initialize(void* windowHandle);
  void Shutdown();

  // Called when the viewport panel is resized
  void OnWindowResize(int width, int height);

  // --- New Rendering Workflow ---
  // 1. Begin drawing the 3D scene to an offscreen texture
  void BeginSceneFrame();
  // 2. Draw all scene contents
  void RenderScene(const Scene& scene, const Camera& camera);
  void RenderHighlight(const ISceneObject& object, const Camera& camera);
  void RenderAnchors(const Scene& scene, const Camera& camera);
  // 3. Finish drawing the 3D scene
  void EndSceneFrame();

  // --- Final Composition ---
  // 4. Begin drawing to the main window
  void BeginFrame();
  // 5. Draw the ImGui interface
  void RenderUI();
  // 6. Swap buffers to display the final image
  void EndFrame();

  // --- Helpers ---
  // Processes a mouse click to see which object was selected
  uint32_t ProcessPicking(int x, int y, const Scene& scene,
                          const Camera& camera);
  uint32_t ProcessGizmoPicking(int x, int y, TransformGizmo& gizmo,
                               const Camera& camera);

  // Gives the UI the ID of the final rendered scene texture
  uint32_t GetSceneTextureId() const { return m_SceneColorTexture; }

 private:
  void createFramebuffers();
  void cleanupFramebuffers();
  void createAnchorMesh();

  GLFWwindow* m_Window;
  int m_Width, m_Height;

  // Framebuffers
  unsigned int m_PickingFBO;
  unsigned int m_PickingTexture;
  unsigned int m_SceneFBO;
  unsigned int m_SceneColorTexture;
  unsigned int m_DepthTexture;

  // Shaders
  std::shared_ptr<Shader> m_PickingShader;
  std::shared_ptr<Shader> m_HighlightShader;
  std::shared_ptr<Shader> m_AnchorShader;

  // Primitives
  unsigned int m_AnchorVAO;
  unsigned int m_AnchorVBO;
  unsigned int m_AnchorEBO;
  GLsizei m_AnchorIndexCount;
};