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
        glm::vec3 translation{0.0f};

        TriangleFace operator[](const std::size_t index) const {
            return {
                vertices[faces[index][0]],
                vertices[faces[index][1]],
                vertices[faces[index][2]]
            };
        }

        glm::mat4 modelTransformation() const {
            const glm::float32_t cosX = std::cos(eulerRotation.x);
            const glm::float32_t sinX = std::sin(eulerRotation.x);
            const glm::float32_t cosY = std::cos(eulerRotation.y);
            const glm::float32_t sinY = std::sin(eulerRotation.y);
            const glm::float32_t cosZ = std::cos(eulerRotation.z);
            const glm::float32_t sinZ = std::sin(eulerRotation.z);

            // The GLM memory layout is row based
            // We need to transpose the elements compared to the original rotation matrices
            const auto rotationX = glm::mat4{
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, cosX, sinX, 0.0f,
                0.0f, -sinX, cosX, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f,
            };
            const auto rotationY = glm::mat4{
                cosY, 0.0f, -sinY, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                sinY, 0.0f, cosY, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            const auto rotationZ = glm::mat4{
                cosZ, sinZ, 0.0f, 0.0f,
                -sinZ, cosZ, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f,
            };
            const auto scaleTranslate = glm::mat4{
                scale.x, 0.0f, 0.0f, 0.0f,
                0.0f, scale.y, 0.0f, 0.0f,
                0.0f, 0.0f, scale.z, 0.0f,
                translation.x, translation.y, translation.z, 1.0f
            };

            // First rotate, then translate
            // Scale is a diagonal matrix => is product commutative => can be performed in any order
            return scaleTranslate * rotationZ * rotationY * rotationX;
        }
    };
}
