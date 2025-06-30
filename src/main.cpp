// =======================================================================
// File: src/main.cpp
// =======================================================================
#include "Core/Application.h"
#include <iostream>

int main() {
    try {
        auto app = std::make_unique<Application>();
        app->Run();
    } catch (const std::exception& e) {
        std::cerr << "An unrecoverable error occurred: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

