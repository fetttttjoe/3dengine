#pragma once
#include <memory>
#include <cstdint>

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

    uint32_t ProcessPicking(int x, int y, const Scene& scene, const Camera& camera);
    // NEW: Method to specifically pick gizmo handles
    uint32_t ProcessGizmoPicking(int x, int y, TransformGizmo& gizmo, const Camera& camera);

private:
    void createPickingFramebuffer();
    void createStaticSceneCache();
    void createFullscreenQuad();
    void cleanupFramebuffers();

    GLFWwindow* m_Window = nullptr;

    // Framebuffers
    unsigned int m_PickingFBO = 0;
    unsigned int m_PickingTexture = 0;
    unsigned int m_StaticSceneFBO = 0;
    unsigned int m_StaticSceneColorTexture = 0;
    unsigned int m_DepthTexture = 0;

    // Shaders
    std::unique_ptr<Shader> m_PickingShader;
    std::unique_ptr<Shader> m_HighlightShader;
    std::unique_ptr<Shader> m_BlitShader;

    // Fullscreen Quad for blitting
    unsigned int m_FullscreenQuadVAO = 0;
};
