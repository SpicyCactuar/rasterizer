#pragma once

#include <vector>

#include "mesh.hpp"
#include "light.hpp"

namespace rasterizer {
    class Scene {
    public:
        std::vector<Mesh> meshes;
        DirectionalLight light{{0.0f, 0.0f, 1.0f}};

        Scene(std::vector<Mesh> meshes) : meshes(std::move(meshes)) {
        }
    };
}
