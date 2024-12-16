#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

#include "app.hpp"

int main(int argc, char* argv[]) try {
    static constexpr std::string_view title = "Hello Rasterizer";

    rasterizer::Application app(title);

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
