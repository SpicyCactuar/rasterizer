#pragma once

#include <cstdint>
#include <functional>
#include <iostream>

#include <SDL2/SDL_render.h>
#include <glm/glm.hpp>

#include "common.hpp"
#include "context.hpp"
#include "polygon.hpp"

namespace rasterizer {
    enum class PolygonMode : std::uint32_t {
        FILL = 1 << 0,
        LINE = 1 << 1,
        POINT = 1 << 2,
    };

    enum class FillMode : std::uint32_t {
        SOLID = 1 << 0,
        TEXTURE = 1 << 1,
    };

    typedef std::function<color_t(const glm::vec3&, const glm::float32_t&)> ColorShader;

    class Canvas {
    public:
        const std::uint32_t width, height;

        Canvas(const std::uint32_t width, const std::uint32_t height,
               const RenderContext& context) : width(width), height(height) {
            SDL_Texture* rawFramebufferTexture = createFramebufferTexture(context.renderer.get(), width, height);
            if (rawFramebufferTexture == nullptr) {
                throw std::runtime_error("Failed to initialize Canvas.frambufferTexture");
            }
            framebufferTexture.reset(rawFramebufferTexture);

            colorBuffer = createColorBuffer(width, height);
            if (colorBuffer == nullptr) {
                throw std::runtime_error("Failed to create Canvas.colorBuffer");
            }

            depthBuffer = createDepthBuffer(width, height);
            if (depthBuffer == nullptr) {
                throw std::runtime_error("Failed to create Canvas.depthBuffer");
            }
        }

        ~Canvas() {
            if (depthBuffer != nullptr) {
                std::free(depthBuffer);
                depthBuffer = nullptr;
            }
            if (colorBuffer != nullptr) {
                std::free(colorBuffer);
                colorBuffer = nullptr;
            }
        }

        const color_t* framebuffer() const {
            return colorBuffer;
        }

        SDL_Texture* texture() const {
            return framebufferTexture.get();
        }

        void drawPixel(const std::int32_t row, const std::int32_t column, const color_t color) const {
            if (0 <= row && row < height && 0 <= column && column < width) {
                colorBuffer[row * width + column] = color;
            }
        }

        void setDepth(const std::int32_t row, const std::int32_t column, const glm::float32_t depth) const {
            if (0 <= row && row < height && 0 <= column && column < width) {
                depthBuffer[row * width + column] = depth;
            }
        }

        void drawBarycentricPixel(const std::int32_t row, const std::int32_t column,
                                  const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
                                  const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2,
                                  const ColorShader& shader) const {
            if (row < 0 || row >= height || column < 0 || column >= width) {
                return;
            }

            const auto barycentric = barycentricWeights(p0, p1, p2, {column, row});

            // Perspective-correct interpolation
            // TODO: Optimize into a single division
            // https://courses.pikuma.com/courses/take/learn-computer-graphics-programming/lessons/11822193-perspective-correct-interpolation-code/discussions/886660
            const glm::float32_t wReciprocalInterpolated = barycentric.x * (1 / v0.w) +
                                                           barycentric.y * (1 / v1.w) +
                                                           barycentric.z * (1 / v2.w);

            // d = 1 / w which is smaller as w is larger (ie: further away)
            // Invert the value so that d is larger as values are further away
            const glm::float32_t normalizedDepth = 1.0f - wReciprocalInterpolated;

            // If value is further away, we avoid drawing
            if (normalizedDepth >= depthBuffer[row * width + column]) {
                return;
            }

            drawPixel(row, column, shader(barycentric, wReciprocalInterpolated));
            setDepth(row, column, normalizedDepth);
        }

        void drawRectangle(const std::int32_t x, const std::int32_t y,
                           const std::uint32_t width, const std::uint32_t height,
                           const color_t color) const {
            const std::uint32_t endX = std::min(x + width, this->width);
            const std::uint32_t endY = std::min(y + height, this->height);

            for (std::uint32_t row = std::max(y, 0); row < endY; ++row) {
                for (std::uint32_t column = std::max(x, 0); column < endX; ++column) {
                    drawPixel(row, column, color);
                }
            }
        }

        void drawPoint(const glm::ivec2& point, const color_t color) const {
            static constexpr std::uint32_t pointWidth = 10, pointHeight = 10;
            // Draw centered, with side length 10
            drawRectangle(point.x - static_cast<std::int32_t>(pointWidth / 2),
                          point.y - static_cast<std::int32_t>(pointHeight / 2),
                          pointWidth, pointHeight, color);
        }

