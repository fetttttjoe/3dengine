#pragma once

#include "Interfaces.h"
#include <vector>
#include <memory>
#include <unordered_map> // <--- ADDED THIS INCLUDE
#include <glad/glad.h>
#include <glm/glm.hpp>

// Forward declarations
class Camera;
class Shader;
class ISceneObject;

// Enum to identify which axis a handle controls
enum class GizmoDimension {
    WIDTH, HEIGHT, DEPTH, NONE
};

// Represents a single draggable handle
struct GizmoHandle {
    uint32_t id;
    glm::vec3 basePosition; // Position relative to object's origin
    glm::vec4 color;
    GizmoDimension dimension;
    float direction; // +1 or -1 along the axis
};

// Manages all handles for a selected object
class TransformGizmo {
public:
    TransformGizmo();
    ~TransformGizmo();

    // Assigns the gizmo to a target object, creating handles for it
    void SetTarget(ISceneObject* target);
    ISceneObject* GetTarget() const { return m_Target; }

    // Returns a handle by its unique picking ID
    GizmoHandle* GetHandleByID(uint32_t id);

    // Updates the target object's dimension based on mouse movement
    void Update(const Camera& camera, const glm::vec2& mouseDelta, bool isDragging);

    // Renders the gizmo handles
    void Draw(const Camera& camera);

    // Renders the gizmo for picking
    void DrawForPicking(const Camera& camera, Shader& pickingShader);

    void SetActiveHandle(uint32_t id);
    GizmoHandle* GetActiveHandle() { return m_ActiveHandle; }
    
    // Checks if a given ID belongs to this gizmo
    static bool IsGizmoID(uint32_t id) { return id >= GIZMO_ID_START; }

private:
    void CreateHandles();
    void InitializeRendererObjects();

    ISceneObject* m_Target;
    std::vector<GizmoHandle> m_Handles;
    GizmoHandle* m_ActiveHandle;

    // Rendering resources
    std::unique_ptr<Shader> m_Shader;
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLsizei m_IndexCount = 0;

    // A map to get the float* for each dimension from the target
    std::unordered_map<GizmoDimension, float*> m_DimensionPointers;

    // A constant to offset gizmo IDs from object IDs
    static const uint32_t GIZMO_ID_START = 1000000;
};
