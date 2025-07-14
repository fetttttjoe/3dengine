#pragma once
#include <string_view>

/**
 * @namespace ObjectTypes
 * @brief A centralized location for object type name constants.
 */
namespace ObjectTypes {
constexpr std::string_view Grid = "Grid";
constexpr std::string_view Pyramid = "Pyramid";
constexpr std::string_view Triangle = "Triangle";
constexpr std::string_view Sphere = "Sphere";
constexpr std::string_view Icosphere = "Icosphere";
constexpr std::string_view CustomMesh = "CustomMesh";
}  // namespace ObjectTypes