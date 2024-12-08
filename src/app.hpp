#pragma once

#include <cstdio>
#include <cstdint>
#include <stdexcept>

#include <SDL.h>
#include <SDL_render.h>

namespace rasterizer {
    class Application {
    public:
        bool isRunning = false;

        explicit Application(const std::string_view& title,
                             const std::uint32_t windowWidth,
                             const std::uint32_t windowHeight) {
            initializeSDL();
            window = initializeWindow(title, windowWidth, windowHeight);
            renderer = initializeRenderer(window);

            if (window == nullptr || renderer == nullptr) {
                throw std::runtime_error("Failed to initialize application");
            }

            isRunning = true;
        }

        ~Application() {
            isRunning = false;
            if (renderer != nullptr) {
                SDL_DestroyRenderer(renderer);
                renderer = nullptr;
            }
            if (window != nullptr) {
                SDL_DestroyWindow(window);
                window = nullptr;
            }
            SDL_Quit();
        }

        void processInput() {
            SDL_Event event;
            SDL_PollEvent(&event);

            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                case SDL_KEYDOWN:
                    if (SDL_SCANCODE_ESCAPE == event.key.keysym.scancode) {
                        isRunning = false;
                    }
                default:
                    break;
            }
        }

        void update() {
            // TODO: Implement
        }

        void render() const {
            SDL_SetRenderDrawColor(renderer, 124, 58, 237, 255);
            SDL_RenderClear(renderer);

            SDL_RenderPresent(renderer);
        }

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        std::uint32_t* colorBuffer;

        static bool initializeSDL() {
            if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                std::printf("SDL_Init Error: %s\n", SDL_GetError());
                return false;
            }

            SDL_version compiled, linked;
            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);

            std::printf("Compiled against SDL version: %d.%d.%d\n",
                        compiled.major, compiled.minor, compiled.patch);
            std::printf("Linked SDL version: %d.%d.%d\n",
                        linked.major, linked.minor, linked.patch);

            return true;
        }

        static SDL_Window* initializeWindow(const std::string_view& title,
                                            const std::uint32_t width,
                                            const std::uint32_t height) {
            SDL_Window* window = SDL_CreateWindow(title.data(),
                                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);

            if (window == nullptr) {
                std::printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
                return nullptr;
            }

            return window;
        }

        static SDL_Renderer* initializeRenderer(SDL_Window* window) {
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

            if (renderer == nullptr) {
                std::printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
                return nullptr;
            }

            return renderer;
        }
    };
}
