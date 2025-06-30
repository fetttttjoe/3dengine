
// =======================================================================
// File: src/Scene/Objects/Triangle.cpp
// =======================================================================
#include "Scene/Objects/Triangle.h"
#include "Shader.h"

Triangle::Triangle() {
    name = "Triangle";
    float positions[] = { -0.5f,-0.5f,0.0f, 0.5f,-0.5f,0.0f, 0.0f,0.5f,0.0f };
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    m_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}
Triangle::~Triangle() { glDeleteVertexArrays(1, &m_VAO); glDeleteBuffers(1, &m_VBO); }

void Triangle::Draw(const glm::mat4& view, const glm::mat4& projection) {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.2f, 0.5f, 0.8f, 1.0f);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

