#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "texture.hpp"

namespace rasterizer {
    struct TriangleFace {
        const std::array<glm::vec3, 3> vertices;
        const std::array<rasterizer::uv, 3> uvs;
    };

    /*
     * The mesh faces are assumed to be:
     *  - Clockwise
     *  - Triangular
     */
    struct Mesh {
        const std::vector<glm::vec3> vertices;
        const std::vector<rasterizer::uv> uvs;
        const std::vector<std::uint32_t> faceIndices;
        const std::vector<std::uint32_t> uvIndices;
        glm::vec3 eulerRotation{0.0f};
        glm::vec3 scale{1.0f};
        glm::vec3 translation{0.0f};

        size_t facesAmount() const {
            return faceIndices.size() / 3;
        }

        // TODO: Allow for per-vertex optional uv texturing, right now if there is a mix this will assign incorrectly
        TriangleFace operator[](const std::size_t index) const {
            const size_t fi = 3 * index;

            const auto v0 = faceIndices[fi];
            const auto v1 = faceIndices[fi + 1];
            const auto v2 = faceIndices[fi + 2];

            const auto vt0 = uvIndices[fi];
            const auto vt1 = uvIndices[fi + 1];
            const auto vt2 = uvIndices[fi + 2];

            return {
                .vertices = {
                    vertices[v0],
                    vertices[v1],
                    vertices[v2]
                },
                .uvs = {
                    uvs[vt0],
                    uvs[vt1],
                    uvs[vt2],
                }
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
