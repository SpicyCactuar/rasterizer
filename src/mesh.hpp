#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace rasterizer {
    struct Triangle {
        const std::array<glm::vec3, 3> vertices;
    };

    /*
     * The mesh faces are:
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
                vertices[faces[index][0]],
                vertices[faces[index][1]],
                vertices[faces[index][2]]
            };
        }
    };
}
