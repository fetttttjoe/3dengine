#pragma once

#include <iostream> // For std::ostream
#include <glm/glm.hpp> // For glm::vec3

// Overload for glm::vec3
inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    return os << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

// Optional: Overload for glm::vec4 if you log them
inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
    return os << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

// Add other GLM types as needed (e.g., glm::mat4)