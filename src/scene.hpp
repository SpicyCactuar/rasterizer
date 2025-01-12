#pragma once

#include <vector>

#include "mesh.hpp"
#include "light.hpp"
#include "texture.hpp"

namespace rasterizer {
    class Scene {
    public:
        std::vector<Mesh> meshes;
        DirectionalLight light{{0.0f, 0.0f, 1.0f}};

        Surface* cubeSurface = nullptr;
        Surface* brickSurface = nullptr;

        explicit Scene(std::vector<Mesh> meshes) : meshes(std::move(meshes)) {
            cubeSurface = loadPngSurface("../assets/cube.png");
            if (cubeSurface == nullptr) {
                throw std::runtime_error("Failed to load cube surface");
            }

            brickSurface = loadDataSurface(reinterpret_cast<const std::uint32_t*>(brickData.data()),
                                           brickWidth, brickHeight);
            if (brickSurface == nullptr) {
                throw std::runtime_error("Failed to load brick surface");
            }
        }

        void lock() const {
            brickSurface->lock();
            cubeSurface->lock();
        }

        void unlock() const {
            cubeSurface->unlock();
            brickSurface->unlock();
        }

        ~Scene() {
            if (cubeSurface != nullptr) {
                delete cubeSurface;
                cubeSurface = nullptr;
            }
            if (brickSurface != nullptr) {
                delete brickSurface;
                brickSurface = nullptr;
            }
        }
    };
}
