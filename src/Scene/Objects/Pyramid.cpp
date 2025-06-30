
// =======================================================================
// File: src/Scene/Objects/Pyramid.cpp
// =======================================================================
#include "Scene/Objects/Pyramid.h"
#include "Shader.h"

Pyramid::Pyramid() {
    name = "Pyramid";
    float vertices[] = { -0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,-0.5f,0.5f, -0.5f,-0.5f,0.5f, 0.0f,0.5f,0.0f };
    unsigned int indices[] = { 0,1,2, 2,3,0, 0,1,4, 1,2,4, 2,3,4, 3,0,4 };
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    m_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}
Pyramid::~Pyramid() { glDeleteVertexArrays(1, &m_VAO); glDeleteBuffers(1, &m_VBO); glDeleteBuffers(1, &m_EBO); }

void Pyramid::Draw(const glm::mat4& view, const glm::mat4& projection) {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.8f, 0.2f, 0.3f, 1.0f);
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
}
