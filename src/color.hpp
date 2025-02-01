#pragma once

namespace rasterizer {
    typedef std::uint32_t color_t;
    static constexpr auto colorFormat = SDL_PIXELFORMAT_RGBA8888;

    inline color_t randomColor(const size_t seed) {
        // Golden ratio color
        const color_t colorSeed = seed * 2654435761u;
        constexpr color_t a = 0x000000FF;
        const color_t r = colorSeed & 0xFF000000; // 0xRR000000
        const color_t g = colorSeed & 0x00FF0000; // 0x00GG0000
        const color_t b = colorSeed & 0x0000FF00; // 0x0000BB00

        return r | g | b | a;
    }

    inline color_t interpolateColor(const glm::vec3& weights, const std::array<color_t, 3>& colors) {
        const color_t r = static_cast<color_t>(
            weights.x * (colors[0] & 0xFF000000) +
            weights.y * (colors[1] & 0xFF000000) +
            weights.z * (colors[2] & 0xFF000000)
        );

        const color_t g = static_cast<color_t>(
            weights.x * (colors[0] & 0x00FF0000) +
            weights.y * (colors[1] & 0x00FF0000) +
            weights.z * (colors[2] & 0x00FF0000)
        );

        const color_t b = static_cast<color_t>(
            weights.x * (colors[0] & 0x0000FF00) +
            weights.y * (colors[1] & 0x0000FF00) +
            weights.z * (colors[2] & 0x0000FF00)
        );

        return r & 0xFF000000 | g & 0x00FF0000 | b & 0x0000FF00 | 0x000000FF;
    }
}
