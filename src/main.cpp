#include <print>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "app.hpp"

int main(int argc, char* argv[]) try {
    static constexpr std::string_view title = "Hello Rasterizer";
    static constexpr std::uint32_t FPS = 120;
    static constexpr std::uint64_t FRAME_TARGET_TIME = 1000 / FPS;

    rasterizer::Application app(title);
    std::uint64_t previousFrameTime = 0;

    while (app.isRunning) {
        // Only delay execution if we are running too fast
        if (const std::uint64_t timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
            0 < timeToWait && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        const auto deltaTime = static_cast<glm::float32_t>(SDL_GetTicks64() - previousFrameTime) / 1000.0f;

        previousFrameTime = SDL_GetTicks64();

        app.processInput();
        app.update(deltaTime);
        app.render();
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::print("Exiting due to: {}\n", e.what());
    return EXIT_FAILURE;
}
