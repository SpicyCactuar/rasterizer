#pragma once

#include <cstdint>

#include <glm/glm.hpp>

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
                           const std::uint32_t width, const std::uint32_t height) const {
            static constexpr std::uint32_t rectangleColor = 0xFF7C3AED;

            const std::uint32_t endX = std::min(x + width, this->width);
            const std::uint32_t endY = std::min(y + height, this->height);

            for (std::uint32_t row = y; row < endY; ++row) {
                for (std::uint32_t column = x; column < endX; ++column) {
                    drawPixel(row, column, rectangleColor);
                }
            }
        }

        void drawPoint(const glm::vec2& point) const {
            static constexpr std::uint32_t pointWidth = 10, pointHeight = 10;
            // Draw centered, with side length 10
            drawRectangle(static_cast<std::uint32_t>(point.x) - pointWidth / 2,
                          static_cast<std::uint32_t>(point.y) - pointHeight / 2,
                          pointWidth, pointHeight);
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
                drawPixel(static_cast<std::uint32_t>(y), static_cast<std::uint32_t>(x), lineColor);
                x += xIncrement;
                y += yIncrement;
            }
        }

        void drawTriangle(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2) const {
            drawPoint(p0);
            drawPoint(p1);
            drawPoint(p2);
            drawLine(p0, p1);
            drawLine(p0, p2);
            drawLine(p1, p2);
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
    };
}
