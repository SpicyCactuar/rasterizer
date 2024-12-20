#pragma once

#include <glm/glm.hpp>

namespace rasterizer {
    struct Frustum {
        // TODO: Use std::float32 when Clang supports it
        // https://github.com/llvm/llvm-project/issues/97335
        const glm::float32 fov = 640;
        const glm::vec3 position{0.0f, 0.0f, -5.0f};

        glm::vec2 perspectiveDivide(const glm::vec3& point) const {
            return {
                point.x * fov / point.z,
                point.y * fov / point.z
            };
        }
    };
}
