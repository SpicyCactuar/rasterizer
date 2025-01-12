#pragma once

#include <cstdint>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

namespace rasterizer {
    static constexpr std::uint32_t defaultWindowWidth = 1600;
    static constexpr std::uint32_t defaultWindowHeight = 1075;

    class RenderContext {
    public:
        // TODO: Use smart pointers
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* framebufferTexture = nullptr;

        std::uint32_t framebufferWidth, framebufferHeight;

        explicit RenderContext(const std::string_view& windowTitle) {
            if (!initializeSDL()) {
                throw std::runtime_error("Failed to initialize SDL");
            }

            window = createWindow(windowTitle, defaultWindowWidth, defaultWindowHeight);
            if (window == nullptr) {
                throw std::runtime_error("Failed to create window");
            }

            renderer = createRenderer(window);
            if (renderer == nullptr) {
                throw std::runtime_error("Failed to initialize renderer");
            }

            std::int32_t windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            framebufferWidth = static_cast<std::uint32_t>(windowWidth);
            framebufferHeight = static_cast<std::uint32_t>(windowHeight);

            std::print("Display size: {} x {}\n", framebufferWidth, framebufferHeight);

            framebufferTexture = createFramebufferTexture(renderer, framebufferWidth, framebufferHeight);
            if (framebufferTexture == nullptr) {
                throw std::runtime_error("Failed to initialize framebuffer texture");
            }
        }

        ~RenderContext() {
            if (framebufferTexture != nullptr) {
                SDL_DestroyTexture(framebufferTexture);
                framebufferTexture = nullptr;
            }
            if (renderer != nullptr) {
                SDL_DestroyRenderer(renderer);
                renderer = nullptr;
            }
            if (window != nullptr) {
                SDL_DestroyWindow(window);
                window = nullptr;
            }
            quitSDL();
        }

        void clear() const {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        void render(const Canvas& canvas) const {
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr, static_cast<const color_t*>(canvas),
                                                  static_cast<int>(sizeof(color_t) * framebufferWidth));
            const auto renderCopy = SDL_RenderCopy(renderer, framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render framebuffer texture");
            }
        }

        void present() const {
            SDL_RenderPresent(renderer);
        }

    private:
        static bool initializeSDL() {
            if (SDL_Init(SDL_INIT_EVERYTHING) != EXIT_SUCCESS) {
                std::print("SDL_Init Error: %s\n", SDL_GetError());
                return false;
            }

            // IMG_Init returns a bit mask
            if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
                std::print("IMG_Init Error: %s\n", IMG_GetError());
                return false;
            }

            SDL_version compiled, linked;
            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);

            std::print("Compiled against SDL version: {}.{}.{}\n",
                       compiled.major, compiled.minor, compiled.patch);
            std::print("Linked SDL version: {}.{}.{}\n",
                       linked.major, linked.minor, linked.patch);

            return true;
        }

        static void quitSDL() {
            IMG_Quit();
            SDL_Quit();
        }

        static SDL_Window* createWindow(const std::string_view& title,
                                        const std::uint32_t width,
                                        const std::uint32_t height) {
            SDL_DisplayMode displayMode;
            const auto displayModeQuery = SDL_GetCurrentDisplayMode(0, &displayMode);
            const bool fullscreen = displayModeQuery == EXIT_SUCCESS;

            const std::uint32_t windowWidth = fullscreen && displayMode.w > 0 ? displayMode.w : width;
            const std::uint32_t windowHeight = fullscreen && displayMode.h > 0 ? displayMode.h : height;

            SDL_Window* window = SDL_CreateWindow(title.data(),
                                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                  static_cast<std::int32_t>(windowWidth),
                                                  static_cast<std::int32_t>(windowHeight),
                                                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);

            if (window == nullptr) {
                std::print(std::cerr, "SDL_CreateWindow Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return window;
        }

        static SDL_Renderer* createRenderer(SDL_Window* window) {
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

            if (renderer == nullptr) {
                std::print(std::cerr, "SDL_CreateRenderer Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return renderer;
        }

        static SDL_Texture* createFramebufferTexture(SDL_Renderer* renderer,
                                                     const std::uint32_t width,
                                                     const std::uint32_t height) {
            SDL_Texture* framebufferTexture = SDL_CreateTexture(renderer,
                                                                SDL_PIXELFORMAT_RGBA8888,
                                                                SDL_TEXTUREACCESS_STREAMING,
                                                                static_cast<std::int32_t>(width),
                                                                static_cast<std::int32_t>(height));

            if (framebufferTexture == nullptr) {
                std::print(std::cerr, "SDL_CreateTexture Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return framebufferTexture;
        }
    };
}
