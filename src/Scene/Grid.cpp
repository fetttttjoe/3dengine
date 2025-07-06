#include "Scene/Grid.h"
#include "Shader.h"
#include "Core/ResourceManager.h"
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Grid::Grid(int size, int divisions) {
    name = "Grid";
    m_Shader = ResourceManager::LoadShader("grid_shader", "shaders/default.vert", "shaders/default.frag");

    // IMPROVEMENT: Set behavioral flags in the constructor. This makes the Grid's
    // special status (not selectable, rendered statically) a property of the object
    // itself, rather than requiring special checks elsewhere in the code.
    isSelectable = false;
    isStatic = true;

    std::vector<float> vertices;
    float step = (float)size / divisions;
    float halfSize = (float)size / 2.0f;
    for (int i = 0; i <= divisions; ++i) {
        float pos = -halfSize + i * step;
        // Lines along Z axis
        vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(pos);
        vertices.push_back(halfSize);  vertices.push_back(0.0f); vertices.push_back(pos);
        // Lines along X axis
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Grid::~Grid() {
    if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
}

void Grid::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_Shader) return;
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.3f, 0.3f, 0.3f, 1.0f);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, m_VertexCount);
    glBindVertexArray(0);
}

void Grid::DrawForPicking(Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) {
    // Grid is not pickable, this is now enforced by the isSelectable flag.
}

void Grid::DrawHighlight(const glm::mat4& view, const glm::mat4& projection) const {
    // Grid cannot be selected, so no highlight is needed.
}

std::string Grid::GetTypeString() const {
    return "Grid";
}

// The grid has no user-editable properties.
const std::vector<ObjectProperty>& Grid::GetProperties() const {
    return m_Properties;
}
