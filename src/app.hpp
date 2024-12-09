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
            window = createWindow(title, windowWidth, windowHeight);
            renderer = createRenderer(window);

            if (window == nullptr || renderer == nullptr) {
                throw std::runtime_error("Failed to initialize SDL window");
            }

            framebuffer = static_cast<std::uint32_t*>(std::calloc(windowWidth * windowHeight, sizeof(uint32_t)));
            if (framebuffer == nullptr) {
                throw std::runtime_error("Failed to allocate framebuffer");
            }
            framebufferWidth = windowWidth;
            framebufferHeight = windowHeight;

            framebufferTexture = createFramebufferTexture(renderer, windowWidth, windowHeight);

            if (framebufferTexture == nullptr) {
                throw std::runtime_error("Failed to create framebufferTexture");
            }

            isRunning = true;
        }

        ~Application() {
            isRunning = false;

            if (framebufferTexture != nullptr) {
                SDL_DestroyTexture(framebufferTexture);
                framebufferTexture = nullptr;
            }
            if (framebuffer != nullptr) {
                std::free(framebuffer);
                framebuffer = nullptr;
            }

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
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            clearFramebuffer();
            renderFramebufferTexture();

            SDL_RenderPresent(renderer);
        }

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        /*
         * Intentionally handled in a C-like manner for learning purposes.
         * The C++ approach I would use is std::array<std::array<std::uint32_t, width>, height>
         */
        std::uint32_t* framebuffer = nullptr;
        std::uint32_t framebufferWidth, framebufferHeight;
        SDL_Texture* framebufferTexture = nullptr;

        static bool initializeSDL() {
            if (SDL_Init(SDL_INIT_EVERYTHING) != EXIT_SUCCESS) {
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

        static SDL_Window* createWindow(const std::string_view& title,
                                        const std::uint32_t width,
                                        const std::uint32_t height) {
            SDL_Window* window = SDL_CreateWindow(title.data(),
                                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);

            if (window == nullptr) {
                std::printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
                return nullptr;
            }

            return window;
        }

        static SDL_Renderer* createRenderer(SDL_Window* window) {
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

            if (renderer == nullptr) {
                std::printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
                return nullptr;
            }

            return renderer;
        }

        static SDL_Texture* createFramebufferTexture(SDL_Renderer* renderer,
                                                     const std::uint32_t width,
                                                     const std::uint32_t height) {
            SDL_Texture* framebufferTexture = SDL_CreateTexture(renderer,
                                                                SDL_PIXELFORMAT_ARGB8888,
                                                                SDL_TEXTUREACCESS_STREAMING,
                                                                width, height);

            if (framebufferTexture == nullptr) {
                std::printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
                return nullptr;
            }

            return framebufferTexture;
        }

        void clearFramebuffer() const {
            static constexpr std::uint32_t clearColor = 0xFF7C3AED;
            static constexpr std::uint32_t circleColor = 0xFF2E1065;

            // Clear framebuffer and draw a 100 pixel radius circle in the center
            for (int j = 0; j < framebufferHeight; ++j) {
                for (int i = 0; i < framebufferWidth; ++i) {
                    const int x = i - framebufferWidth / 2;
                    const int y = j - framebufferHeight / 2;
                    framebuffer[i + j * framebufferWidth] = std::sqrt(x*x + y*y) <= 100 ? circleColor : clearColor;
                }
            }
        }

        void renderFramebufferTexture() const {
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr, framebuffer,
                                                  static_cast<int>(sizeof(uint32_t) * framebufferWidth));
            const auto renderCopy = SDL_RenderCopy(renderer, framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render framebuffer texture");
            }
        }
    };
}
