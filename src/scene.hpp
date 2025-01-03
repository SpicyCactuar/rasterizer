#pragma once

#include <vector>

#include "frustum.hpp"
#include "mesh.hpp"

namespace rasterizer {
    class Scene {
    public:
        const Frustum frustum;
        std::vector<Mesh> meshes;

        Scene(std::vector<Mesh> meshes) : meshes(std::move(meshes)) {
        }
    };
}
