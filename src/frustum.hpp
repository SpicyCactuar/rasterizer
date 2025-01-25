#pragma once

#include <glm/glm.hpp>

#include "polygon.hpp"

namespace rasterizer {
    enum class FrustumPlane : std::size_t {
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
        NEAR = 4,
        FAR = 5
    };

    struct Plane {
        glm::vec3 point;
        glm::vec3 normal;
    };

    struct Frustum {
        // TODO: Use std::float32_t when Clang supports it
        // https://github.com/llvm/llvm-project/issues/97335
        const glm::float32_t aspectVertical; // height / width
        const glm::float32_t aspectHorizontal; // width / height
        const glm::float32_t fovVertical;
        const glm::float32_t fovHorizontal;
        const glm::float32_t near, far; // assumed to be along positive z-axis

        glm::vec3 eye{0.0f};
        glm::float32_t yaw = 0.0f;
        glm::vec3 direction{0.0f, 0.0f, 1.0f};

        Frustum(const glm::float32_t width, const glm::float32_t height,
                const glm::float32_t fovVertical, const glm::float32_t near, const glm::float32_t far)
            : aspectVertical(height / width), aspectHorizontal(width / height),
              fovVertical(fovVertical),
              // See: https://en.wikipedia.org/wiki/Field_of_view_in_video_games
              fovHorizontal(2.0f * std::atan(std::tan(fovVertical / 2.0f) * aspectHorizontal)),
              near(near), far(far), planes(createFrustumPlanes(fovVertical, fovHorizontal, near, far)) {
        }

        glm::mat4 view(const glm::vec3& target, const glm::vec3& up) const {
            const glm::vec3 forward = glm::normalize(target - eye);
            const glm::vec3 right = glm::normalize(glm::cross(up, forward));
            const glm::vec3 upward = glm::cross(forward, right);

            return {
                right.x, upward.x, forward.x, 0.0f,
                right.y, upward.y, forward.y, 0.0f,
                right.z, upward.z, forward.z, 0.0f,
                -glm::dot(right, eye), -glm::dot(upward, eye), -glm::dot(forward, eye), 1.0f
            };
        }

        glm::mat4 perspectiveProjection() const {
            // Transpose values to account for GLM row-based memory layout
            // Negate Y component to map to downward +Y screen space
            return glm::mat4{
                aspectVertical * (1.0f / std::tan(fovVertical / 2.0f)), 0.0f, 0.0f, 0.0f,
                0.0f, -1.0f / std::tan(fovVertical / 2.0f), 0.0f, 0.0f,
                0.0f, 0.0f, far / (far - near), 1.0f,
                0.0f, 0.0f, -(far * near) / (far - near), 0.0f
            };
        }

        Polygon clipPolygon(Polygon polygon) const {
            clipAgainstPlane(polygon, FrustumPlane::LEFT);
            clipAgainstPlane(polygon, FrustumPlane::RIGHT);
            clipAgainstPlane(polygon, FrustumPlane::TOP);
            clipAgainstPlane(polygon, FrustumPlane::BOTTOM);
            clipAgainstPlane(polygon, FrustumPlane::NEAR);
            clipAgainstPlane(polygon, FrustumPlane::FAR);

            return polygon;
        }

    private:
        const std::array<Plane, 6> planes;

