#pragma once

#include <string>

/**
 * @class IView
 * @brief The abstract base class for all UI panels (views).
 *
 * This interface ensures that any UI component can be treated uniformly by the
 * main UI host (AppUI).
 */
class IView {
 public:
  virtual ~IView() = default;

  /**
   * @brief Draws the UI for this view.
   */
  virtual void Draw() = 0;

  /**
   * @brief Gets the name of the view, used for identification.
   * @return The name of the view.
   */
  virtual const char* GetName() const = 0;
};
