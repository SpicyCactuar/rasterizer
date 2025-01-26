#pragma once

#include <array>
#include <tuple>

namespace rasterizer {
    struct Triangle {
        static constexpr color_t defaultSolidColor = 0x4C1D95FF;

        const std::array<glm::vec4, 3> vertices;
        const std::array<glm::vec2, 3> uvs;
        const color_t solidColor = defaultSolidColor;
        const Surface* surface = nullptr;
    };

    glm::vec3 computeNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        return glm::normalize(glm::cross(v1 - v0, v2 - v0));
    }

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

        std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec2, glm::vec2, glm::vec2> operator[](
            const std::size_t index) const {
            // Resulting polygon might have less than 2 vertices that were not clipped, no triangles
            if (index >= verticesAmount - 2) {
                return {};
            }

            // We extract triangle fan center at index 0, therefore:
            // facesAmount == polygon.verticesAmount - 2
            // Assumes polygon has at least 3 vertices (a non-clipped triangle)
            return {
                vertices[0],
                vertices[index + 1],
                vertices[index + 2],
                uvs[0],
                uvs[index + 1],
                uvs[index + 2],
            };
        }

        std::size_t trianglesAmount() const {
            if (verticesAmount <= 2) {
                return 0;
            }

            return verticesAmount - 2;
        }
    };
}
