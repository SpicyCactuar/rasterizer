#pragma once

#include <cstdint>
#include <iostream>
#include <print>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

#include "mesh.hpp"
#include "scene.hpp"
#include "transformation.hpp"
#include "canvas.hpp"

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

            const auto framebufferWidth = static_cast<std::uint32_t>(windowWidth);
            const auto framebufferHeight = static_cast<std::uint32_t>(windowHeight);

            std::print("Display size: {} x {}\n", framebufferWidth, framebufferHeight);

            canvas = new Canvas(framebufferWidth, framebufferHeight);

            framebufferTexture = createFramebufferTexture(renderer, framebufferWidth, framebufferHeight);
            if (framebufferTexture == nullptr) {
                throw std::runtime_error("Failed to initialize framebuffer texture");
            }

            isRunning = true;
        }

        ~Application() {
            isRunning = false;

            if (framebufferTexture != nullptr) {
                SDL_DestroyTexture(framebufferTexture);
                framebufferTexture = nullptr;
            }
            if (canvas != nullptr) {
                delete canvas;
                canvas = nullptr;
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
                mesh.eulerRotation += glm::vec3{0.005f, 0.005f, 0.01f};
            }
        }

        void render() const {
            clearFramebuffer();
            canvas->drawGrid();
            drawScene();
            renderFramebufferTexture();
            SDL_RenderPresent(renderer);
        }

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        Canvas* canvas = nullptr;
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

                    const glm::vec2 center = {canvas->width / 2, canvas->height / 2};
                    // Draw the centered triangles of the mesh
                    const auto [p0, p1, p2] = std::tuple{
                        scene.frustum.perspectiveDivide(v0) + center,
                        scene.frustum.perspectiveDivide(v1) + center,
                        scene.frustum.perspectiveDivide(v2) + center
                    };

                    canvas->drawTriangle(p0, p1, p2);
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

            // Clear Canvas
            canvas->clear();
        }

        void renderFramebufferTexture() const {
            const auto update = SDL_UpdateTexture(framebufferTexture, nullptr,
                                                  static_cast<const std::uint32_t*>(*canvas),
                                                  static_cast<int>(sizeof(std::uint32_t) * canvas->width));
            const auto renderCopy = SDL_RenderCopy(renderer, framebufferTexture, nullptr, nullptr);

            if (update != EXIT_SUCCESS || renderCopy != EXIT_SUCCESS) {
                throw std::runtime_error("Failed to render framebuffer texture");
            }
        }
    };
}
