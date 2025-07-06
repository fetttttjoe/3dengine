#include "BaseObject.h"
#include "Core/ResourceManager.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

BaseObject::BaseObject()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // Get the default shader from the resource manager
    m_Shader = ResourceManager::LoadShader("default", "shaders/default.vert", "shaders/default.frag");

    // Register common properties that all base objects will have
    AddProperty("Color", &m_Color, PropertyType::Color_Vec4);
    AddProperty("Width", &m_Width, PropertyType::Float);
    AddProperty("Height", &m_Height, PropertyType::Float);
    AddProperty("Depth", &m_Depth, PropertyType::Float);
}

BaseObject::~BaseObject()
{
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void BaseObject::AddProperty(const std::string& name, void* value_ptr, PropertyType type) {
    m_Properties.push_back({name, value_ptr, type});
}

const std::vector<ObjectProperty>& BaseObject::GetProperties() const {
    return m_Properties;
}

void BaseObject::SetupMesh(const std::vector<float> &vertices,
                           const std::vector<unsigned int> &indices)
{
    m_IndexCount = static_cast<GLsizei>(indices.size());
    if (m_IndexCount == 0) return;

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void BaseObject::RebuildMesh()
{
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    BuildMeshData(verts, inds);
    SetupMesh(verts, inds);
}

void BaseObject::Draw(const glm::mat4 &view,
                      const glm::mat4 &projection)
{
    if (!m_Shader) return;
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model", transform);
    m_Shader->SetUniformMat4f("u_View", view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniformVec4("u_Color", m_Color);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void BaseObject::DrawForPicking(Shader &pickingShader,
                                const glm::mat4 &view,
                                const glm::mat4 &projection)
{
    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_Model", transform);
    pickingShader.SetUniformMat4f("u_View", view);
    pickingShader.SetUniformMat4f("u_Projection", projection);
    pickingShader.SetUniform1ui("u_ObjectID", id);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void BaseObject::DrawHighlight(const glm::mat4 &view,
                               const glm::mat4 &projection) const
{
    // A highlight shader is assumed to be bound by the renderer
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}