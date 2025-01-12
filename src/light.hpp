#pragma once

#include <glm/glm.hpp>

#include "canvas.hpp"

namespace rasterizer {
    struct DirectionalLight {
        const glm::vec3 direction;

        color_t modulateSurfaceColor(const color_t color, const glm::vec3& normal) const {
            // Use reversed light direction to correctly compute dot from eye's perspective
            const glm::float32_t attenuation = std::clamp(glm::dot(-this->direction, normal), 0.0f, 1.0f);

            const color_t r = (color & 0xFF000000) * attenuation;
            const color_t g = (color & 0x00FF0000) * attenuation;
            const color_t b = (color & 0x0000FF00) * attenuation;
            const color_t a = color & 0x000000FF;

            return r & 0xFF000000 | g & 0x00FF0000 | b & 0x0000FF00 | a & 0x000000FF;
        }
    };
}
