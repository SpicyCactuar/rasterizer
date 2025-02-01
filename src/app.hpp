#pragma once

#include <algorithm>
#include <numbers>
#include <filesystem>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "color.hpp"
#include "scene.hpp"
#include "frustum.hpp"
#include "canvas.hpp"
#include "context.hpp"
#include "mesh.hpp"
#include "polygon.hpp"

namespace rasterizer {
#ifndef RESOLUTION_SCALE
    #define RESOLUTION_SCALE 1
#endif

    static constexpr std::uint32_t resolutionScale = std::max(RESOLUTION_SCALE, 1);

    class Application {
    public:
        bool isRunning = false;

        explicit Application(const std::string_view& title)
            : context(title),
              canvas(context.windowWidth / resolutionScale, context.windowHeight / resolutionScale, context),
              frustum(
                  static_cast<glm::float32_t>(canvas.width), static_cast<glm::float32_t>(canvas.height),
                  std::numbers::pi / 3.0f, // 60 degrees
                  0.1f, 100.0f
              ) {
            isRunning = true;
        }

        ~Application() {
            isRunning = false;
        }

        void processInput(const glm::float32_t delta) {
            SDL_Event event;
            SDL_PollEvent(&event);

            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN:
                    processKeypress(event.key.keysym.sym, delta);
                    break;
                default:
                    break;
            }
        }

        void update(const glm::float32_t delta) {
            // Orientate frustum according to rotation
            auto cameraRotation = glm::rotate(glm::identity<glm::mat4>(), frustum.yaw, up);
            cameraRotation = glm::rotate(cameraRotation, frustum.pitch, right);
            frustum.forward = glm::mat3(cameraRotation) * forward;
        }

        void render() const {
            context.clear();
            canvas.clear();
            canvas.drawGrid();
            drawScene();
            context.render(canvas.texture(), canvas.framebuffer(), canvas.width);
            context.present();
        }

    private:
        // Left-handed system => +Z forward
        static constexpr glm::vec3 forward{0.0f, 0.0, 1.0f};
        static constexpr glm::vec3 right{1.0f, 0.0f, 0.0f};
        static constexpr glm::vec3 up{0.0f, 1.0f, 0.0f};

        Scene scene;
        RenderContext context;
        Canvas canvas;
        Frustum frustum;

        bool backFaceCulling = true;
        RasterizationRule currentRule = RasterizationRule::DDA;

        void processKeypress(const SDL_Keycode keycode, const glm::float32_t delta) {
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
                    canvas.set(FillMode::VERTEX_COLOR);
                    break;
                case SDLK_4:
                    canvas.enable(PolygonMode::FILL);
                    canvas.enable(PolygonMode::LINE);
                    canvas.disable(PolygonMode::POINT);
                    canvas.set(FillMode::VERTEX_COLOR);
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
                case SDLK_x:
                    canvas.set(RasterizationRule::DDA);
                    break;
                case SDLK_z:
                    canvas.set(RasterizationRule::TOP_LEFT);
                    break;
                case SDLK_c:
                    backFaceCulling = !backFaceCulling;
                    break;
                case SDLK_UP:
                    frustum.pitch += 1.0f * delta;
                    break;
                case SDLK_DOWN:
                    frustum.pitch -= 1.0f * delta;
                    break;
                case SDLK_LEFT:
                    frustum.yaw -= 1.0f * delta;
                    break;
                case SDLK_RIGHT:
                    frustum.yaw += 1.0f * delta;
                    break;
                case SDLK_w: {
                    frustum.eye += 5.0f * frustum.forward * delta;
                    break;
                }
                case SDLK_s: {
                    frustum.eye += 5.0f * -frustum.forward * delta;
                    break;
                }
                case SDLK_d: {
                    frustum.eye += 5.0f * glm::normalize(glm::cross(up, frustum.forward)) * delta;
                    break;
                }
                case SDLK_a: {
                    frustum.eye += 5.0f * -glm::normalize(glm::cross(up, frustum.forward)) * delta;
                    break;
                }
                default:
                    break;
            }
        }

        void drawScene() const {
            const auto trianglesToRender = computeTrianglesToRender();

            scene.lock();
            for (const auto& triangle : trianglesToRender) {
                canvas.drawTriangle(triangle);
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

            // Offset the camera position in the direction where the camera is pointing at
            const auto view = frustum.view(frustum.eye + frustum.forward, up);
            for (std::size_t m = 0; m < scene.meshes.size(); ++m) {
                const auto& mesh = scene.meshes[m];
                for (std::size_t face = 0; face < mesh.facesAmount(); ++face) {
                    // Extract vertices
                    const auto meshModel = mesh.modelTransformation();
                    auto [v0, v1, v2] = std::apply(
                        [](const auto&... vertices) {
                            return std::make_tuple(glm::vec4{vertices, 1.0f}...);
                        },
                        mesh[face].vertices
                    );

                    // Transform to View-space
                    v0 = toViewSpace(v0, meshModel, view); /*    v0     */
                    v1 = toViewSpace(v1, meshModel, view); /*  /    \   */
                    v2 = toViewSpace(v2, meshModel, view); /* v2 --- v1 */

                    // Cull if necessary
                    const auto normal = computeNormal(v0, v1, v2);

                    if (backFaceCulling) {
                        // Points are in View-space, camera position in View-space is always [0 0 0]
                        // [0 0 0] - v = -v
                        const auto triangleToCamera = glm::normalize(-glm::vec3(v0));

                        // Cull if triangle normal and triangleToCamera are not pointing in the same direction
                        if (glm::dot(normal, triangleToCamera) < 0.0f) {
                            continue;
                        }
                    }

                    // Clip and add clipped triangles to result
                    const auto clippedPolygon = frustum.clipPolygon(
                        Polygon::fromTriangle({v0, v1, v2}, mesh[face].uvs));

                    for (std::size_t t = 0; t < clippedPolygon.trianglesAmount(); ++t) {
                        const auto [pv0, pv1, pv2, puv0, puv1, puv2] = clippedPolygon[t];

                        const color_t surfaceColor = scene.light.modulateSurfaceColor(
                            rasterizer::randomColor(face), normal);
                        trianglesToRender.emplace_back(Triangle{
                            .vertices = {
                                // These are points, not vectors => w = 1.0f
                                toScreenSpace(glm::vec4{pv0, 1.0f}, projection, viewport),
                                toScreenSpace(glm::vec4{pv1, 1.0f}, projection, viewport),
                                toScreenSpace(glm::vec4{pv2, 1.0f}, projection, viewport)
                            },
                            .uvs = {puv0, puv1, puv2},
                            .colors = {surfaceColor, surfaceColor, surfaceColor},
                            .surface = scene.meshSurfaces[m].get()
                        });
                    }
                }
            }

            return trianglesToRender;
        }

        static glm::vec4 toViewSpace(const glm::vec4& pointModelSpace,
                                     const glm::mat4& model,
                                     const glm::mat4& view) {
            // Model-space -> World-space -> View-space
            return view * model * pointModelSpace;
        }

        static glm::vec4 toScreenSpace(const glm::vec4& pointViewSpace,
                                       const glm::mat4& projection,
                                       const glm::mat4& viewport) {
            // View-space -> Clip-space
            auto projected = projection * pointViewSpace;

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
