// =======================================================================
// File: src/Renderer/OpenGLRenderer.h
// =======================================================================
#pragma once
#include "Interfaces.h" // For IRenderer and ISceneObject

struct GLFWwindow;
class Shader; // Forward declare Shader

class OpenGLRenderer : public IRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    bool Initialize(void* windowHandle) override;
    void BeginFrame() override;
    void RenderScene(const Scene& scene, const Camera& camera) override;
    void RenderUI() override;
    void EndFrame() override;
    void Shutdown() override;
    uint32_t ProcessPicking(int x, int y, const Scene& scene, const Camera& camera) override;

    // New: Implementation for the highlight rendering
    void RenderHighlight(const ISceneObject& object, const Camera& camera) override;

private:
    void createPickingFramebuffer();

    GLFWwindow* m_Window = nullptr;
    std::unique_ptr<Shader> m_PickingShader;
    unsigned int m_PickingFBO = 0;
    unsigned int m_PickingTexture = 0;
    unsigned int m_DepthTexture = 0;

    // New: Shader for drawing outlines/highlights
    std::unique_ptr<Shader> m_HighlightShader;
};