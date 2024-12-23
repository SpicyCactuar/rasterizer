#pragma once

#include <cstdint>
#include <iostream>
#include <print>
#include <stdexcept>
#include <tuple>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

#include "mesh.hpp"
#include "scene.hpp"
#include "transformation.hpp"

namespace rasterizer {
    static constexpr std::uint32_t defaultWindowWidth = 1600;
    static constexpr std::uint32_t defaultWindowHeight = 1075;

    class Application {
    public:
        bool isRunning = false;

        explicit Application(const std::string_view& title, Scene& scene) : scene(scene) {
            if (!initializeSDL()) {
                throw std::runtime_error("Failed to initialize SDL");
            }

            window = createWindow(title, defaultWindowWidth, defaultWindowHeight);
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

        void update() const {
            for (Mesh& mesh : scene.meshes) {
                mesh.eulerRotation += glm::vec3{0.01f, 0.01f, 0.02f};
            }
        }

        void render() const {
            clearFramebuffer();
            drawGrid();
            drawScene();
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

        Scene& scene;

        static bool initializeSDL() {
            if (SDL_Init(SDL_INIT_EVERYTHING) != EXIT_SUCCESS) {
                std::print("SDL_Init Error: %s\n", SDL_GetError());
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
                                                                static_cast<std::int32_t>(width),
                                                                static_cast<std::int32_t>(height));

            if (framebufferTexture == nullptr) {
                std::print(std::cerr, "SDL_CreateTexture Error: {}\n", SDL_GetError());
                return nullptr;
            }

            return framebufferTexture;
        }

        void drawPixel(const std::uint32_t row, const std::uint32_t column, const std::uint32_t& color) const {
            if (row < framebufferHeight && column < framebufferWidth) {
                framebuffer[row * framebufferWidth + column] = color;
            }
        }

        void drawRectangle(const std::uint32_t x, const std::uint32_t y,
                           const std::uint32_t width, const std::uint32_t height) const {
            static constexpr std::uint32_t rectangleColor = 0xFF7C3AED;

            const std::uint32_t endX = std::min(x + width, framebufferWidth);
            const std::uint32_t endY = std::min(y + height, framebufferHeight);

            for (std::uint32_t row = y; row < endY; ++row) {
                for (std::uint32_t column = x; column < endX; ++column) {
                    drawPixel(row, column, rectangleColor);
                }
            }
        }

        void drawPoint(const glm::vec2& point) const {
            // Draw centered, with side length 10
            drawRectangle(static_cast<std::uint32_t>(point.x) + framebufferWidth / 2,
                          static_cast<std::uint32_t>(point.y) + framebufferHeight / 2,
                          10, 10);
        }

        void drawLine(const glm::vec2& start, const glm::vec2& end) const {
            // DDA line rasterizer
            static constexpr std::uint32_t lineColor = 0xFFA78BFA;
            const std::int32_t dx = static_cast<std::int32_t>(end.x) - static_cast<std::int32_t>(start.x);
            const std::int32_t dy = static_cast<std::int32_t>(end.y) - static_cast<std::int32_t>(start.y);

            const std::uint32_t longestLength = std::abs(dx) >= std::abs(dy) ? std::abs(dx) : abs(dy);

            const auto df = static_cast<glm::float32_t>(longestLength);
            const glm::float32_t xIncrement = static_cast<glm::float32_t>(dx) / df;
            const glm::float32_t yIncrement = static_cast<glm::float32_t>(dy) / df;

            glm::float32_t x = start.x;
            glm::float32_t y = start.y;
            for (std::uint32_t l = 0; l < longestLength; l += 1) {
                drawPixel(static_cast<std::uint32_t>(y) + framebufferHeight / 2,
                          static_cast<std::uint32_t>(x) + framebufferWidth / 2,
                          lineColor);
                x += xIncrement;
                y += yIncrement;
            }
        }

        void drawScene() const {
            for (auto& mesh : scene.meshes) {
                for (std::size_t t = 0; t < mesh.faces.size(); ++t) {
                    // Extract and transform vertices
                    auto [v0, v1, v2] = mesh[t].vertices;
                    v0 = transformPoint(v0, mesh.eulerRotation); /*    v0     */
                    v1 = transformPoint(v1, mesh.eulerRotation); /*  /    \   */
                    v2 = transformPoint(v2, mesh.eulerRotation); /* v2 --- v1 */

                    // Cull if backfacing
                    const auto v01 = v1 - v0;
                    const auto v02 = v2 - v0;
                    const auto normal = rasterizer::cross(v01, v02);
                    const auto triangleToCamera = scene.frustum.position - v0;

                    // Cull if triangle normal and triangleToCamera are not pointing in the same direction
                    if (rasterizer::dot(normal, triangleToCamera) < 0.0f) {
                        continue;
                    }

                    // Draw its points and lines
                    const auto [p0, p1, p2] = std::tuple{
                        scene.frustum.perspectiveDivide(v0),
                        scene.frustum.perspectiveDivide(v1),
                        scene.frustum.perspectiveDivide(v2)
                    };

                    drawPoint(p0);
                    drawPoint(p1);
                    drawPoint(p2);
                    drawLine(p0, p1);
                    drawLine(p0, p2);
                    drawLine(p1, p2);
                }
            }
        }

        static glm::vec3 transformPoint(const glm::vec3& point, const glm::vec3 rotationInDegrees) {
            // First rotate, then translate
            glm::vec3 transformedPoint = point;
            transformedPoint = rotateAroundX(transformedPoint, rotationInDegrees.x);
            transformedPoint = rotateAroundY(transformedPoint, rotationInDegrees.y);
            transformedPoint = rotateAroundZ(transformedPoint, rotationInDegrees.z);
            return transformedPoint + glm::vec3{0.0f, 0.0f, 5.0f};
        }

        void clearFramebuffer() const {
            // Clear renderer
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Clear framebuffer contents
            static constexpr std::uint32_t clearColor = 0xFF2E1065;

            for (std::uint32_t row = 0; row < framebufferHeight; ++row) {
                for (std::uint32_t column = 0; column < framebufferWidth; ++column) {
                    drawPixel(row, column, clearColor);
                }
            }
        }

        void drawGrid() const {
            static constexpr std::uint32_t gridColor = 0xFF7C3AED;

            for (std::uint32_t row = 0; row < framebufferHeight; row += 10) {
                for (std::uint32_t column = 0; column < framebufferWidth; column += 10) {
                    drawPixel(row, column, gridColor);
                }
            }
        }

        void renderFramebufferTexture() const {
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr, framebuffer,
                                                  static_cast<int>(sizeof(std::uint32_t) * framebufferWidth));
            const auto renderCopy = SDL_RenderCopy(renderer, framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render framebuffer texture");
            }
        }
    };
}
