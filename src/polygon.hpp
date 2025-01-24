#pragma once

namespace rasterizer {
    struct Triangle {
        static constexpr color_t defaultSolidColor = 0x4C1D95FF;

        const std::array<glm::vec4, 3> vertices;
        const std::array<glm::vec2, 3> uvs;
        const color_t solidColor = defaultSolidColor;
        const Surface* surface = nullptr;
    };

    struct Polygon {
        static constexpr std::size_t MAX_POLYGON_VERTICES = 10;

        std::array<glm::vec3, MAX_POLYGON_VERTICES> vertices;
        std::size_t verticesAmount;

        static Polygon fromTriangle(const glm::vec3 v0, const glm::vec3 v1, const glm::vec3 v2) {
            return {
                .vertices = {v0, v1, v2},
                .verticesAmount = 3
            };
        }
    };
}
