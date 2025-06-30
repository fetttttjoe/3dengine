
// =======================================================================
// File: src/Scene/Objects/Pyramid.h
// =======================================================================
#pragma once
#include "Interfaces.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

class Shader;

class Pyramid : public ISceneObject {
public:
    Pyramid();
    ~Pyramid() override;
    void Draw(const glm::mat4& view, const glm::mat4& projection) override;
private:
    GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
    std::unique_ptr<Shader> m_Shader;
};
