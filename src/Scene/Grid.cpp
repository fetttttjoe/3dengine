
// =======================================================================
// File: src/Scene/Objects/Grid.cpp
// =======================================================================
#include "Grid.h"
#include "Shader.h"

Grid::Grid(int size, int divisions) {
    name = "Grid";
    std::vector<float> vertices;
    float step = (float)size / divisions;
    float halfSize = (float)size / 2.0f;
    for (int i = 0; i <= divisions; ++i) {
        float pos = -halfSize + i * step;
        vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(pos);
        vertices.push_back(halfSize);  vertices.push_back(0.0f); vertices.push_back(pos);
        vertices.push_back(pos); vertices.push_back(0.0f); vertices.push_back(-halfSize);
        vertices.push_back(pos); vertices.push_back(0.0f); vertices.push_back(halfSize);
    }
    m_VertexCount = static_cast<int>(vertices.size() / 3);
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    m_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}
Grid::~Grid() { glDeleteVertexArrays(1, &m_VAO); glDeleteBuffers(1, &m_VBO); }

void Grid::Draw(const glm::mat4& view, const glm::mat4& projection) {
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.3f, 0.3f, 0.3f, 1.0f);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, m_VertexCount);
}
