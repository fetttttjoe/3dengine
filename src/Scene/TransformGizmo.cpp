#include "TransformGizmo.h"
#include "Core/Camera.h"
#include "Shader.h"
#include "Interfaces.h"
#include "Core/Log.h"
#include "Core/ResourceManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

// We define the shaders here as string literals to avoid needing separate files.
const char* GIZMO_VERTEX_SHADER_SRC = R"glsl(
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

const char* GIZMO_FRAGMENT_SHADER_SRC = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}
)glsl";


TransformGizmo::TransformGizmo() : m_Target(nullptr), m_ActiveHandle(nullptr) {
    m_Shader = ResourceManager::LoadShaderFromMemory("gizmo_shader", GIZMO_VERTEX_SHADER_SRC, GIZMO_FRAGMENT_SHADER_SRC);
    InitializeRendererObjects();
    Log::Debug("TransformGizmo spawned.");
}

TransformGizmo::~TransformGizmo() {
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void TransformGizmo::InitializeRendererObjects() {
    // A simple quad that will be billboarded to face the camera
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    m_IndexCount = 6;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    // EBO can be deleted after setup as VAO retains the binding
    glDeleteBuffers(1, &ebo);
}

void TransformGizmo::SetTarget(ISceneObject* target) {
    m_Target = target;
    m_Handles.clear();
    m_DimensionPointers.clear();
    m_ActiveHandle = nullptr;

    if (m_Target) {
        // Find the dimension properties from the target object
        const auto& props = m_Target->GetProperties();
        for(const auto& prop : props) {
            if (prop.name == "Width")  { m_DimensionPointers[GizmoDimension::WIDTH]  = static_cast<float*>(prop.value_ptr); }
            if (prop.name == "Height") { m_DimensionPointers[GizmoDimension::HEIGHT] = static_cast<float*>(prop.value_ptr); }
            if (prop.name == "Depth")  { m_DimensionPointers[GizmoDimension::DEPTH]  = static_cast<float*>(prop.value_ptr); }
        }
        CreateHandles();
    }
}

void TransformGizmo::CreateHandles() {
    uint32_t currentId = GIZMO_ID_START;
    
    // BUG FIX: Only create handles for dimensions that were successfully found.
    // This makes the gizmo correctly support objects that might not have all three dimensions (like a 2D triangle).
    if (m_DimensionPointers.count(GizmoDimension::WIDTH)) {
        m_Handles.push_back({currentId++, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}, GizmoDimension::WIDTH, 1.0f});
        m_Handles.push_back({currentId++, {-1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}, GizmoDimension::WIDTH, -1.0f});
    }
    if (m_DimensionPointers.count(GizmoDimension::HEIGHT)) {
        m_Handles.push_back({currentId++, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}, GizmoDimension::HEIGHT, 1.0f});
        m_Handles.push_back({currentId++, {0.0f, -1.0f, 0.0f}, {0, 1, 0, 1}, GizmoDimension::HEIGHT, -1.0f});
    }
    // A triangle has 0 depth, so a depth handle is not useful. We also check if the pointer exists.
    if (m_DimensionPointers.count(GizmoDimension::DEPTH) && *m_DimensionPointers[GizmoDimension::DEPTH] > 0.0f) {
        m_Handles.push_back({currentId++, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}, GizmoDimension::DEPTH, 1.0f});
        m_Handles.push_back({currentId++, {0.0f, 0.0f, -1.0f}, {0, 0, 1, 1}, GizmoDimension::DEPTH, -1.0f});
    }
}

GizmoHandle* TransformGizmo::GetHandleByID(uint32_t id) {
    for (auto& handle : m_Handles) {
        if (handle.id == id) return &handle;
    }
    return nullptr;
}

void TransformGizmo::SetActiveHandle(uint32_t id) {
    m_ActiveHandle = GetHandleByID(id);
}

