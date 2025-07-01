#pragma once
#include "Interfaces.h" // Ensures ISceneObject is included
#include <memory>
#include <vector> // For std::vector in GetProperties

// Forward-declaration for Shader
class Shader;

class Pyramid : public ISceneObject {
public:
    Pyramid();
    ~Pyramid() override;

    // Main function for drawing the object visually
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;

    // Special function for drawing the object's ID for mouse picking
    void DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) override;

    // FIX: Added 'const' here to match the ISceneObject declaration and cpp definition
    void DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const override;

    // Already inline defined in the header, so no separate .cpp definition is needed
    std::string GetTypeString() const override { return "Pyramid"; }

    // Methods for the UI to interact with this object's unique properties
    std::vector<ObjectProperty> GetProperties() override;
    void RebuildMesh() override;

private:
    void Build(); // Helper function to generate the mesh vertices
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    std::unique_ptr<Shader> m_Shader;

    // Configurable properties for the pyramid's shape
    float m_Width = 1.0f;
    float m_Height = 1.0f;
    float m_Depth = 1.0f;
};