#pragma once

#include <memory>
#include <vector>

#include "mesh.hpp"
#include "light.hpp"
#include "obj.hpp"
#include "texture.hpp"

namespace rasterizer {
    class Scene {
    public:
        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<Surface>> meshSurfaces;
        DirectionalLight light{{0.0f, -1.0f, 0.0f}};

        explicit Scene()
            : meshes({
                  rasterizer::parseObj("../assets/mesh/runway.obj"),
                  rasterizer::parseObj("../assets/mesh/f22.obj"),
                  rasterizer::parseObj("../assets/mesh/efa.obj"),
                  rasterizer::parseObj("../assets/mesh/f117.obj"),
              }),
              surfaces({
                  std::shared_ptr<Surface>(rasterizer::loadPngSurface("../assets/mesh/runway.png")),
                  std::shared_ptr<Surface>(rasterizer::loadPngSurface("../assets/mesh/f22.png")),
                  std::shared_ptr<Surface>(rasterizer::loadPngSurface("../assets/mesh/efa.png")),
                  std::shared_ptr<Surface>(rasterizer::loadPngSurface("../assets/mesh/f117.png"))
              }) {
            meshSurfaces = {
                surfaces[0],
                surfaces[1],
                surfaces[2],
                surfaces[3]
            };

            meshes[0].translation = {0.0f, -1.5f, 23.0f};
            meshes[1].translation = {0.0f, -1.3f, 5.0f};
            meshes[2].translation = {-2.0f, -1.3f, 9.0f};
            meshes[3].translation = {2.0f, -1.3f, 9.0f};

            meshes[0].rotation = {0.0f, 0.0f, 0.0f};
            meshes[1].rotation = {0.0f, -std::numbers::pi / 2.0f, 0.0f};
            meshes[2].rotation = {0.0f, -std::numbers::pi / 2.0f, 0.0f};
            meshes[3].rotation = {0.0f, -std::numbers::pi / 2.0f, 0.0f};
        }

        void lock() const {
            for (const auto& surface : surfaces) {
                surface->lock();
            }
        }

        void unlock() const {
            for (const auto& surface : surfaces) {
                surface->unlock();
            }
        }

    private:
        std::vector<std::shared_ptr<Surface>> surfaces;
    };
}
