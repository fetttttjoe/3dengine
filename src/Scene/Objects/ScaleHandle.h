// Scene/Objects/ScaleHandle.h
#pragma once

#include "Interfaces.h"   // for ISceneObject, ObjectProperty
#include <cstdint>        // for uint32_t, uint8_t
#include <vector>
#include <string>
#include <glm/glm.hpp>    // for glm::mat4, glm::vec3
#include <glad/glad.h>    // for GLuint

// Forward‐declare Shader so we can use it in the method signatures.
class Shader;

/**
 * A small circular handle used to scale BaseObject-derived meshes.
 * Implements ISceneObject so it can be drawn and picked like any object.
 */
class ScaleHandle : public ISceneObject {
public:
    // Which face/axis this handle sits on
    enum Axis : uint8_t {
        X_POS = 0,
        X_NEG,
        Y_POS,
        Y_NEG,
        Z_POS,
        Z_NEG
    };

    // parentID is the ISceneObject::id of the object this handle belongs to
    ScaleHandle(uint32_t parentID, Axis which);
    ~ScaleHandle() override;

    // ISceneObject impl:
    void Draw(const glm::mat4& view,
              const glm::mat4& projection) override;
    void DrawForPicking(Shader& pickingShader,
                        const glm::mat4& view,
                        const glm::mat4& projection) override;
    void DrawHighlight(const glm::mat4&, const glm::mat4&) const override {}
    std::string GetTypeString() const override { return "ScaleHandle"; }
    std::vector<ObjectProperty> GetProperties() override { return {}; }
    void RebuildMesh() override {}

    // Update where the handle should sit, given the parent’s world transform
    // and half-dimensions of its bounding box.
    void UpdateTransform(const glm::mat4& parentModel,
                         float halfW,
                         float halfH,
                         float halfD);

    // Accessors for picking logic
    uint32_t ParentID() const     { return m_ParentID; }
    Axis     HandleAxis() const   { return m_Axis; }

private:
    uint32_t    m_ParentID;
    Axis        m_Axis;

    GLuint      m_VAO;
    GLuint      m_VBO;
    glm::mat4   m_Model;
    Shader*     m_Shader;

    // Circle radius in the handle’s local space
    static constexpr float g_CircleSize = 0.05f;
};
