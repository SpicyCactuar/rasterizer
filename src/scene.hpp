#pragma once

#include <memory>
#include <vector>

#include "mesh.hpp"
#include "light.hpp"
#include "texture.hpp"

namespace rasterizer {
    class Scene {
    public:
        std::vector<Mesh> meshes;
        DirectionalLight light{{0.0f, 0.0f, 1.0f}};

        std::unique_ptr<Surface> cubeSurface;
        std::unique_ptr<Surface> brickSurface;
        std::unique_ptr<Surface> meshSurface;

        explicit Scene(std::vector<Mesh> meshes) : meshes(std::move(meshes)) {
            Surface* rawCubeSurface = loadPngSurface("../assets/cube.png");
            if (rawCubeSurface == nullptr) {
                throw std::runtime_error("Failed to load cube surface");
            }
            cubeSurface.reset(rawCubeSurface);

            Surface* rawBrickSurface = loadDataSurface(reinterpret_cast<const std::uint32_t*>(brickData.data()),
                                                       brickWidth, brickHeight);
            if (rawBrickSurface == nullptr) {
                throw std::runtime_error("Failed to load brick surface");
            }
            brickSurface.reset(rawBrickSurface);

            Surface* rawMeshSurface = loadPngSurface("../assets/f22.png");
            if (rawMeshSurface == nullptr) {
                throw std::runtime_error("Failed to load mesh surface");
            }
            meshSurface.reset(rawMeshSurface);
        }

        void lock() const {
            brickSurface->lock();
            cubeSurface->lock();
            meshSurface->lock();
        }

        void unlock() const {
            cubeSurface->unlock();
            brickSurface->unlock();
            meshSurface->unlock();
        }
    };
}
