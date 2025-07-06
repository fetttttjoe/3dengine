// =======================================================================
// File: src/Scene/Objects/Grid.h
// =======================================================================
#pragma once
#include "Interfaces.h"
#include <vector>
#include <memory>
#include <string>

class Shader;

class Grid : public ISceneObject {
public:
    Grid(int size = 20, int divisions = 20);
    ~Grid() override;

    // ISceneObject Overrides
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;
    void DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) override;
    void DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const override;
    std::string GetTypeString() const override;
    const std::vector<ObjectProperty>& GetProperties() const override;

private:
    unsigned int m_VAO = 0, m_VBO = 0;
    // FIX: This must be a std::shared_ptr to match the ResourceManager return type.
    std::shared_ptr<Shader> m_Shader;
    int m_VertexCount = 0;
    std::vector<ObjectProperty> m_Properties; // Grid has no editable properties
};