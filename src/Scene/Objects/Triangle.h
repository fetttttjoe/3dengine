#pragma once
#include "Interfaces.h"
#include <vector>
#include <memory>

class Shader;

class Triangle : public ISceneObject {
public:
    Triangle();
    ~Triangle() override;
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;
    void DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) override;
    void DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const override; // NEW

    std::string GetTypeString() const override { return "Triangle"; } // Already inline defined

private:
    unsigned int m_VAO, m_VBO;
    std::unique_ptr<Shader> m_Shader;
    int m_VertexCount;
};