#pragma once

#include <memory>
#include <cstdint>
#include "glm/glm.hpp"

// Forward declarations
class Scene;
class Camera;
class ISceneObject;
class Shader;
struct GLFWwindow;

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool Initialize(void* windowHandle);
    void Shutdown();
    
    // NEW: Public method to handle window resizing
    void OnWindowResize(int width, int height);
    
    void BeginFrame();
    void EndFrame();
    
    void RenderStaticScene(const Scene& scene, const Camera& camera);
    void DrawCachedStaticScene();
    void RenderDynamicScene(const Scene& scene, const Camera& camera);
    void RenderHighlight(const ISceneObject& object, const Camera& camera);
    void RenderUI();
    
    uint32_t ProcessPicking(int x, int y, const Scene& scene, const Camera& camera);

private:
    void createPickingFramebuffer();
    void createStaticSceneCache();
    void createFullscreenQuad();
    
    // NEW: Private helper to delete framebuffer resources before recreating them
    void cleanupFramebuffers();

    GLFWwindow* m_Window = nullptr;

    // --- Framebuffer Objects ---
    uint32_t m_PickingFBO = 0;
    uint32_t m_PickingTexture = 0;
    
    uint32_t m_StaticSceneFBO = 0;
    uint32_t m_StaticSceneColorTexture = 0;
    
    // NOTE: This depth texture is shared by both FBOs
    uint32_t m_DepthTexture = 0; 
    
    // --- Shaders ---
    std::unique_ptr<Shader> m_PickingShader;
    std::unique_ptr<Shader> m_HighlightShader;
    std::unique_ptr<Shader> m_BlitShader;
    
    // --- Geometry ---
    uint32_t m_FullscreenQuadVAO = 0;
};