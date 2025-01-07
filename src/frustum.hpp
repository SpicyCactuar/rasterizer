#pragma once

#include <glm/glm.hpp>

namespace rasterizer {
    struct Frustum {
        // TODO: Use std::float32 when Clang supports it
        // https://github.com/llvm/llvm-project/issues/97335
        const glm::float32_t aspect; // aspect = height / width
        const glm::float32_t fov;
        const glm::float32_t near, far; // assumed to be along z-axis
        const glm::vec3 position{0.0f, 0.0f, 0.0f};

        Frustum(const glm::float32_t aspect, const glm::float32_t fov,
                const glm::float32_t near, const glm::float32_t far)
            : aspect(aspect), fov(fov), near(near), far(far) {
        }

        glm::mat4 perspectiveProjection() const {
            // Transpose values to account for GLM row-based memory layout
            // Negate Y component to map to downward +Y screen space
            return glm::mat4{
                aspect * (1.0f / std::tan(fov / 2.0f)), 0.0f, 0.0f, 0.0f,
                0.0f, -1.0f / std::tan(fov / 2.0f), 0.0f, 0.0f,
                0.0f, 0.0f, far / (far - near), 1.0f,
                0.0f, 0.0f, -(far * near) / (far - near), 0.0f
            };
        }
    };
}
