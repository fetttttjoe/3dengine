#include "BaseObject.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp> 

BaseObject::BaseObject() {
    // Generate GL buffers & arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // Default shader
    m_Shader = std::make_unique<Shader>(
        "shaders/default.vert", "shaders/default.frag"
    );
}

BaseObject::~BaseObject() {
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

glm::vec3 BaseObject::GetPosition() const {
    return glm::vec3(transform[3]);
}

void BaseObject::SetPosition(const glm::vec3& position) {
    transform[3] = glm::vec4(position, 1.0f);
}

glm::vec3 BaseObject::GetEulerAngles() const {
    glm::vec3 scale, skew, position;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(transform, scale, rotation, position, skew, perspective);
    return glm::degrees(glm::eulerAngles(rotation));
}

// --- EXISTING METHODS ---

void BaseObject::SetupMesh(const std::vector<float> &vertices,
                           const std::vector<unsigned int> &indices)
{
    m_IndexCount = static_cast<GLsizei>(indices.size());

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

      // position attribute
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                            3 * sizeof(float),
                            (void*)0);
      glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void BaseObject::RebuildMesh() {
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    BuildMeshData(verts, inds);
    SetupMesh(verts, inds);
}

void BaseObject::Draw(const glm::mat4 &view,
                      const glm::mat4 &projection)
{
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model",      transform);
    m_Shader->SetUniformMat4f("u_View",       view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color",
                           m_Color.x, m_Color.y,
                           m_Color.z, m_Color.w);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount,
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void BaseObject::DrawForPicking(Shader &pickingShader,
                                const glm::mat4 &view,
                                const glm::mat4 &projection)
{
    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_Model",      transform);
    pickingShader.SetUniformMat4f("u_View",       view);
    pickingShader.SetUniformMat4f("u_Projection", projection);
    pickingShader.SetUniform1ui("u_ObjectID",    id);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount,
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void BaseObject::DrawHighlight(const glm::mat4 &view,
                               const glm::mat4 &projection) const
{
    // Highlight shader is assumed bound externally
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount,
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

std::vector<ObjectProperty> BaseObject::GetProperties() {
    return {
        { "Width",  &m_Width  },
        { "Height", &m_Height },
        { "Depth",  &m_Depth  },
        { "Color R",&m_Color.r },
        { "Color G",&m_Color.g },
        { "Color B",&m_Color.b },
        { "Color A",&m_Color.a }
    };
}
