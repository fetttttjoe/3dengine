#pragma once
#include "imgui.h"
#include <functional> // For std::function

/**
 * A static helper class containing reusable functions for creating
 * standardized ImGui UI elements throughout the application.
 */
class UIElements {
public:
    /**
     * @brief Creates a standardized menu item that executes a given action.
     * @param label The text to display on the menu item.
     * @param action The function/lambda to call when the item is clicked.
     * @param enabled Whether the menu item should be clickable.
     */
    static void ActionMenuItem(const char* label, std::function<void()> action, bool enabled = true) {
        if (ImGui::MenuItem(label, nullptr, false, enabled)) {
            action();
        }
    }

    /**
     * @brief Creates a small, standardized button for use in lists or tight spaces.
     * @param label The text on the button.
     * @return True if the button was clicked this frame.
     */
    static bool SmallButton(const char* label) {
        // A standardized size for all small action buttons
        return ImGui::Button(label, ImVec2(70, 0));
    }
};