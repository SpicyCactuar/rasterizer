#pragma once

#include <span>
#include <vector>
#include <SDL2/SDL.h>

namespace rasterizer {
    class Scene {
    public:
        Scene() = default;

        void emplaceRectangle(const std::uint32_t positionX, const std::uint32_t positionY,
                              const std::uint32_t width, const std::uint32_t height) {
            rects.emplace_back(positionX, positionY, width, height);
        }

        std::span<const SDL_Rect> rectangles() const {
            return {rects.data(), rects.size()};
        }

    private:
        std::vector<SDL_Rect> rects;
    };
}
