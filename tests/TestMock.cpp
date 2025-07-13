#include "TestMocks.h" 

// --- Mock Camera functions DEFINITIONS ---
namespace MockCamera {
    // Define (initialize) the static variables here, ONCE.
    // REMOVE 'extern' keyword here, as this is the definition.
    glm::vec3 s_CameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::mat4 s_ViewMatrix = glm::lookAt(s_CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 s_ProjectionMatrix = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    // Define the functions. (These functions remain unchanged from previous versions)
    glm::vec3 GetPosition() { return s_CameraPosition; }

    glm::vec2 WorldToScreen(const glm::vec3& worldPos,
                            const glm::mat4& viewProj, int windowWidth,
                            int windowHeight) {
        glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);
        glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
        float screenX = (ndcPos.x + 1.0f) / 2.0f * windowWidth;
        float screenY = (1.0f - ndcPos.y) / 2.0f * windowHeight;
        return glm::vec2(screenX, screenY);
    }

    glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                                 const glm::mat4& invViewProj,
                                 int windowWidth, int windowHeight) {
        float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
        float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f;
        glm::vec4 worldPos = invViewProj * glm::vec4(ndcX, ndcY, ndcZ, 1.0f);
        if (worldPos.w == 0.0f) return glm::vec3(0.0f);
        return glm::vec3(worldPos) / worldPos.w;
    }
    
    glm::vec3 ScreenToWorldRay(const glm::vec2& screenPos, int windowWidth, int windowHeight) {
        return glm::normalize(glm::vec3(0,0,-1));
    }
} // namespace MockCamera