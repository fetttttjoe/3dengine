// src/Scene/TransformGizmo.cpp
#include "TransformGizmo.h"
#include "Core/Camera.h"
#include "Shader.h"
#include "Interfaces.h" // For ISceneObject and ObjectProperty
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Log.h" // For Log::Debug

// --- Shader Source Code ---
// We define the shaders here as string literals to avoid needing separate files.

const char* GIZMO_VERTEX_SHADER = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}
)glsl";

const char* GIZMO_FRAGMENT_SHADER = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec4 u_Color;
uniform uint u_ObjectID;

void main()
{
    // This variable helps ensure both u_ObjectID and u_Color are "used" to prevent optimization
    // if only one path is taken, while not affecting the final output.
    uint active_id = u_ObjectID; 
    vec4 active_color = u_Color;

    if (active_id > 0u) { // Use active_id here
        // Encode ID into color for picking
        // FIX: Add 'u' suffix to 0xFF to make it an unsigned integer literal.
        float r = float((active_id >> 0) & 0xFFu) / 255.0; 
        float g = float((active_id >> 8) & 0xFFu) / 255.0; 
        float b = float((active_id >> 16) & 0xFFu) / 255.0; 
        FragColor = vec4(r, g, b, 1.0);
    } else {
        FragColor = active_color; // Use active_color here
    }
}
)glsl";


TransformGizmo::TransformGizmo() : m_Target(nullptr), m_ActiveHandle(nullptr) {
    m_Shader = std::make_unique<Shader>(GIZMO_VERTEX_SHADER, GIZMO_FRAGMENT_SHADER, true);
    InitializeRendererObjects();
    Log::Debug("TransformGizmo spawned.");
}

TransformGizmo::~TransformGizmo() {
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void TransformGizmo::InitializeRendererObjects() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    m_IndexCount = 6;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    GLuint EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &EBO);
}


void TransformGizmo::SetTarget(ISceneObject* target) {
    m_Target = target;
    m_Handles.clear();
    m_DimensionPointers.clear();
    m_ActiveHandle = nullptr;

    if (m_Target) {
        auto props = m_Target->GetProperties();
        Log::Debug("Setting target ", m_Target->name, ". Found ", props.size(), " properties.");
        for(const auto& prop : props) {
            if (prop.name == "Width") { m_DimensionPointers[GizmoDimension::WIDTH] = prop.value; Log::Debug("Registered Width pointer."); }
            if (prop.name == "Height") { m_DimensionPointers[GizmoDimension::HEIGHT] = prop.value; Log::Debug("Registered Height pointer."); }
            if (prop.name == "Depth") { m_DimensionPointers[GizmoDimension::DEPTH] = prop.value; Log::Debug("Registered Depth pointer."); }
        }
        CreateHandles();
    }
}

void TransformGizmo::CreateHandles() {
    uint32_t currentId = GIZMO_ID_START;
    
    // Width Handles (X-axis, Red)
    if (m_DimensionPointers.count(GizmoDimension::WIDTH)) {
        m_Handles.push_back({currentId++, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}, GizmoDimension::WIDTH, 1.0f});
        m_Handles.push_back({currentId++, {-1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}, GizmoDimension::WIDTH, -1.0f});
    }
    // Height Handles (Y-axis, Green)
    if (m_DimensionPointers.count(GizmoDimension::HEIGHT)) {
        m_Handles.push_back({currentId++, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}, GizmoDimension::HEIGHT, 1.0f});
        m_Handles.push_back({currentId++, {0.0f, -1.0f, 0.0f}, {0, 1, 0, 1}, GizmoDimension::HEIGHT, -1.0f});
    }
    // Depth Handles (Z-axis, Blue)
    if (m_DimensionPointers.count(GizmoDimension::DEPTH)) {
        m_Handles.push_back({currentId++, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}, GizmoDimension::DEPTH, 1.0f});
        m_Handles.push_back({currentId++, {0.0f, 0.0f, -1.0f}, {0, 0, 1, 1}, GizmoDimension::DEPTH, -1.0f});
    }
}

GizmoHandle* TransformGizmo::GetHandleByID(uint32_t id) {
    for (auto& handle : m_Handles) {
        if (handle.id == id) {
            return &handle;
        }
    }
    return nullptr;
}

void TransformGizmo::SetActiveHandle(uint32_t id) {
    m_ActiveHandle = GetHandleByID(id);
}

