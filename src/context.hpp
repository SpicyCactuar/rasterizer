#pragma once

#include <cstdint>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

namespace rasterizer {
    static constexpr std::uint32_t defaultWindowWidth = 1600, defaultWindowHeight = 1075;

    class RenderContext {
    public:
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{nullptr, SDL_DestroyWindow};
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer{nullptr, SDL_DestroyRenderer};

        std::uint32_t windowWidth;
        std::uint32_t windowHeight;

        explicit RenderContext(const std::string_view& windowTitle) {
            if (!initializeSDL()) {
                throw std::runtime_error("Failed to initialize SDL");
            }

            SDL_Window* rawWindow = createWindow(windowTitle, defaultWindowWidth, defaultWindowHeight);
            if (rawWindow == nullptr) {
                throw std::runtime_error("Failed to create window");
            }
            window.reset(rawWindow);

            SDL_Renderer* rawRenderer = createRenderer(rawWindow);
            if (rawRenderer == nullptr) {
                throw std::runtime_error("Failed to initialize renderer");
            }
            renderer.reset(rawRenderer);

            std::int32_t windowWidth, windowHeight;
            SDL_GetWindowSize(rawWindow, &windowWidth, &windowHeight);

            this->windowWidth = static_cast<std::uint32_t>(windowWidth);
            this->windowHeight = static_cast<std::uint32_t>(windowHeight);

            std::print("Display size: {} x {}\n", windowWidth, windowHeight);
        }

        ~RenderContext() {
            // Field destructors are called before this body
            // Therefore, quitSDL is executed at the end
            quitSDL();
        }

        void clear() const {
            SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.get());
        }

        void render(SDL_Texture* framebufferTexture,
                    const color_t* framebuffer, const std::uint32_t framebufferStride) const {
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr,
                                                  framebuffer, static_cast<int>(sizeof(color_t) * framebufferStride));
            const auto renderCopy = SDL_RenderCopy(renderer.get(), framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render frame");
            }
        }

        void present() const {
            SDL_RenderPresent(renderer.get());
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
    };
}
