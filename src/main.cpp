#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdint>
#include <iostream>

#include "app.hpp"

int main(int argc, char* argv[]) try {
    static constexpr std::string_view title = "Hello Rasterizer";
    static constexpr std::uint32_t windowWidth = 1200;
    static constexpr std::uint32_t windowHeight = 675;

    rasterizer::Application app(title, windowWidth, windowHeight);

    while (app.isRunning) {
        app.processInput();
        app.update();
        app.render();
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::fprintf(stderr, "Exiting due to: %s\n", e.what());
    return EXIT_FAILURE;
}
