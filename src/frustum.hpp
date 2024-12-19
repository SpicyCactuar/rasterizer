#pragma once

namespace rasterizer {
    struct Frustum {
        const glm::float32 fov = 640;
        const glm::vec3 position{0.0f, 0.0f, -5.0f};

        glm::vec2 perspectiveDivide(glm::vec3 point) const {
            point += position;
            return {
                point.x * fov / point.z,
                point.y * fov / point.z
            };
        }
    };
}
