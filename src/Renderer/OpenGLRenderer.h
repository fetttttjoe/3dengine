#pragma once
#include "Interfaces.h"

struct GLFWwindow;

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

private:
    GLFWwindow* m_Window = nullptr;
};