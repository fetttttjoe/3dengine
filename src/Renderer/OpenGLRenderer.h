#pragma once
#include <cstdint>
#include <memory>

// Forward declarations
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

  void OnWindowResize(int width, int height);

  void BeginFrame();
  void RenderStaticScene(const Scene& scene, const Camera& camera);
  void DrawCachedStaticScene();
  void RenderDynamicScene(const Scene& scene, const Camera& camera);
  void RenderHighlight(const ISceneObject& object, const Camera& camera);
  void RenderUI();
  void EndFrame();

  uint32_t ProcessPicking(int x, int y, const Scene& scene,
                          const Camera& camera);
  uint32_t ProcessGizmoPicking(int x, int y, TransformGizmo& gizmo,
                               const Camera& camera);

 private:
  void createFramebuffers();
  void cleanupFramebuffers();
  void createFullscreenQuad();

  GLFWwindow* m_Window = nullptr;
  int m_Width = 0, m_Height = 0;

  // Framebuffers
  unsigned int m_PickingFBO = 0;
  unsigned int m_PickingTexture = 0;
  unsigned int m_StaticSceneFBO = 0;
  unsigned int m_StaticSceneColorTexture = 0;
  unsigned int m_DepthTexture = 0;  // Shared depth texture

  // Shaders
  std::shared_ptr<Shader> m_PickingShader;
  std::shared_ptr<Shader> m_HighlightShader;
  std::shared_ptr<Shader> m_BlitShader;

  // Fullscreen Quad for blitting
  unsigned int m_FullscreenQuadVAO = 0;
};