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

    inline glm::float32_t dot(const glm::vec3& a, const glm::vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline glm::vec3 cross(const glm::vec3& a, const glm::vec3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
}
