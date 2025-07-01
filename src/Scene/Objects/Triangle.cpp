#include "Scene/Objects/Triangle.h"
#include "Shader.h" // Adjust path as needed
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Triangle::Triangle() {
    name = "Triangle";
    // Basic triangle data
    float vertices[] = {
        // positions
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
    m_VertexCount = 3;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}

Triangle::~Triangle() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void Triangle::Draw(const glm::mat4& view, const glm::mat4& projection) {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f); // Red triangle

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
    glBindVertexArray(0);
}

void Triangle::DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) {
    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_Model", transform);
    pickingShader.SetUniformMat4f("u_View", view);
    pickingShader.SetUniformMat4f("u_Projection", projection);
    pickingShader.SetUniform1ui("u_ObjectID", id);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
    glBindVertexArray(0);
}

// FIX: Added 'const' here to match the declaration in Triangle.h
void Triangle::DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const {
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
    glBindVertexArray(0);
}

// GetTypeString() is defined inline in Triangle.h, so no redefinition here.