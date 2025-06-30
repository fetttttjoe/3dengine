
// =======================================================================
// File: src/Scene/Objects/Grid.h
// =======================================================================
#pragma once
#include "Interfaces.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Shader;

class Grid : public ISceneObject {
public:
    Grid(int size = 20, int divisions = 20);
    ~Grid() override;
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;
private:
    GLuint m_VAO = 0, m_VBO = 0;
    std::unique_ptr<Shader> m_Shader;
    int m_VertexCount = 0;
};

