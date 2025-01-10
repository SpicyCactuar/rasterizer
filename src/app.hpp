#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <numbers>
#include <print>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

#include "mesh.hpp"
#include "scene.hpp"
#include "canvas.hpp"
#include "frustum.hpp"

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
            frustum = new Frustum(
                static_cast<glm::float32_t>(framebufferHeight) / static_cast<glm::float32_t>(framebufferWidth),
                std::numbers::pi / 3.0f, // 60 degrees
                0.01f, 100.0f
            );
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
            if (frustum != nullptr) {
                delete frustum;
                frustum = nullptr;
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
                    break;
                case SDL_KEYUP:
                    processKeypress(event.key.keysym.sym);
                    break;
                default:
                    break;
            }
        }

        void update() const {
            for (Mesh& mesh : scene.meshes) {
                mesh.eulerRotation += glm::vec3{0.01f, 0.01f, 0.01f};
                // Put object in front of camera
                mesh.translation.z = 5.0f;
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
        Scene& scene;

        // TODO: Initialize in Window object to avoid having these fields as pointers
        // TODO: Use smart pointers
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        Canvas* canvas = nullptr;
        SDL_Texture* framebufferTexture = nullptr;

        Frustum* frustum;

        bool backFaceCulling = true;

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

        void processKeypress(const SDL_Keycode keycode) {
            switch (keycode) {
                case SDLK_ESCAPE:
                    isRunning = false;
                    break;
                case SDLK_1:
                    canvas->disable(PolygonMode::FILL);
                    canvas->enable(PolygonMode::LINE);
                    canvas->enable(PolygonMode::POINT);
                    break;
                case SDLK_2:
                    canvas->disable(PolygonMode::FILL);
                    canvas->enable(PolygonMode::LINE);
                    canvas->disable(PolygonMode::POINT);
                    break;
                case SDLK_3:
                    canvas->enable(PolygonMode::FILL);
                    canvas->disable(PolygonMode::LINE);
                    canvas->disable(PolygonMode::POINT);
                    canvas->set(FillMode::SOLID);
                    break;
                case SDLK_4:
                    canvas->enable(PolygonMode::FILL);
                    canvas->enable(PolygonMode::LINE);
                    canvas->disable(PolygonMode::POINT);
                    canvas->set(FillMode::SOLID);
                    break;
                case SDLK_5:
                    canvas->enable(PolygonMode::FILL);
                    canvas->enable(PolygonMode::LINE);
                    canvas->disable(PolygonMode::POINT);
                    canvas->set(FillMode::TEXTURE);
                    break;
                case SDLK_6:
                    canvas->enable(PolygonMode::FILL);
                    canvas->disable(PolygonMode::LINE);
                    canvas->disable(PolygonMode::POINT);
                    canvas->set(FillMode::TEXTURE);
                    break;
                case SDLK_c:
                    backFaceCulling = true;
                    break;
                case SDLK_d:
                    backFaceCulling = false;
                    break;
                default:
                    break;
            }
        }

        void drawScene() const {
            auto trianglesToRender = computeTrianglesToRender();

            // Sort from back to front
            std::vector<size_t> indicesByDepth(trianglesToRender.size());
            std::iota(indicesByDepth.begin(), indicesByDepth.end(), 0);
            std::sort(indicesByDepth.begin(), indicesByDepth.end(),
                      [&trianglesToRender](const size_t t1, const size_t t2) {
                          return trianglesToRender[t1].averageDepth > trianglesToRender[t2].averageDepth;
                      });

            for (const auto ti : indicesByDepth) {
                canvas->drawTriangle(trianglesToRender[ti]);
            }
        }

        std::vector<Triangle> computeTrianglesToRender() const {
            std::vector<Triangle> trianglesToRender;
            trianglesToRender.reserve(scene.meshes.size());

            const auto projection = frustum->perspectiveProjection();
            const auto viewport = glm::mat4{
                canvas->width / 2.0f, 0.0f, 0.0f, 0.0f,
                0.0f, canvas->height / 2.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                canvas->width / 2.0f, canvas->height / 2.0f, 0.0f, 1.0f
            };

            for (auto& mesh : scene.meshes) {
                for (std::size_t face = 0; face < mesh.facesAmount(); ++face) {
                    // Extract and transform vertices
                    const auto meshModel = mesh.modelTransformation();
                    auto [v0, v1, v2] = std::apply(
                        [](const auto&... vertices) {
                            return std::make_tuple(glm::vec4{vertices, 1.0f}...);
                        },
                        mesh[face].vertices
                    );
                    v0 = toWorldCoordinate(v0, meshModel); /*    v0     */
                    v1 = toWorldCoordinate(v1, meshModel); /*  /    \   */
                    v2 = toWorldCoordinate(v2, meshModel); /* v2 --- v1 */

                    const auto e01 = glm::normalize(glm::vec3(v1 - v0));
                    const auto e02 = glm::normalize(glm::vec3(v2 - v0));
                    const auto normal = glm::normalize(glm::cross(e01, e02));

                    // Cull if necessary
                    if (backFaceCulling) {
                        const auto triangleToCamera = glm::normalize(frustum->position - glm::vec3(v0));

                        // Cull if triangle normal and triangleToCamera are not pointing in the same direction
                        if (glm::dot(normal, triangleToCamera) < 0.0f) {
                            continue;
                        }
                    }

                    // Compute synthetic color from face index
                    const std::uint32_t colorSeed = face * 2654435761u;
                    constexpr color_t a = 0xFF000000;
                    const color_t r = colorSeed & 0x00FF0000; // 0x00RR0000
                    const color_t g = colorSeed & 0x0000FF00; // 0x0000GG00
                    const color_t b = colorSeed & 0x000000FF; // 0x000000BB
                    const color_t triangleColor = a | r | g | b;

                    // Draw the centered triangles of the mesh
                    const auto triangle = Triangle{
                        .vertices = {
                            // Map from clip space to screen space
                            toScreenCoordinate(v0, projection, viewport),
                            toScreenCoordinate(v1, projection, viewport),
                            toScreenCoordinate(v2, projection, viewport)
                        },
                        .uvs = mesh[face].uvs,
                        .averageDepth = (v0.z + v1.z + v2.z) / 3.0f,
                        .color = scene.light.modulateSurfaceColor(triangleColor, normal)
                    };

                    trianglesToRender.emplace_back(triangle);
                }
            }

            trianglesToRender.shrink_to_fit();
            return trianglesToRender;
        }

        static glm::vec4 toWorldCoordinate(const glm::vec4& point, const glm::mat4& model) {
            return model * point;
        }

        static glm::vec4 toScreenCoordinate(const glm::vec4& point,
                                            const glm::mat4& projection,
                                            const glm::mat4& viewport) {
            // World-space -> Clip-space
            auto projected = projection * point;

            // {x, y} Clip-space -> {x, y} Screen-space
            projected = viewport * projected;

            // Fail safe to avoid division by 0
            if (projected.w == 0.0f) {
                return projected;
            }

            // Perspective divide
            projected.x /= projected.w;
            projected.y /= projected.w;
            projected.z /= projected.w;
            return projected;
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
