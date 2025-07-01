
// Scene/Objects/ScaleHandle.cpp
#include "ScaleHandle.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>

// Build a unit circle in the XY plane
static std::vector<float> BuildCircleVerts() {
    const int segments = 32;
    std::vector<float> verts;
    verts.reserve((segments+1)*3);
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / segments * 2.0f * 3.14159265358979323846f;
        verts.push_back(std::cos(a));
        verts.push_back(std::sin(a));
        verts.push_back(0.0f);
    }
    return verts;
}

// Lazy init shared shader for handles
static Shader* getHandleShader() {
    static std::unique_ptr<Shader> s_Shader;
    if (!s_Shader) {
        s_Shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    }
    return s_Shader.get();
}

ScaleHandle::ScaleHandle(uint32_t parentID, Axis which)
  : m_ParentID(parentID)
  , m_Axis(which)
  , m_VAO(0)
  , m_VBO(0)
  , m_Shader(nullptr)
{
    // Build circle mesh
    auto verts = BuildCircleVerts();
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(float),
                 verts.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    m_Shader = getHandleShader();
}

ScaleHandle::~ScaleHandle() {
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void ScaleHandle::UpdateTransform(const glm::mat4& parentModel,
                                  float halfW,
                                  float halfH,
                                  float halfD)
{
    glm::vec3 off;
    switch (m_Axis) {
        case X_POS: off = { halfW, 0.0f, 0.0f}; break;
        case X_NEG: off = {-halfW, 0.0f, 0.0f}; break;
        case Y_POS: off = {0.0f,  halfH,  0.0f}; break;
        case Y_NEG: off = {0.0f, -halfH,  0.0f}; break;
        case Z_POS: off = {0.0f,  0.0f,  halfD}; break;
        case Z_NEG: off = {0.0f,  0.0f, -halfD}; break;
    }
    m_Model = glm::translate(parentModel, off)
            * glm::scale(glm::mat4(1.0f), glm::vec3(g_CircleSize));
}

void ScaleHandle::Draw(const glm::mat4& view,
                       const glm::mat4& projection)
{
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_Model",      m_Model);
    m_Shader->SetUniformMat4f("u_View",       view);
    m_Shader->SetUniformMat4f("u_Projection", projection);
    m_Shader->SetUniform4f("u_Color", 0.9f, 0.9f, 0.1f, 1.0f);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINE_STRIP, 0, 33);
    glBindVertexArray(0);
}

void ScaleHandle::DrawForPicking(Shader& pickingShader,
                                 const glm::mat4& view,
                                 const glm::mat4& projection)
{
    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_Model",      m_Model);
    pickingShader.SetUniformMat4f("u_View",       view);
    pickingShader.SetUniformMat4f("u_Projection", projection);
    uint32_t pickID = (m_ParentID << 3) | uint32_t(m_Axis);
    pickingShader.SetUniform1ui("u_ObjectID", pickID);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINE_STRIP, 0, 33);
    glBindVertexArray(0);
}
