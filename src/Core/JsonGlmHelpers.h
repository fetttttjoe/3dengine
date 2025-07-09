#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

/**
 * @file JsonGlmHelpers.h
 * @brief Provides inline serialization/deserialization functions for GLM types
 * with nlohmann::json.
 *
 * Placing these as inline functions in a header prevents One Definition Rule
 * (ODR) violations and linker errors (LNK2005).
 */
namespace glm {

inline void to_json(nlohmann::json& j, const glm::vec3& v) {
  j = {v.x, v.y, v.z};
}

inline void from_json(const nlohmann::json& j, glm::vec3& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
  j.at(2).get_to(v.z);
}

inline void to_json(nlohmann::json& j, const glm::vec4& v) {
  j = {v.x, v.y, v.z, v.w};
}

inline void from_json(const nlohmann::json& j, glm::vec4& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
  j.at(2).get_to(v.z);
  j.at(3).get_to(v.w);
}

inline void to_json(nlohmann::json& j, const glm::quat& q) {
  j = {q.w, q.x, q.y, q.z};
}

inline void from_json(const nlohmann::json& j, glm::quat& q) {
  j.at(0).get_to(q.w);
  j.at(1).get_to(q.x);
  j.at(2).get_to(q.y);
  j.at(3).get_to(q.z);
}

}  // namespace glm
