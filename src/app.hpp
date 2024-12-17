#pragma once

#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <SDL.h>
#include <SDL_render.h>

namespace rasterizer {
    static constexpr std::uint32_t defaultWindowWidth = 1600;
    static constexpr std::uint32_t defaultWindowHeight = 1075;

    class Application {
    public:
        bool isRunning = false;

        explicit Application(const std::string_view& title) {
            initializeSDL();
            window = createWindow(title, defaultWindowWidth, defaultWindowHeight);
            renderer = createRenderer(window);

            if (window == nullptr || renderer == nullptr) {
                throw std::runtime_error("Failed to initialize SDL window");
            }

            std::int32_t windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            framebufferWidth = static_cast<std::uint32_t>(windowWidth);
            framebufferHeight = static_cast<std::uint32_t>(windowHeight);

            framebuffer = createFramebuffer(framebufferWidth, framebufferHeight);
            framebufferTexture = createFramebufferTexture(renderer, framebufferWidth, framebufferHeight);

            if (framebuffer == nullptr || framebufferTexture == nullptr) {
                throw std::runtime_error("Failed to initialize framebuffer");
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

        void drawRectangles() const {
            static constexpr std::uint32_t rectangleColor = 0xFF7C3AED;

            for (const auto& rectangle : rectangles) {
                const std::uint32_t endX = std::min(
                    static_cast<std::uint32_t>(rectangle.x + rectangle.w), framebufferWidth);
                const std::uint32_t endY = std::min(
                    static_cast<std::uint32_t>(rectangle.y + rectangle.h), framebufferHeight);

                for (std::uint32_t row = rectangle.x; row < endY; ++row) {
                    for (std::uint32_t column = rectangle.y; column < endX; ++column) {
                        framebuffer[row * framebufferWidth + column] = rectangleColor;
                    }
                }
            }
        }

        void render() const {
            clearFramebuffer();
            drawGrid();
            drawRectangles();
            renderFramebufferTexture();
            SDL_RenderPresent(renderer);
        }

        void drawRectangleOnRender(const std::uint32_t positionX, const std::uint32_t positionY,
                                   const std::uint32_t width, const std::uint32_t height) {
            rectangles.emplace_back(positionX, positionY, width, height);
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

        std::vector<SDL_Rect> rectangles;

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
            SDL_DisplayMode displayMode;
            const auto displayModeQuery = SDL_GetDesktopDisplayMode(0, &displayMode);
            const bool fullscreen = displayModeQuery == EXIT_SUCCESS;

            const std::uint32_t windowWidth = fullscreen && displayMode.w > 0 ? displayMode.w : width;
            const std::uint32_t windowHeight = fullscreen && displayMode.h > 0 ? displayMode.h : height;

            SDL_Window* window = SDL_CreateWindow(title.data(),
                                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                                  windowWidth, windowHeight,
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

        static std::uint32_t* createFramebuffer(const std::uint32_t width, const std::uint32_t height) {
            return static_cast<std::uint32_t*>(
                std::calloc(width * height, sizeof(std::uint32_t)));
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
            // Clear renderer
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Clear framebuffer contents
            static constexpr std::uint32_t clearColor = 0xFF2E1065;

            for (std::uint32_t row = 0; row < framebufferHeight; ++row) {
                for (std::uint32_t column = 0; column < framebufferWidth; ++column) {
                    framebuffer[row * framebufferWidth + column] = clearColor;
                }
            }
        }

        void drawGrid() const {
            static constexpr std::uint32_t gridColor = 0xFF7C3AED;

            for (std::uint32_t row = 0; row < framebufferHeight; row += 10) {
                for (std::uint32_t column = 0; column < framebufferWidth; column += 10) {
                    framebuffer[row * framebufferWidth + column] = gridColor;
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
