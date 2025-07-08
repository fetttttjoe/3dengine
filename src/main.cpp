#include "Core/Application.h"
#include "Core/Log.h"

int main() {
  try {
    auto app = std::make_unique<Application>(1920, 1080);
    app->Run();
  } catch (const std::exception& e) {
    Log::Debug("An unrecoverable error occurred: ", e.what());
    return -1;
  }
  return 0;
}