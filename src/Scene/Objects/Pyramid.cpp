#include "Scene/Objects/Pyramid.h"
#include "Shader.h" // Assuming Shader.h is in Renderer/
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr if needed, though not directly used in this snippet

Pyramid::Pyramid() {
    name = "Pyramid";
    m_VAO = 0;
    m_VBO = 0;
    m_EBO = 0;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    m_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    Build(); // Call Build to setup initial mesh data
}

Pyramid::~Pyramid() {
    if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
    if (m_EBO != 0) glDeleteBuffers(1, &m_EBO);
}

void Pyramid::Build() {
    float w = m_Width / 2.0f;
    float h = m_Height;
    float d = m_Depth / 2.0f;

    float vertices[] = {
        // Base
        -w, 0.0f, -d, // 0
         w, 0.0f, -d, // 1
         w, 0.0f,  d, // 2
        -w, 0.0f,  d, // 3
        // Apex
         0.0f, h, 0.0f // 4
    };

    unsigned int indices[] = {
        // Base triangles (CCW winding)
        0, 1, 2,
        2, 3, 0,
        // Side triangles
        // Front face (0-1-4)
        0, 1, 4,
        // Right face (1-2-4)
        1, 2, 4,
        // Back face (2-3-4)
        2, 3, 4,
        // Left face (3-0-4)
        3, 0, 4
    };

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind VAO
}

void Pyramid::RebuildMesh() {
    Build();
}

std::vector<ObjectProperty> Pyramid::GetProperties() {
    return {
        {"Width", &m_Width},
        {"Height", &m_Height},
        {"Depth", &m_Depth}
    };
}

void Pyramid::Draw(const glm::mat4& view, const glm::mat4& projection) {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.8f, 0.2f, 0.3f, 1.0f);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Pyramid::DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) {
    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_Model", transform);
    pickingShader.SetUniformMat4f("u_View", view);
    pickingShader.SetUniformMat4f("u_Projection", projection);
    pickingShader.SetUniform1ui("u_ObjectID", id);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// FIX: Added 'const' here to match the declaration in Pyramid.h
void Pyramid::DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}