        /**
         * Frustum planes are defined by a point and a normal vector
         *
         * Near plane   :  P=(0, 0, znear), N=(0, 0,  1)
         * Far plane    :  P=(0, 0, zfar),  N=(0, 0, -1)
         * Top plane    :  P=(0, 0, 0),     N=(0, -cos(fov/2), sin(fov/2))
         * Bottom plane :  P=(0, 0, 0),     N=(0, cos(fov/2), sin(fov/2))
         * Left plane   :  P=(0, 0, 0),     N=(cos(fov/2), 0, sin(fov/2))
         * Right plane  :  P=(0, 0, 0),     N=(-cos(fov/2), 0, sin(fov/2))
         *
         *           /|\
         *         /  | |
         *       /\   | |
         *     /      | |
         *  P*|-->  <-|*|   ----> +z-axis
         *     \      | |
         *       \/   | |
         *         \  | |
         *           \|/
         *
         * Diagram by: Pikuma (Gustavo Pezzi)
         */
        static std::array<Plane, 6> createFrustumPlanes(const glm::float32_t fovVertical,
                                                        const glm::float32_t fovHorizontal,
                                                        const glm::float32_t near, const glm::float32_t far) {
            const auto cosHalfFovVertical = std::cos(fovVertical / 2.0f);
            const auto sinHalfFovVertical = std::sin(fovVertical / 2.0f);

            const auto cosHalfFovHorizontal = std::cos(fovHorizontal / 2.0f);
            const auto sinHalfFovHorizontal = std::sin(fovHorizontal / 2.0f);

            std::array<Plane, 6> frustumPlanes{};

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::LEFT)].point = {0.0f, 0.0f, 0.0f};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::LEFT)].normal = {
                cosHalfFovHorizontal, 0.0f, sinHalfFovHorizontal
            };

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::RIGHT)].point = {0.0f, 0.0f, 0.0f};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::RIGHT)].normal = {
                -cosHalfFovHorizontal, 0.0f, sinHalfFovHorizontal
            };

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::TOP)].point = {0.0f, 0.0f, 0.0f};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::TOP)].normal = {
                0.0f, -cosHalfFovVertical, sinHalfFovVertical
            };

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::BOTTOM)].point = {0.0f, 0.0f, 0.0f};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::BOTTOM)].normal = {
                0.0f, cosHalfFovVertical, sinHalfFovVertical
            };

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::NEAR)].point = {0.0f, 0.0f, near};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::NEAR)].normal = {0.0f, 0.0f, 1.0f};

            frustumPlanes[static_cast<std::size_t>(FrustumPlane::FAR)].point = {0.0f, 0.0f, far};
            frustumPlanes[static_cast<std::size_t>(FrustumPlane::FAR)].normal = {0.0f, 0.0f, -1.0f};

            return frustumPlanes;
        }

        void clipAgainstPlane(Polygon& polygon, const FrustumPlane plane) const {
            const auto [planePoint, planeNormal] = planes[static_cast<std::size_t>(plane)];

            std::array<glm::vec3, Polygon::MAX_POLYGON_VERTICES> insideVertices{};
            std::array<glm::vec2, Polygon::MAX_POLYGON_VERTICES> insideUvs{};
            std::size_t insideAmount = 0;

            // TODO: Refactor to use std::reference_wrapper
            const auto* previousVertex = &polygon.vertices[polygon.verticesAmount - 1];
            const auto* previousUv = &polygon.uvs[polygon.verticesAmount - 1];

            const auto* currentVertex = &polygon.vertices[0];
            const auto* currentUv = &polygon.uvs[0];

            glm::float32_t previousDot = glm::dot(*previousVertex - planePoint, planeNormal);
            glm::float32_t currentDot = 0.0f;

            // TODO: Loop using indices
            // Loop until end position (last index polygon.verticesAmount - 1 => end is at polygon.verticesAmount)
            while (currentVertex != &polygon.vertices[polygon.verticesAmount]) {
                currentDot = glm::dot(*currentVertex - planePoint, planeNormal);

                // If going from outside to inside, or viceversa, add intersection to inside
                if (currentDot * previousDot < 0.0f) {
                    // Compute intersection point I = Qp + t (Qc - Qp)
                    const glm::float32_t t = previousDot / (previousDot - currentDot);
                    insideVertices[insideAmount] = glm::mix(*previousVertex, *currentVertex, t);
                    insideUvs[insideAmount] = glm::mix(*previousUv, *currentUv, t);
                    insideAmount++;
                }

                // If current point is inside the plane, add it to inside
                if (currentDot > 0.0f) {
                    insideVertices[insideAmount] = *currentVertex;
                    insideUvs[insideAmount] = *currentUv;
                    insideAmount++;
                }

                // Move to the next vertex
                previousDot = currentDot;
                previousVertex = currentVertex;
                previousUv = currentUv;
                currentVertex++;
                currentUv++;
            }

            polygon.vertices = insideVertices;
            polygon.uvs = insideUvs;
            polygon.verticesAmount = insideAmount;
        }
    };
}
