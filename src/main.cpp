#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

#include "scene.hpp"
#include "app.hpp"

int main(int argc, char* argv[]) try {
    static constexpr std::string_view title = "Hello Rasterizer";

    rasterizer::Application app(title);
    rasterizer::Scene scene;

    constexpr std::uint32_t rectangleWidth = 1200, rectangleHeight = 750;
    constexpr std::uint32_t positionX = 720, positionY = 450;
    scene.emplaceRectangle(positionX, positionY, rectangleWidth, rectangleHeight);

    while (app.isRunning) {
        app.processInput();
        app.update();
        app.render(scene);
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::fprintf(stderr, "Exiting due to: %s\n", e.what());
    return EXIT_FAILURE;
}
