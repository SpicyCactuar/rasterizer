#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <iostream>
#include <ostream>
#include <format>
#else
#include <print>
#include <ostream>
#endif

namespace rasterizer {
    template<typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args) {
#ifdef __EMSCRIPTEN__
        // Emscripten-specific behavior: Use std::cout (works in the browser console)
        std::cout << std::format(fmt, std::forward<Args>(args)...);
#else
        // Forward to std::print
        std::print(fmt, std::forward<Args>(args)...);
#endif
    }

    template<typename... Args>
    void print(std::ostream& out, std::format_string<Args...> fmt, Args&&... args) {
#ifdef __EMSCRIPTEN__
        // Emscripten-specific behavior: Use parameter out stream (works in the browser console)
        out << std::format(fmt, std::forward<Args>(args)...);
#else
        // Forward to std::print
        std::print(out, fmt, std::forward<Args>(args)...);
#endif
    }
}
