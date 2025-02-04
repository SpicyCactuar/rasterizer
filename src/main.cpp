#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "common.hpp"
#include "app.hpp"

static std::uint64_t previousFrameTime = 0;

void newFrame(void* argument) {
    if (const auto app = static_cast<rasterizer::Application*>(argument); app != nullptr && app->isRunning) {
        const auto deltaTime = static_cast<glm::float32_t>(SDL_GetTicks64() - previousFrameTime) / 1000.0f;
        previousFrameTime = SDL_GetTicks64();

        app->processInput(deltaTime);
        app->update(deltaTime);
        app->render();
    }
}

int main(int argc, char* argv[]) try {
    constexpr std::string_view title = "Hello Rasterizer";
    constexpr std::uint32_t FPS = 120;
    constexpr std::uint64_t FRAME_TARGET_TIME = 1000 / FPS;

    rasterizer::Application app(title);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(newFrame, &app, 0, true);
#else
    while (app.isRunning) {
        // Only delay execution if we are running too fast
        if (const std::uint64_t timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
            0 < timeToWait && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        newFrame(&app);
    }
#endif

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    rasterizer::print("Exiting due to: {}\n", e.what());
    return EXIT_FAILURE;
}
