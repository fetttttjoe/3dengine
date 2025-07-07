// UIElements.h
#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "imgui.h"

/**
 * A static helper class containing reusable functions for standardized ImGui
 * elements and a full-screen host.
 */
class UIElements {
 public:
  // — Menu helpers
  static void ActionMenuItem(const char* label, std::function<void()> action,
                             bool enabled = true) {
    if (ImGui::MenuItem(label, nullptr, false, enabled)) action();
  }

  static bool SmallButton(const char* label) {
    return ImGui::Button(label, ImVec2(70, 0));
  }

  static bool Button(const char* label, bool enabled = true) {
    if (!enabled) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      ImGui::BeginDisabled();
    }
    bool clicked = ImGui::Button(label);
    if (!enabled) {
      ImGui::EndDisabled();
      ImGui::PopStyleVar();
    }
    return clicked;
  }

  // — Full-screen container
  static void BeginFullScreen(const char* id) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin(id, nullptr, flags);
    ImGui::PopStyleVar();
  }

  static void EndFullScreen() { ImGui::End(); }

  // — Toolbar inside a child region
  static void Toolbar(const std::function<void()>& content,
                      float height = 28.0f) {
    ImGui::BeginChild("##ToolbarFS", ImVec2(0, height), false,
                      ImGuiWindowFlags_NoScrollbar);
    content();
    ImGui::EndChild();
  }

  // — Viewport image (flipped Y so it appears right-side up)
  //    also reports its bounds, focus, and hover state
  static ImVec2 ViewportImage(uint32_t textureId, std::array<ImVec2, 2>& bounds,
                              bool& focused, bool& hovered) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)(intptr_t)textureId, avail, ImVec2(0, 1),
                 ImVec2(1, 0));
    ImVec2 p0 = ImGui::GetItemRectMin();
    bounds[0] = p0;
    bounds[1] = {p0.x + avail.x, p0.y + avail.y};
    focused = ImGui::IsWindowFocused();
    hovered = ImGui::IsWindowHovered();
    return avail;
  }

  /**
   * @brief Vertical splitter you can drag to resize two adjacent panes.
   * @param id         Unique ID
   * @param sizeA      (in/out) width of the first (left) pane
   * @param sizeB      (in/out) width of the second (right) pane
   * @param minSize    Minimum width for either pane
   * @param thickness  Thickness of the draggable area
   */
  static void Splitter(const char* id, float& sizeA, float& sizeB,
                       float minSize = 50.0f, float thickness = 4.0f) {
    ImGui::PushID(id);
    ImGui::InvisibleButton("##splitter", ImVec2(thickness, -1),
                           ImGuiButtonFlags_MouseButtonLeft);
    if (ImGui::IsItemHovered())
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    if (ImGui::IsItemActive()) {
      float delta = ImGui::GetIO().MouseDelta.x;
      sizeA += delta;
      sizeB -= delta;
      if (sizeA < minSize) {
        sizeB += sizeA - minSize;
        sizeA = minSize;
      }
      if (sizeB < minSize) {
        sizeA += sizeB - minSize;
        sizeB = minSize;
      }
    }

    ImGui::PopID();

    // draw a line down the middle of the splitter region
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 a = ImGui::GetItemRectMin();
    ImVec2 b = ImGui::GetItemRectMax();
    float x = (a.x + b.x) * 0.5f;
    dl->AddLine({x, a.y}, {x, b.y}, ImGui::GetColorU32(ImGuiCol_Separator),
                thickness);
  }
};