void TransformGizmo::Update(const Camera& camera, const glm::vec2& mouseDelta, bool isDragging, int winWidth, int winHeight) {
    if (!m_Target || !m_ActiveHandle || !isDragging) {
        return;
    }

    // 1. Define the local axis for the active handle (e.g., X-axis is {1,0,0})
    glm::vec3 handleAxisLocal = {
        m_ActiveHandle->dimension == GizmoDimension::WIDTH ? 1.0f : 0.0f,
        m_ActiveHandle->dimension == GizmoDimension::HEIGHT ? 1.0f : 0.0f,
        m_ActiveHandle->dimension == GizmoDimension::DEPTH ? 1.0f : 0.0f,
    };
    
    // 2. Transform this local axis into world space using the object's rotation
    glm::mat4 objectRotationMatrix = glm::mat4_cast(glm::quat(glm::radians(m_Target->GetEulerAngles())));
    glm::vec3 axisWorldDir = glm::normalize(glm::vec3(objectRotationMatrix * glm::vec4(handleAxisLocal, 0.0f)));

    // 3. Project the 3D world axis onto the 2D screen
    glm::vec3 objectWorldPos = m_Target->GetPosition();
    glm::vec2 screenPosStart = camera.WorldToScreen(objectWorldPos, winWidth, winHeight);
    glm::vec2 screenPosEnd   = camera.WorldToScreen(objectWorldPos + axisWorldDir, winWidth, winHeight);
    
    glm::vec2 screenAxis = screenPosEnd - screenPosStart;

    // Avoid division by zero if the axis is pointing directly at the camera
    if (glm::length(screenAxis) < 0.001f) return;
    screenAxis = glm::normalize(screenAxis);

    // 4. Project the mouse movement onto the screen-space axis
    float dot_product = glm::dot(mouseDelta, screenAxis);
    float sensitivity = 0.01f; 
    float change = dot_product * sensitivity;

    // 5. Apply the calculated change to the object's dimension property
    float* dimension = m_DimensionPointers[m_ActiveHandle->dimension];
    if (dimension) {
        *dimension += change * m_ActiveHandle->direction;
        // Clamp to a minimum size to prevent inversion
        if (*dimension < 0.05f) {
            *dimension = 0.05f;
        }
        m_Target->RebuildMesh();
    }
}


void TransformGizmo::Draw(const Camera& camera) {
    if (!m_Target || m_Handles.empty()) return;

    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_View", camera.GetViewMatrix());
    m_Shader->SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());

    // Scale handles based on distance to camera to maintain a constant screen size
    float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
    
    // FIX: Reduced the scaling factor to make the gizmo handles smaller and less obtrusive.
    float scale = distance * 0.02f; 

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_VAO);

    for (const auto& handle : m_Handles) {
        m_Shader->SetUniformVec4("u_Color", handle.color);
        
        // Calculate handle world position based on object's transform and dimensions
        float w = m_DimensionPointers.count(GizmoDimension::WIDTH) ? *m_DimensionPointers.at(GizmoDimension::WIDTH) * 0.5f : 0.f;
        float h = m_DimensionPointers.count(GizmoDimension::HEIGHT) ? *m_DimensionPointers.at(GizmoDimension::HEIGHT) * 0.5f : 0.f;
        float d = m_DimensionPointers.count(GizmoDimension::DEPTH) ? *m_DimensionPointers.at(GizmoDimension::DEPTH) * 0.5f : 0.f;

        glm::vec3 handlePosLocal = {
            handle.basePosition.x * w,
            handle.basePosition.y * h,
            handle.basePosition.z * d
        };

        glm::vec3 handleWorldPosition = glm::vec3(m_Target->transform * glm::vec4(handlePosLocal, 1.0f));

        // Create model matrix for the handle quad (translate, billboard, scale)
        glm::mat4 handleModelMatrix = glm::translate(glm::mat4(1.0f), handleWorldPosition);
        // Billboard: orient handle to face the camera
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

    float distance = glm::length(camera.GetPosition() - m_Target->GetPosition());
    
    // FIX: Reduced the scaling factor to match the visual size in the Draw() method.
    // This ensures the clickable area is consistent with what the user sees.
    float scale = distance * 0.02f; 

    glBindVertexArray(m_VAO);

    for (const auto& handle : m_Handles) {
        pickingShader.SetUniform1ui("u_ObjectID", handle.id);

        float w = m_DimensionPointers.count(GizmoDimension::WIDTH) ? *m_DimensionPointers.at(GizmoDimension::WIDTH) * 0.5f : 0.f;
        float h = m_DimensionPointers.count(GizmoDimension::HEIGHT) ? *m_DimensionPointers.at(GizmoDimension::HEIGHT) * 0.5f : 0.f;
        float d = m_DimensionPointers.count(GizmoDimension::DEPTH) ? *m_DimensionPointers.at(GizmoDimension::DEPTH) * 0.5f : 0.f;

        glm::vec3 handlePosLocal = {
            handle.basePosition.x * w,
            handle.basePosition.y * h,
            handle.basePosition.z * d
        };

        glm::vec3 handleWorldPosition = glm::vec3(m_Target->transform * glm::vec4(handlePosLocal, 1.0f));

        glm::mat4 handleModelMatrix = glm::translate(glm::mat4(1.0f), handleWorldPosition);
        handleModelMatrix = handleModelMatrix * glm::mat4(glm::mat3(glm::inverse(camera.GetViewMatrix())));
        handleModelMatrix = glm::scale(handleModelMatrix, glm::vec3(scale));

        pickingShader.SetUniformMat4f("u_Model", handleModelMatrix);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}