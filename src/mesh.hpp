#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace rasterizer {
    struct TriangleFace {
        const std::array<glm::vec3, 3> vertices;
    };

    /*
     * The mesh faces are assumed to be:
     *  - Clockwise
     *  - Triangular
     */
    struct Mesh {
        const std::vector<glm::vec3> vertices;
        const std::vector<glm::uvec3> faces;
        glm::vec3 eulerRotation{0.0f};
        glm::vec3 scale{1.0f};

        TriangleFace operator[](const std::size_t index) const {
            return {
                vertices[faces[index][0]],
                vertices[faces[index][1]],
                vertices[faces[index][2]]
            };
        }

        glm::mat4 modelTransformation() const {
            return {
                scale.x, 0.0f, 0.0f, 0.0f,
                0.0f, scale.y, 0.0f, 0.0f,
                0.0f, 0.0f, scale.z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
        }
    };
}
