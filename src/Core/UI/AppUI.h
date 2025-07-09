#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Core/UI/IView.h"

// Forward declarations
class Application;
struct GLFWwindow;

class AppUI {
 public:
  explicit AppUI(Application* app);
  ~AppUI();

  void Initialize(GLFWwindow* window);
  void Shutdown();
  void BeginFrame();
  void EndFrame();

  template <typename T, typename... Args>
  void RegisterView(Args&&... args) {
    static_assert(std::is_base_of<IView, T>::value, "T must derive from IView");
    auto view = std::make_unique<T>(std::forward<Args>(args)...);
    m_ViewMap[typeid(T)] = view.get();
    m_Views.push_back(std::move(view));
  }

  template <typename T>
  T* GetView() {
    static_assert(std::is_base_of<IView, T>::value, "T must derive from IView");
    auto it = m_ViewMap.find(typeid(T));
    if (it != m_ViewMap.end()) {
      return static_cast<T*>(it->second);
    }
    return nullptr;
  }

  void Draw();

 private:
  void DrawSplitter(const char* id, float& valueToAdjust, bool invertDirection);

  Application* m_App;

  float m_LeftPaneWidth;
  float m_RightPaneWidth;

  std::vector<std::unique_ptr<IView>> m_Views;
  std::unordered_map<std::type_index, IView*> m_ViewMap;
};
