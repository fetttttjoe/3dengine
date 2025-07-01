// Core/UI/UIElements.h
#pragma once

#include "imgui.h"
#include <functional>

/**
 * A static helper class containing reusable functions for standardized ImGui elements.
 */
class UIElements {
public:
    static void ActionMenuItem(const char* label, std::function<void()> action, bool enabled = true) {
        if (ImGui::MenuItem(label, nullptr, false, enabled)) action();
    }

    static bool SmallButton(const char* label) {
        return ImGui::Button(label, ImVec2(70,0));
    }
};
