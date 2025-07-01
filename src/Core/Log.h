#pragma once
#include <iostream>
#include <string_view>

class Log {
public:
    // A variadic template to print any number of arguments
    template<typename... Args>
    static void Debug(Args&&... args) {
        // The code inside only exists if NDEBUG is NOT defined.
        // CMake automatically defines NDEBUG for Release builds.
#ifndef NDEBUG
        (std::cout << ... << args) << std::endl;
#endif
    }
};