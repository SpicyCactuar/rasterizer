#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace rasterizer {
    struct Triangle {
        const std::array<glm::vec3, 3> vertices;
    };

    /*
     * Meshes are:
     *  - 1-indexed based, as per OBJ standard
     *  - Clockwise
     *  - Triangular
     */
    struct Mesh {
        const std::vector<glm::vec3> vertices;
        const std::vector<glm::uvec3> faces;
        glm::vec3 eulerRotation{0.0f};

        Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> indices)
            : vertices(std::move(vertices)),
              faces(std::move(indices)) {
            assert(indices.size() % 3 == 0);
        }

        Triangle operator[](const std::size_t index) const {
            return {
                vertices[faces[index][0] - 1],
                vertices[faces[index][1] - 1],
                vertices[faces[index][2] - 1]
            };
        }
    };

    static const auto cubeMesh = Mesh{
        {
            {-1.0f, -1.0f, -1.0f}, // 1
            {-1.0f, 1.0f, -1.0f}, // 2
            {1.0f, 1.0f, -1.0f}, // 3
            {1.0f, -1.0f, -1.0f}, // 4
            {1.0f, 1.0f, 1.0f}, // 5
            {1.0f, -1.0f, 1.0f}, // 6
            {-1.0f, 1.0f, 1.0f}, // 7
            {-1.0f, -1.0f, 1.0f} // 8
        },
        {
            // front
            {1, 2, 3},
            {1, 3, 4},
            // right
            {4, 3, 5},
            {4, 5, 6},
            // back
            {6, 5, 7},
            {6, 7, 8},
            // left
            {8, 7, 2},
            {8, 2, 1},
            // top
            {2, 7, 5},
            {2, 5, 3},
            // bottom
            {6, 8, 1},
            {6, 1, 4}
        }
    };
}
