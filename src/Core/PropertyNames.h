#pragma once

/**
 * @namespace PropertyNames
 * @brief A centralized location for type-safe property name constants.
 *
 * Using these constants instead of raw string literals prevents typos and
 * allows for easier refactoring and code navigation.
 */
namespace PropertyNames {
// Transform Properties
inline constexpr const char* Position = "Position";
inline constexpr const char* Rotation = "Rotation";
inline constexpr const char* Scale = "Scale";

// Common Mesh Properties
inline constexpr const char* Width = "Width";
inline constexpr const char* Height = "Height";
inline constexpr const char* Depth = "Depth";
inline constexpr const char* Radius = "Radius";

// Material / Appearance
inline constexpr const char* Color = "Color";

}  // namespace PropertyNames