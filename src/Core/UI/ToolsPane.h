#pragma once

#include <functional>

class ToolsPane {
 public:
  ToolsPane();
  void Draw(bool& showMetricsWindow);  // showMetricsWindow is passed by
                                       // reference to allow modification

  // Callbacks for actions in the tools pane
  std::function<void()> OnResetCamera;
};