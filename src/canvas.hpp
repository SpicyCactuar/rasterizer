#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "common.hpp"

namespace rasterizer {
    class Canvas {
    public:
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

        const std::uint32_t width, height;

        explicit operator const std::uint32_t*() const {
            return buffer;
        }

        void drawPixel(const std::uint32_t row, const std::uint32_t column, const std::uint32_t& color) const {
            if (row < height && column < width) {
                buffer[row * width + column] = color;
            }
        }

        void drawRectangle(const std::uint32_t x, const std::uint32_t y,
                           const std::uint32_t width, const std::uint32_t height,
                           const std::uint32_t color) const {
            const std::uint32_t endX = std::min(x + width, this->width);
            const std::uint32_t endY = std::min(y + height, this->height);

            for (std::uint32_t row = y; row < endY; ++row) {
                for (std::uint32_t column = x; column < endX; ++column) {
                    drawPixel(row, column, color);
                }
            }
        }

        void drawPoint(const glm::vec2& point) const {
            static constexpr std::uint32_t rectangleColor = 0xFF7C3AED;
            static constexpr std::uint32_t pointWidth = 10, pointHeight = 10;
            // Draw centered, with side length 10
            drawRectangle(static_cast<std::uint32_t>(point.x) - pointWidth / 2,
                          static_cast<std::uint32_t>(point.y) - pointHeight / 2,
                          pointWidth, pointHeight, rectangleColor);
        }

        void drawLine(const glm::vec2& start, const glm::vec2& end, const std::uint32_t lineColor) const {
            // DDA line rasterizer
            const std::int32_t dx = static_cast<std::int32_t>(end.x) - static_cast<std::int32_t>(start.x);
            const std::int32_t dy = static_cast<std::int32_t>(end.y) - static_cast<std::int32_t>(start.y);

            const std::uint32_t longestLength = std::abs(dx) >= std::abs(dy) ? std::abs(dx) : abs(dy);

            const auto df = static_cast<glm::float32_t>(longestLength);
            const glm::float32_t xIncrement = static_cast<glm::float32_t>(dx) / df;
            const glm::float32_t yIncrement = static_cast<glm::float32_t>(dy) / df;

            glm::float32_t x = start.x;
            glm::float32_t y = start.y;
            for (std::uint32_t l = 0; l < longestLength; l += 1) {
                drawPixel(static_cast<std::uint32_t>(y), static_cast<std::uint32_t>(x), lineColor);
                x += xIncrement;
                y += yIncrement;
            }
        }

        void drawTriangle(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2) const {
            static constexpr std::uint32_t triangleLineColor = 0xFFA78BFA;
            static constexpr std::uint32_t triangleFillColor = 0xFF4C1D95;
            drawFilledTriangle(p0, p1, p2, triangleFillColor);

            if constexpr (isDebugMode()) {
                drawPoint(p0);
                drawPoint(p1);
                drawPoint(p2);
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
        void drawFilledTriangle(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, const std::uint32_t color) const {
            sortAscendingVertically(p0, p1, p2);

            // Solution based on similar triangles
            const glm::vec2 mid{
                (p2.x - p0.x) * (p1.y - p0.y) / (p2.y - p0.y) + p0.x,
                p1.y
            };

            // Draw flat-bottom triangle
            drawFlatBottomTriangle(p0, p1, mid, color);

            // Draw flat-top triangle
            drawFlatTopTriangle(p1, mid, p2, color);

            // Draw mid point on top
            if constexpr (isDebugMode()) {
                drawPoint(mid);
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
        void drawFlatBottomTriangle(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2,
                                    const std::uint32_t color) const {
            // Find the two slopes (two triangle legs)
            // Inverse is taken as y becomes independent axis
            const glm::float32_t invSlope01 = (p1.x - p0.x) / (p1.y - p0.y);
            const glm::float32_t invSlope02 = (p2.x - p0.x) / (p2.y - p0.y);

            // Start at the top vertex p0
            glm::float32_t startX = p0.x;
            glm::float32_t endX = p0.x;

            // Loop all the scanlines from top to bottom
            // Since Y+ is downward, we increment Y on each iteration
            for (glm::float32_t y = p0.y; y <= p2.y; ++y) {
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
        void drawFlatTopTriangle(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2,
                                 const std::uint32_t color) const {
            // Find the two slopes (two triangle legs)
            // Inverse is taken as y becomes independent axis
            const glm::float32_t invSlope20 = (p0.x - p2.x) / (p0.y - p2.y);
            const glm::float32_t invSlope21 = (p1.x - p2.x) / (p1.y - p2.y);

            // Start at the top vertex p0
            glm::float32_t startX = p2.x;
            glm::float32_t endX = p2.x;

            // Loop all the scanlines from top to bottom
            // Since Y- is upward, we decrement Y on each iteration
            for (glm::float32_t y = p2.y; y >= p0.y; --y) {
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

    private:
        /*
         * Intentionally handled in a C-like manner for learning purposes.
         * The C++ approach I would use is std::array<std::array<std::uint32_t, width>, height>
         */
        std::uint32_t* buffer = nullptr;

        static std::uint32_t* createFramebuffer(const std::uint32_t width, const std::uint32_t height) {
            return static_cast<std::uint32_t*>(std::calloc(width * height, sizeof(std::uint32_t)));
        }

        static void sortAscendingVertically(glm::vec2& p0, glm::vec2& p1, glm::vec2& p2) {
            if (p1.y < p0.y) {
                // p1.y < p0.y => p0.y < p1.y
                std::swap(p0, p1);
            }

            if (p2.y < p0.y) {
                // p2.y < p0.y < p1.y => p0.y < p1.y < p2.y
                std::swap(p0, p2);
                std::swap(p1, p2);
            }

            if (p2.y < p1.y) {
                // p0.y < p2.y < p1.y => p0.y < p1.y < p2.y
                std::swap(p1, p2);
            }
        }
    };
}
