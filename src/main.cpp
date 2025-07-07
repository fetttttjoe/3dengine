#include <iostream>

#include "Core/Application.h"

int main() {
  try {
    // To use the default size (1280x720), you would use:
    // auto app = std::make_unique<Application>();
    auto app = std::make_unique<Application>(1920, 1080);

    app->Run();
  } catch (const std::exception& e) {
    std::cerr << "An unrecoverable error occurred: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}