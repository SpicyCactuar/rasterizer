#pragma once

#include <algorithm>
#include <numeric>
#include <numbers>
#include <filesystem>

#include <SDL2/SDL.h>

#include "scene.hpp"
#include "obj.hpp"
#include "frustum.hpp"
#include "canvas.hpp"
#include "context.hpp"
#include "mesh.hpp"

namespace rasterizer {
    class Application {
    public:
        bool isRunning = false;

        explicit Application(const std::string_view& title)
            : scene({rasterizer::parseObj("../assets/f22.obj")}),
              context(title),
              canvas(context.framebufferWidth, context.framebufferHeight),
              frustum(
                  static_cast<glm::float32_t>(canvas.height) /
                  static_cast<glm::float32_t>(canvas.width),
                  std::numbers::pi / 3.0f, // 60 degrees
                  0.01f, 100.0f
              ) {
            isRunning = true;
        }

        ~Application() {
            isRunning = false;
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

        void update() {
            for (Mesh& mesh : scene.meshes) {
                mesh.eulerRotation += glm::vec3{0.01f, 0.01f, 0.01f};
                // Put object in front of camera
                mesh.translation.z = 5.0f;
            }
        }

        void render() const {
            context.clear();
            canvas.clear();
            canvas.drawGrid();
            drawScene();
            context.render(canvas);
            context.present();
        }

    private:
        Scene scene;
        RenderContext context;
        Canvas canvas;
        Frustum frustum;

        bool backFaceCulling = true;

        void processKeypress(const SDL_Keycode keycode) {
            switch (keycode) {
                case SDLK_ESCAPE:
                    isRunning = false;
                    break;
                case SDLK_1:
                    canvas.disable(PolygonMode::FILL);
                    canvas.enable(PolygonMode::LINE);
                    canvas.enable(PolygonMode::POINT);
                    break;
                case SDLK_2:
                    canvas.disable(PolygonMode::FILL);
                    canvas.enable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    break;
                case SDLK_3:
                    canvas.enable(PolygonMode::FILL);
                    canvas.disable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    canvas.set(FillMode::SOLID);
                    break;
                case SDLK_4:
                    canvas.enable(PolygonMode::FILL);
                    canvas.enable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    canvas.set(FillMode::SOLID);
                    break;
                case SDLK_5:
                    canvas.enable(PolygonMode::FILL);
                    canvas.enable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    canvas.set(FillMode::TEXTURE);
                    break;
                case SDLK_6:
                    canvas.enable(PolygonMode::FILL);
                    canvas.disable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    canvas.set(FillMode::TEXTURE);
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
            std::vector<std::size_t> indicesByDepth(trianglesToRender.size());
            std::iota(indicesByDepth.begin(), indicesByDepth.end(), 0);
            std::sort(indicesByDepth.begin(), indicesByDepth.end(),
                      [&trianglesToRender](const std::size_t t1, const std::size_t t2) {
                          return trianglesToRender[t1].averageDepth > trianglesToRender[t2].averageDepth;
                      });

            scene.lock();
            for (const auto ti : indicesByDepth) {
                canvas.drawTriangle(trianglesToRender[ti]);
            }
            scene.unlock();
        }

        std::vector<Triangle> computeTrianglesToRender() const {
            std::vector<Triangle> trianglesToRender;
            trianglesToRender.reserve(scene.meshes.size());

            const auto projection = frustum.perspectiveProjection();
            const auto viewport = glm::mat4{
                canvas.width / 2.0f, 0.0f, 0.0f, 0.0f,
                0.0f, canvas.height / 2.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                canvas.width / 2.0f, canvas.height / 2.0f, 0.0f, 1.0f
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
                        const auto triangleToCamera = glm::normalize(frustum.position - glm::vec3(v0));

                        // Cull if triangle normal and triangleToCamera are not pointing in the same direction
                        if (glm::dot(normal, triangleToCamera) < 0.0f) {
                            continue;
                        }
                    }

                    // Compute synthetic color from face index
                    const color_t colorSeed = face * 2654435761u;
                    constexpr color_t a = 0x000000FF;
                    const color_t r = colorSeed & 0xFF000000; // 0xRR000000
                    const color_t g = colorSeed & 0x00FF0000; // 0x00GG0000
                    const color_t b = colorSeed & 0x0000FF00; // 0x0000BB00
                    const color_t triangleColor = r | g | b | a;

                    const auto triangle = Triangle{
                        .vertices = {
                            // Map from clip space to screen space
                            toScreenCoordinate(v0, projection, viewport),
                            toScreenCoordinate(v1, projection, viewport),
                            toScreenCoordinate(v2, projection, viewport)
                        },
                        .uvs = mesh[face].uvs,
                        .averageDepth = (v0.z + v1.z + v2.z) / 3.0f,
                        .solidColor = scene.light.modulateSurfaceColor(triangleColor, normal),
                        .surface = scene.meshSurface.get()
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

            // Fail-safe to avoid division by 0
            if (projected.w == 0.0f) {
                return projected;
            }

            // Perspective divide
            projected.x /= projected.w;
            projected.y /= projected.w;
            projected.z /= projected.w;
            return projected;
        }
    };
}
