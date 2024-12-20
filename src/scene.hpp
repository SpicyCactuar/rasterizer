#pragma once

#include <vector>

#include "frustum.hpp"
#include "mesh.hpp"

namespace rasterizer {
    class Scene {
    public:
        const Frustum frustum;
        std::vector<Mesh> meshes;

        Scene() {
            meshes.emplace_back(cubeMesh);
        }
    };
}
