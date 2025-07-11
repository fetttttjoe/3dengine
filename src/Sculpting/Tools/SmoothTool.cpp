#include "Sculpting/Tools/SmoothTool.h"
#include <glm/gtx/norm.hpp>
#include "Sculpting/SculptableMesh.h"
#include "Core/UI/BrushSettings.h"

void SmoothTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint, const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                       const BrushSettings& settings, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                       int viewportWidth, int viewportHeight) {
    float brushRadiusSq = settings.radius * settings.radius;
    std::vector<glm::vec3> smoothedVertices = mesh.m_Vertices;
    bool verticesAffected = false;

    for (size_t i = 0; i < mesh.m_Vertices.size(); ++i) {
        glm::vec3& vertex = mesh.m_Vertices[i];
        float distSq = glm::distance2(hitPoint, vertex);

        if (distSq < brushRadiusSq) {
            verticesAffected = true;
            glm::vec3 centerOfMass(0.0f);
            int neighbors = 0;
            float falloff = settings.falloff.Evaluate(glm::sqrt(distSq) / settings.radius);

            // Find neighbors
            for (size_t j = 0; j < mesh.m_Vertices.size(); ++j) {
                if (i == j) continue;
                if (glm::distance2(vertex, mesh.m_Vertices[j]) < (settings.radius * 0.2f) * (settings.radius * 0.2f)) {
                    centerOfMass += mesh.m_Vertices[j];
                    neighbors++;
                }
            }

            if (neighbors > 0) {
                centerOfMass /= neighbors;
                smoothedVertices[i] = glm::mix(vertex, centerOfMass, settings.strength * 0.1f * falloff);
            }
        }
    }

    if (verticesAffected) {
        mesh.m_Vertices = smoothedVertices;
    }
}