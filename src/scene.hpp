#pragma once

#include <vector>
#include <span>

#include <glm/glm.hpp>

namespace rasterizer {
    class Scene {
    public:
        const glm::float32 fov = 120;

        Scene() {
            // TODO: Use std::float32 when Clang supports it
            // https://github.com/llvm/llvm-project/issues/97335
            std::size_t pointCount = 0;
            for (glm::float32 x = -1.0f; x <= 1.0f; x += 0.25f) {
                for (glm::float32 y = -1.0f; y <= 1.0f; y += 0.25f) {
                    cube[pointCount] = {x, y, 1.0f};
                    ++pointCount;
                }
            }
        }

        std::span<const glm::vec3> cubePoints() const {
            return {cube.data(), cube.size()};
        }

    private:
        std::array<glm::vec3, 9 * 9> cube{};
    };
}