        void drawLine(const glm::ivec2& start, const glm::ivec2& end, const color_t color) const {
            // DDA line rasterizer
            const std::int32_t dx = end.x - start.x;
            const std::int32_t dy = end.y - start.y;

            const std::uint32_t longestLength = std::abs(dx) >= std::abs(dy) ? std::abs(dx) : std::abs(dy);

            const auto df = static_cast<glm::float32_t>(longestLength);
            const glm::float32_t xIncrement = static_cast<glm::float32_t>(dx) / df;
            const glm::float32_t yIncrement = static_cast<glm::float32_t>(dy) / df;

            glm::float32_t x = start.x;
            glm::float32_t y = start.y;
            for (std::uint32_t l = 0; l <= longestLength; l++) {
                drawPixel(static_cast<std::int32_t>(std::round(y)), static_cast<std::int32_t>(std::round(x)),
                          color);
                x += xIncrement;
                y += yIncrement;
            }
        }

        void drawTriangle(const Triangle& triangle) const {
            // Convention: 3 or 4 dimension vertices -> vN, 2 dimension points pN
            const auto [v0, v1, v2] = triangle.vertices;
            const auto [p0, p1, p2] = std::make_tuple(glm::vec2{v0}, glm::vec2{v1}, glm::vec2{v2});
            // Mirror V coordinate along downward Y axis, effectively flipping the texture
            const auto& [uv0, uv1, uv2] = std::apply(
                [](const auto&... uvs) {
                    return std::make_tuple(glm::vec2{uvs.x, 1.0f - uvs.y}...);
                },
                triangle.uvs
            );
            static constexpr color_t triangleLineColor = 0xA78BFAFF;
            static constexpr color_t trianglePointColor = 0x7C3AEDFF;

            const bool drawTriangleFill = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::FILL);
            const bool drawTriangleLines = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::LINE);
            const bool drawTrianglePoints = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::POINT);
            const bool triangleFillSolid = fillModeMask & static_cast<std::uint32_t>(FillMode::SOLID);

            if (drawTriangleFill) {
                if (triangleFillSolid) {
                    drawSolidTriangle(v0, v1, v2, p0, p1, p2, triangle.solidColor);
                } else {
                    // Only 2 fill modes, therefore this is the FillMode::TEXTURED case
                    drawTexturedTriangle(v0, v1, v2, p0, p1, p2, uv0, uv1, uv2, triangle.surface);
                }
            }

            if (drawTrianglePoints) {
                drawPoint(p0, trianglePointColor);
                drawPoint(p1, trianglePointColor);
                drawPoint(p2, trianglePointColor);
            }

            if (drawTriangleLines) {
                drawLine(p0, p1, triangleLineColor);
                drawLine(p0, p2, triangleLineColor);
                drawLine(p1, p2, triangleLineColor);
            }
        }

        void drawGrid() const {
            static constexpr color_t gridColor = 0x7C3AEDFF;

            for (std::uint32_t row = 0; row < height; row += 10) {
                for (std::uint32_t column = 0; column < width; column += 10) {
                    drawPixel(row, column, gridColor);
                }
            }
        }

        void clear() const {
            // Clear buffers contents
            static constexpr color_t clearColor = 0x2E1065FF;
            static constexpr glm::float32_t clearDepth = 1.0f;

            for (std::uint32_t row = 0; row < height; ++row) {
                for (std::uint32_t column = 0; column < width; ++column) {
                    drawPixel(row, column, clearColor);
                    setDepth(row, column, clearDepth);
                }
            }
        }

        void enable(const PolygonMode mode) {
            polygonModeMask |= static_cast<std::uint32_t>(mode);
        }

        void disable(const PolygonMode mode) {
            polygonModeMask &= ~static_cast<std::uint32_t>(mode);
        }

        void set(const FillMode mode) {
            fillModeMask = static_cast<std::uint32_t>(mode);
        }

    private:
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> framebufferTexture{nullptr, SDL_DestroyTexture};
        /*
         * Intentionally handled in a C-like manner for learning purposes.
         * The C++ approach I would use is boost::static_vector<color_t>
         * Cannot use std::array as size is not known at compile time
         */
        color_t* colorBuffer = nullptr;
        glm::float32_t* depthBuffer = nullptr;

        std::uint32_t polygonModeMask = static_cast<std::uint32_t>(PolygonMode::LINE);
        std::uint32_t fillModeMask = static_cast<std::uint32_t>(FillMode::SOLID);

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

        static color_t* createColorBuffer(const std::uint32_t width, const std::uint32_t height) {
            return static_cast<color_t*>(std::calloc(width * height, sizeof(color_t)));
        }

        static glm::float32_t* createDepthBuffer(const std::uint32_t width, const std::uint32_t height) {
            return static_cast<glm::float32_t*>(std::calloc(width * height, sizeof(color_t)));
        }

        void drawSolidTriangle(glm::vec4 v0, glm::vec4 v1, glm::vec4 v2,
                               glm::ivec2 p0, glm::ivec2 p1, glm::ivec2 p2,
                               const color_t color) const {
            sortAscendingVertically(v0, v1, v2, p0, p1, p2);

            const ColorShader solidColoring = [&color](auto...) {
                return color;
            };

            drawBarycentricTriangle(v0, v1, v2, p0, p1, p2, solidColoring);
        }

        void drawTexturedTriangle(glm::vec4 v0, glm::vec4 v1, glm::vec4 v2,
                                  glm::ivec2 p0, glm::ivec2 p1, glm::ivec2 p2,
                                  glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2,
                                  const Surface* surface) const {
            sortAscendingVertically(v0, v1, v2, p0, p1, p2, uv0, uv1, uv2);

            const ColorShader barycentricColoring = [&v0, &v1, &v2, &uv0, &uv1, &uv2, &surface]
            (const glm::vec3& barycentric, const glm::float32_t& wReciprocal) {
                // v.w holds the depth information but does not interpolate linearly, (1 / v.w) does
                // Interpolate linearly and undo division at the end
                const glm::float32_t uInterpolated = (barycentric.x * (uv0.x / v0.w) +
                                                      barycentric.y * (uv1.x / v1.w) +
                                                      barycentric.z * (uv2.x / v2.w)) / wReciprocal;

                const glm::float32_t vInterpolated = (barycentric.x * (uv0.y / v0.w) +
                                                      barycentric.y * (uv1.y / v1.w) +
                                                      barycentric.z * (uv2.y / v2.w)) / wReciprocal;

                // Map the UV coordinate to the range [0..{texture.width,texture.height} - 1]
                // Apply modulo to account for degenerate point-outside-triangle barycentric case
                const std::size_t xTex = static_cast<std::size_t>(
                                             std::abs(uInterpolated * (surface->width - 1))) % surface->width;
                const std::size_t yTex = static_cast<std::size_t>(
                                             std::abs(vInterpolated * (surface->height - 1))) % surface->height;

                return (*surface)[yTex * surface->width + xTex];
            };

            drawBarycentricTriangle(v0, v1, v2, p0, p1, p2, barycentricColoring);
        }

        ///////////////////////////////////////////////////////////////////////////////
        //
        //            p0
        //            / \
        //           /   \
        //          /     \
        //         /       \
        //        /         \
        //      p1 -------- mid
        //       \_           \
        //          \_         \
        //             \_       \
        //                \_     \
        //                   \    \
        //                     \_  \
        //                        \_\
        //                           \
        //                           p2
        //
        // Based on diagram by: Pikuma (Gustavo Pezzi)
        //
        ///////////////////////////////////////////////////////////////////////////////
        void drawBarycentricTriangle(const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2,
                                     const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2,
                                     const ColorShader& shader) const {
            // Compute inverse slopes 0 -> 1 and 0 -> 2
            glm::float32_t invSlope01 = 0.0f;
            glm::float32_t invSlope02 = 0.0f;

            if (p1.y - p0.y != 0) {
                invSlope01 = static_cast<glm::float32_t>(p1.x - p0.x) / std::abs(p1.y - p0.y);
            }
            if (p2.y - p0.y != 0) {
                invSlope02 = static_cast<glm::float32_t>(p2.x - p0.x) / std::abs(p2.y - p0.y);
            }

            // Draw flat-bottom triangle
            if (p1.y - p0.y != 0) {
                for (std::int32_t y = p0.y; y <= p1.y; ++y) {
                    std::int32_t xStart = p1.x + (y - p1.y) * invSlope01;
                    std::int32_t xEnd = p0.x + (y - p0.y) * invSlope02;

                    if (xEnd < xStart) {
                        // Guarantee xStart is at the left
                        std::swap(xStart, xEnd);
                    }

                    for (std::int32_t x = xStart; x < xEnd; ++x) {
                        drawBarycentricPixel(y, x, v0, v1, v2, p0, p1, p2, shader);
                    }
                }
            }

            // Compute inverse slope 1 -> 2
            glm::float32_t invSlope12 = 0.0f;

            if (p2.y - p1.y != 0) {
                invSlope12 = static_cast<glm::float32_t>(p2.x - p1.x) / std::abs(p2.y - p1.y);
            }

            // Draw flat-top triangle
            if (p2.y - p1.y != 0) {
                for (std::int32_t y = p1.y; y <= p2.y; ++y) {
                    std::int32_t xStart = p1.x + (y - p1.y) * invSlope12;
                    std::int32_t xEnd = p0.x + (y - p0.y) * invSlope02;

                    if (xEnd < xStart) {
                        // Guarantee xStart is at the left
                        std::swap(xStart, xEnd);
                    }

                    for (std::int32_t x = xStart; x < xEnd; ++x) {
                        drawBarycentricPixel(y, x, v0, v1, v2, p0, p1, p2, shader);
                    }
                }
            }
        }

        static void sortAscendingVertically(glm::vec4& v0, glm::vec4& v1, glm::vec4& v2,
                                            glm::ivec2& p0, glm::ivec2& p1, glm::ivec2& p2) {
            // Sort such that p0.y <= p1.y <= p2.y
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
                std::swap(v0, v1);
            }
            if (p2.y < p1.y) {
                // p2.y < p1.y => p1.y < p2.y
                std::swap(p1, p2);
                std::swap(v1, v2);
            }
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
                std::swap(v0, v1);
            }
        }

        static void sortAscendingVertically(glm::vec4& v0, glm::vec4& v1, glm::vec4& v2,
                                            glm::ivec2& p0, glm::ivec2& p1, glm::ivec2& p2,
                                            glm::vec2& uv0, glm::vec2& uv1, glm::vec2& uv2) {
            // Sort such that p0.y <= p1.y <= p2.y
            // Swap vertices and uvs accordingly
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
                std::swap(v0, v1);
                std::swap(uv0, uv1);
            }
            if (p2.y < p1.y) {
                // p2.y < p1.y => p1.y < p2.y
                std::swap(p1, p2);
                std::swap(v1, v2);
                std::swap(uv1, uv2);
            }
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
                std::swap(v0, v1);
                std::swap(uv0, uv1);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////
        //
        // Return the barycentric weights alpha, beta, and gamma for point p
        //
        //         (B)
        //         /|\
        //        / | \
        //       /  |  \
        //      /  (P)  \
        //     /  /   \  \
        //    / /       \ \
        //   //           \\
        //  (A)------------(C)
        //
        // Based on diagram by: Pikuma (Gustavo Pezzi)
        //
        // Points are fed in as integers but barycentric are computed using floats.
        // Therefore, the vertices can get rounded _outside_ of the triangle.
        // This can break the condition 0 <= α, β, γ <= 1 && α + β + γ = 1.
        // Users must guard against this.
        //
        // TODO: Use a floating-point raster algorithm
        ///////////////////////////////////////////////////////////////////////////////
        static glm::vec3 barycentricWeights(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c,
                                            const glm::ivec2& p) {
            const auto ac = c - a;
            const auto ab = b - a;
            const auto ap = p - a;
            const auto pc = c - p;
            const auto pb = b - p;

            /*
             * Compute the ratios of the areas of the triangles
             * The cross-product of the edges of a triangle are equal to the area of their corresponding parallelograms
             * By taking half of each, both in the numerator and denominator, the factor cancels out
             * Therefore, we only need to compute the length of the resulting cross-product:
             *      || [x0 y0 0] x [x1 y1 0] || = || [0 0 z01] || = (x0 * y1 - y0 & x1)
             * We are dealing with 2D points in the xy-plane => the cross-product is always in the z-axis direction
             *
             * It is important to be consistent regarding the winding order of the cross products so that the
             * sign of the result is positive (either +/+ or -/-, but not mixed).
             */

            // area(ABC) = || AC x AB || / 2.0f
            // Avoid the division as it cancels out in the following computations
            const glm::float32_t abcArea = ac.x * ab.y - ac.y * ab.x;

            // α = area(PBC) / area(ABC) = || PC x PB || / || AC x AB ||
            const glm::float32_t alpha = (pc.x * pb.y - pc.y * pb.x) / abcArea;

            // β = area(APC) / area(ABC) = || AC x AP || / || AC x AB ||
            const glm::float32_t beta = (ac.x * ap.y - ac.y * ap.x) / abcArea;

            // γ = 1.0f - α - β
            const glm::float32_t gamma = 1.0f - alpha - beta;

            return {alpha, beta, gamma};
        }
    };
}
