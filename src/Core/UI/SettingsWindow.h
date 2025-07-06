#pragma once

class SettingsWindow {
 public:
  SettingsWindow();
  void Draw(bool& showWindow);  // Pass showWindow by reference to allow closing

 private:
  // No direct members needed for now, as it accesses SettingsManager directly
};