#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

#include "scene.hpp"
#include "app.hpp"

int main(int argc, char* argv[]) try {
    static constexpr std::string_view title = "Hello Rasterizer";
    static constexpr std::uint32_t FPS = 120;
    static constexpr std::uint32_t FRAME_TIME = 1000 / FPS;

    rasterizer::Scene scene;
    rasterizer::Application app(title, scene);

    while (app.isRunning) {
        const std::uint64_t frameStart = SDL_GetTicks64();
        app.processInput();
        app.update();
        app.render();
        if (const std::uint64_t pendingFrameTime = SDL_GetTicks64() - frameStart;
            pendingFrameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - pendingFrameTime);
        }
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::fprintf(stderr, "Exiting due to: %s\n", e.what());
    return EXIT_FAILURE;
}
