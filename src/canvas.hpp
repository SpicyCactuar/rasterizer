#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "common.hpp"

namespace rasterizer {
    typedef std::uint32_t color_t;

    enum class PolygonMode : std::uint32_t {
        FILL = 1 << 0, // 0b0001
        LINE = 1 << 1, // 0b0010
        POINT = 1 << 2 // 0b0100
    };

    static constexpr color_t triangleFillColor = 0xFF4C1D95;

    struct Triangle {
        const std::array<glm::ivec2, 3> vertices;
        const glm::float32_t averageDepth;
        const color_t color = triangleFillColor;
    };

    class Canvas {
    public:
        const std::uint32_t width, height;

        Canvas(const std::uint32_t width, const std::uint32_t height) : width(width),
                                                                        height(height) {
            buffer = createFramebuffer(width, height);

            if (buffer == nullptr) {
                throw std::runtime_error("Failed to create Canvas");
            }
        }

        ~Canvas() {
            if (buffer != nullptr) {
                std::free(buffer);
                buffer = nullptr;
            }
        }

        explicit operator const std::uint32_t*() const {
            return buffer;
        }

        void drawPixel(const std::int32_t row, const std::int32_t column, const color_t color) const {
            if (0 <= row && row < height && 0 <= column && column < width) {
                buffer[row * width + column] = color;
            }
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
            const auto& [p0, p1, p2] = triangle.vertices;
            static constexpr color_t triangleLineColor = 0xFFA78BFA;
            static constexpr color_t trianglePointColor = 0xFF7C3AED;

            const bool drawTriangleFill = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::FILL);
            const bool drawTriangleLines = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::LINE);
            const bool drawTrianglePoints = polygonModeMask & static_cast<std::uint32_t>(PolygonMode::POINT);

            if (drawTriangleFill) {
                drawFilledTriangle(p0, p1, p2, triangle.color);
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
        void drawFilledTriangle(glm::ivec2 p0, glm::ivec2 p1, glm::ivec2 p2, const color_t color) const {
            sortAscendingVertically(p0, p1, p2);

            if (p1.y == p2.y) {
                // Triangle is flat bottom
                drawFlatBottomTriangle(p0, p1, p2, color);
            } else if (p0.y == p1.y) {
                // Triangle is flat top
                drawFlatTopTriangle(p0, p1, p2, color);
            } else {
                // Solution based on similar triangles
                const glm::ivec2 mid{
                    (p2.x - p0.x) * (p1.y - p0.y) / (p2.y - p0.y) + p0.x,
                    p1.y
                };

                // Draw flat-bottom triangle
                drawFlatBottomTriangle(p0, p1, mid, color);

                // Draw flat-top triangle
                drawFlatTopTriangle(p1, mid, p2, color);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////
        //
        //           p0
        //          /  \
        //         /    \
        //        /      \
        //       /        \
        //      /          \
        //     p1 -------- p2
        //
        // Based on diagram by: Pikuma (Gustavo Pezzi)
        //
        ///////////////////////////////////////////////////////////////////////////////
        void drawFlatBottomTriangle(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2,
                                    const color_t color) const {
            // Find the two slopes (two triangle legs)
            // Inverse is taken as y becomes independent axis
            const auto invSlope01 = static_cast<glm::float32_t>(p1.x - p0.x) / static_cast<glm::float32_t>(p1.y - p0.y);
            const auto invSlope02 = static_cast<glm::float32_t>(p2.x - p0.x) / static_cast<glm::float32_t>(p2.y - p0.y);

            // Start at the top vertex p0
            auto startX = static_cast<glm::float32_t>(p0.x);
            auto endX = static_cast<glm::float32_t>(p0.x);

            // Loop all the scanlines from top to bottom
            // Since Y+ is downward, we increment Y on each iteration
            for (auto y = p0.y; y <= p2.y; ++y) {
                drawLine({startX, y}, {endX, y}, color);
                startX += invSlope01;
                endX += invSlope02;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////
        //
        //     p0 -------- p1
        //      \          /
        //       \        /
        //        \      /
        //         \    /
        //          \  /
        //           p2
        //
        // Based on diagram by: Pikuma (Gustavo Pezzi)
        //
        ///////////////////////////////////////////////////////////////////////////////
        void drawFlatTopTriangle(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2,
                                 const color_t color) const {
            // Find the two slopes (two triangle legs)
            // Inverse is taken as y becomes independent axis
            const auto invSlope20 = static_cast<glm::float32_t>(p0.x - p2.x) / static_cast<glm::float32_t>(p0.y - p2.y);
            const auto invSlope21 = static_cast<glm::float32_t>(p1.x - p2.x) / static_cast<glm::float32_t>(p1.y - p2.y);

            // Start at the top vertex p0
            auto startX = static_cast<glm::float32_t>(p2.x);
            auto endX = static_cast<glm::float32_t>(p2.x);

            // Loop all the scanlines from top to bottom
            // Since Y- is upward, we decrement Y on each iteration
            for (auto y = p2.y; y >= p0.y; --y) {
                drawLine({startX, y}, {endX, y}, color);
                startX -= invSlope20;
                endX -= invSlope21;
            }
        }

        void drawGrid() const {
            static constexpr std::uint32_t gridColor = 0xFF7C3AED;

            for (std::uint32_t row = 0; row < height; row += 10) {
                for (std::uint32_t column = 0; column < width; column += 10) {
                    drawPixel(row, column, gridColor);
                }
            }
        }

        void clear() const {
            // Clear framebuffer contents
            static constexpr std::uint32_t clearColor = 0xFF2E1065;

            for (std::uint32_t row = 0; row < height; ++row) {
                for (std::uint32_t column = 0; column < width; ++column) {
                    drawPixel(row, column, clearColor);
                }
            }
        }

        void enable(const PolygonMode mode) {
            polygonModeMask |= static_cast<std::uint32_t>(mode);
        }

        void disable(const PolygonMode mode) {
            polygonModeMask &= ~static_cast<std::uint32_t>(mode);
        }

    private:
        /*
         * Intentionally handled in a C-like manner for learning purposes.
         * The C++ approach I would use is std::array<std::array<std::uint32_t, width>, height>
         */
        std::uint32_t* buffer = nullptr;

        std::uint32_t polygonModeMask = static_cast<std::uint32_t>(PolygonMode::LINE);

        static std::uint32_t* createFramebuffer(const std::uint32_t width, const std::uint32_t height) {
            return static_cast<std::uint32_t*>(std::calloc(width * height, sizeof(std::uint32_t)));
        }

        static void sortAscendingVertically(glm::ivec2& p0, glm::ivec2& p1, glm::ivec2& p2) {
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
            }
            if (p2.y < p1.y) {
                // p2.y < p1.y => p1.y < p2.y
                std::swap(p1, p2);
            }
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
            }
        }
    };
}
