#pragma once

#include <glm/glm.hpp>

namespace rasterizer {
    inline glm::vec3 rotateAroundX(const glm::vec3& point, const glm::float32 angleInDegrees) {
        return {
            point.x,
            point.y * glm::cos(angleInDegrees) - point.z * glm::sin(angleInDegrees),
            point.y * glm::sin(angleInDegrees) + point.z * glm::cos(angleInDegrees),
        };
    }

    inline glm::vec3 rotateAroundY(const glm::vec3& point, const glm::float32 angleInDegrees) {
        return {
            point.x * glm::cos(angleInDegrees) - point.z * glm::sin(angleInDegrees),
            point.y,
            point.x * glm::sin(angleInDegrees) + point.z * glm::cos(angleInDegrees),
        };
    }

    inline glm::vec3 rotateAroundZ(const glm::vec3& point, const glm::float32 angleInDegrees) {
        return {
            point.x * glm::cos(angleInDegrees) - point.y * glm::sin(angleInDegrees),
            point.x * glm::sin(angleInDegrees) + point.y * glm::cos(angleInDegrees),
            point.z
        };
    }
}
