// =======================================================================
// File: src/Scene/Objects/Grid.h
// =======================================================================
#pragma once
#include "Interfaces.h" // Ensures ISceneObject is included
#include <vector>
#include <memory>
#include <string> // For std::string for GetTypeString

class Shader; // Forward declaration

class Grid : public ISceneObject {
public:
    Grid(int size = 20, int divisions = 20);
    ~Grid() override;
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;
    void DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) override;

    // FIX: Add declarations for pure virtual functions from ISceneObject
    void DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const override; // ADD THIS
    std::string GetTypeString() const override; // ADD THIS (or inline if preferred)

private:
    unsigned int m_VAO = 0, m_VBO = 0;
    std::unique_ptr<Shader> m_Shader;
    int m_VertexCount = 0;
};