#pragma once

#include <cstdint>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

#include "ui.hpp"

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

            rasterizer::print("Display size: {} x {}\n", windowWidth, windowHeight);

            rasterizer::ui::initialize(window.get(), renderer.get());
        }

        ~RenderContext() {
            rasterizer::ui::destroy();
            // Field destructors are called before this body
            // Therefore, quitSDL is executed at the end
            quitSDL();
        }

        void newFrame() const {
            // Clear renderer
            const auto& io = ImGui::GetIO();
            SDL_RenderSetScale(renderer.get(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.get());

            // UI new frame
            rasterizer::ui::newFrame();
        }

        void render(SDL_Texture* framebufferTexture,
                    const color_t* framebuffer, const std::uint32_t framebufferStride) const {
            // Render frame
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr,
                                                  framebuffer, static_cast<int>(sizeof(color_t) * framebufferStride));
            const auto renderCopy = SDL_RenderCopy(renderer.get(), framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render frame");
            }
        }

        void present() const {
            rasterizer::ui::present(renderer.get());
            SDL_RenderPresent(renderer.get());
        }

    private:
        static bool initializeSDL() {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != EXIT_SUCCESS) {
                rasterizer::print("SDL_Init Error: {}\n", SDL_GetError());
                return false;
            }

            // IMG_Init returns a bit mask
            if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
                rasterizer::print("IMG_Init Error: {}\n", IMG_GetError());
                return false;
            }

            SDL_version compiled, linked;
            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);

            rasterizer::print("Compiled against SDL version: {}.{}.{}\n",
                              compiled.major, compiled.minor, compiled.patch);
            rasterizer::print("Linked SDL version: {}.{}.{}\n",
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
                                                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);

            if (window == nullptr) {
                rasterizer::print(std::cerr, "SDL_CreateWindow Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return window;
        }

        static SDL_Renderer* createRenderer(SDL_Window* window) {
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

            if (renderer == nullptr) {
                rasterizer::print(std::cerr, "SDL_CreateRenderer Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return renderer;
        }
    };
}
