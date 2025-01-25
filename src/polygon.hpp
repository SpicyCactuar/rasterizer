#pragma once

#include <array>

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
        std::array<glm::vec2, MAX_POLYGON_VERTICES> uvs;
        std::size_t verticesAmount;

        static Polygon fromTriangle(const std::array<glm::vec3, 3>& vertices, const std::array<glm::vec2, 3>& uvs) {
            return {
                .vertices = {vertices[0], vertices[1], vertices[2]},
                .uvs = {uvs[0], uvs[1], uvs[2]},
                .verticesAmount = 3
            };
        }
    };
}
