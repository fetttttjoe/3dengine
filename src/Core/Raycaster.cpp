#include "Core/Raycaster.h"
#include "Sculpting/SculptableMesh.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace Raycaster {

// Internal helper for Möller–Trumbore ray-triangle intersection
bool IntersectTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                       const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                       float& outDistance) {
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDirection, edge2);
    float a = glm::dot(edge1, h);

    if (a > -1e-6 && a < 1e-6)
        return false; // Ray is parallel to the triangle.

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDirection, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * glm::dot(edge2, q);
    if (t > 1e-6) {
        outDistance = t;
        return true;
    }

    return false;
}

bool IntersectMesh(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                   const SculptableMesh& mesh, const glm::mat4& modelMatrix,
                   RaycastResult& outResult) {
    
    glm::mat4 invModelMatrix = glm::inverse(modelMatrix);
    glm::vec3 rayOriginLocal = glm::vec3(invModelMatrix * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 rayDirectionLocal = glm::normalize(glm::vec3(invModelMatrix * glm::vec4(rayDirection, 0.0f)));

    bool foundHit = false;
    outResult.distance = std::numeric_limits<float>::max();

    const auto& vertices = mesh.GetVertices();
    const auto& indices = mesh.GetIndices();

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        const glm::vec3& v0 = vertices[indices[i]];
        const glm::vec3& v1 = vertices[indices[i + 1]];
        const glm::vec3& v2 = vertices[indices[i + 2]];
        
        float t = 0.0f;
        if (IntersectTriangle(rayOriginLocal, rayDirectionLocal, v0, v1, v2, t)) {
            if (t < outResult.distance) {
                foundHit = true;
                outResult.distance = t;
                outResult.hitPoint = rayOriginLocal + rayDirectionLocal * t;
            }
        }
    }

    outResult.hit = foundHit;
    return foundHit;
}

} // namespace Raycaster