void TransformGizmo::Update(const Camera& camera, const glm::vec2& mouseDelta, bool isDragging) {
    if (!m_Target || !m_ActiveHandle || !isDragging) {
        return;
    }

    glm::vec3 handleAxisLocal = {
        m_ActiveHandle->dimension == GizmoDimension::WIDTH ? 1.0f : 0.0f,
        m_ActiveHandle->dimension == GizmoDimension::HEIGHT ? 1.0f : 0.0f,
        m_ActiveHandle->dimension == GizmoDimension::DEPTH ? 1.0f : 0.0f,
    };
    
    glm::vec3 objectEulerAngles = m_Target->GetEulerAngles(); 
    glm::mat4 objectRotationMatrix = glm::mat4_cast(glm::quat(glm::radians(objectEulerAngles)));

    glm::vec3 handleAxisWorld = glm::normalize(glm::vec3(objectRotationMatrix * glm::vec4(handleAxisLocal, 0.0f)));

    glm::vec3 screenAxis = camera.WorldToScreenDirection(handleAxisWorld);

    if (glm::length(screenAxis) < 0.001f) return;
    screenAxis = glm::normalize(screenAxis);

    float dot_product = glm::dot(glm::vec2(screenAxis), mouseDelta);

    float sensitivity = 0.01f; 
    float change = dot_product * sensitivity;

    float* dimension = m_DimensionPointers[m_ActiveHandle->dimension];
    if (dimension) {
        *dimension += change * m_ActiveHandle->direction;
        if (*dimension < 0.05f) {
            *dimension = 0.05f;
        }
        Log::Debug("Updating dimension: ", (int)m_ActiveHandle->dimension, " by change: ", change, " new value: ", *dimension);
        m_Target->RebuildMesh();
    } else {
        Log::Debug("ERROR: No dimension pointer found for handle dimension: ", (int)m_ActiveHandle->dimension);
    }
}


void TransformGizmo::Draw(const Camera& camera) {
    if (!m_Target || m_Handles.empty()) return;

    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_Shader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    m_Shader->SetUniform1ui("u_ObjectID", 0); // 0 for normal drawing
    m_Shader->SetUniform4f("u_Color", 0,0,0,0); // Dummy set to make it active, actual color set below.

    float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
    float scale = distance * 0.01f;

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_VAO);

    for (const auto& handle : m_Handles) {
        m_Shader->SetUniform4f("u_Color", handle.color.r, handle.color.g, handle.color.b, handle.color.a);
        
        float w = *m_DimensionPointers[GizmoDimension::WIDTH] * 0.5f;
        float h = *m_DimensionPointers[GizmoDimension::HEIGHT] * 0.5f;
        float d = *m_DimensionPointers[GizmoDimension::DEPTH] * 0.5f;

        glm::vec3 handlePosLocal = {
            handle.basePosition.x * w,
            handle.basePosition.y * h,
            handle.basePosition.z * d
        };

        glm::mat4 targetModelMatrix = m_Target->transform; 
        glm::vec3 handleWorldPosition = glm::vec3(targetModelMatrix * glm::vec4(handlePosLocal, 1.0f));

        glm::mat4 handleModelMatrix = glm::translate(glm::mat4(1.0f), handleWorldPosition);
        handleModelMatrix = handleModelMatrix * glm::mat4(glm::mat3(glm::inverse(camera.GetViewMatrix())));
        handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(scale));

        m_Shader->SetUniformMat4f("u_Model", handleModelMatrix);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void TransformGizmo::DrawForPicking(const Camera& camera, Shader& pickingShader) {
    if (!m_Target || m_Handles.empty()) return;

    pickingShader.Bind();
    pickingShader.SetUniformMat4f("u_View", camera.GetViewMatrix());
    pickingShader.SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    pickingShader.SetUniform1ui("u_ObjectID", 0); // Dummy set to make it active.

    float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
    float scale = distance * 0.01f;

    glBindVertexArray(m_VAO);

    for (const auto& handle : m_Handles) {
        pickingShader.SetUniform1ui("u_ObjectID", handle.id);

        float w = *m_DimensionPointers[GizmoDimension::WIDTH] * 0.5f;
        float h = *m_DimensionPointers[GizmoDimension::HEIGHT] * 0.5f;
        float d = *m_DimensionPointers[GizmoDimension::DEPTH] * 0.5f;

        glm::vec3 handlePosLocal = {
            handle.basePosition.x * w,
            handle.basePosition.y * h,
            handle.basePosition.z * d
        };

        glm::mat4 targetModelMatrix = m_Target->transform; 
        glm::vec3 handleWorldPosition = glm::vec3(targetModelMatrix * glm::vec4(handlePosLocal, 1.0f));

        glm::mat4 handleModelMatrix = glm::translate(glm::mat4(1.0f), handleWorldPosition);
        handleModelMatrix = handleModelMatrix * glm::mat4(glm::mat3(glm::inverse(camera.GetViewMatrix())));
        handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(scale));

        pickingShader.SetUniformMat4f("u_Model", handleModelMatrix);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}