#pragma once

#include <glm/glm.hpp>

#include "canvas.hpp"

namespace rasterizer {
    class DirectionalLight {
    public:
        const glm::vec3 direction;

        color_t modulateSurfaceColor(const color_t color, const glm::vec3& normal) const {
            const glm::float32_t attenuation = std::clamp(glm::dot(-this->direction, normal), 0.0f, 1.0f);

            const color_t a = color & 0xFF000000;
            const color_t r = (color & 0x00FF0000) * attenuation;
            const color_t g = (color & 0x0000FF00) * attenuation;
            const color_t b = (color & 0x000000FF) * attenuation;

            return a | r & 0x00FF0000 | g & 0x0000FF00 | b & 0x000000FF;
        }
    };
}
