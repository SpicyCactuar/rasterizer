#pragma once

#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "mesh.hpp"

namespace {
    bool parseFace(const std::string& line,
                   std::vector<std::uint32_t>& faceIndices, std::vector<std::uint32_t>& uvIndices) {
        // Discard 'f '
        std::stringstream ss(line.substr(2));
        std::string faceData;

        // Need to parse 3 triplets of either form:
        // v1 v2 v3 | v1/vt1 v2/vt2 v3/vt3 | v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 | v1//vn1 v2//vn2 v3//vn3
        for (std::uint32_t f = 0; ss >> faceData && f < 3; f++) {
            std::uint32_t v = 0, vt = 0, vn = 0;

            const char* faceData_c = faceData.c_str();
            // Indices are mapped 1-indexed -> 0-indexed
            if (std::sscanf(faceData_c, "%u/%u/%u", &v, &vt, &vn) == 3) {
                faceIndices.emplace_back(v - 1);
                uvIndices.emplace_back(vt - 1);
            } else if (std::sscanf(faceData_c, "%u//%u", &v, &vn) == 2) {
                faceIndices.emplace_back(v - 1);
            } else if (std::sscanf(faceData_c, "%u/%u", &v, &vt) == 2) {
                faceIndices.emplace_back(v - 1);
                uvIndices.emplace_back(vt - 1);
            } else if (std::sscanf(faceData_c, "%u", &v) == 1) {
                faceIndices.emplace_back(v - 1);
            } else {
                return false;
            }
        }

        return true;
    }
}

namespace rasterizer {
    inline Mesh parseObj(const std::filesystem::path& objPath) {
        std::ifstream file(objPath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + objPath.string());
        }

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<std::uint32_t> faceIndices;
        std::vector<std::uint32_t> uvIndices;

        std::string line;
        for (std::uint32_t lineCount = 0; std::getline(file, line); ++lineCount) {
            if (line.empty() || line.starts_with('#')) {
                // Skip empty lines and comments
                continue;
            }

            if (line.starts_with("v ")) {
                if (glm::vec3 vertex;
                    std::sscanf(line.c_str(), "v %f %f %f", &vertex[0], &vertex[1], &vertex[2]) == 3) {
                    vertices.emplace_back(vertex);
                } else {
                    std::print(std::cerr, "Failed to parse vertex line {}: {}", lineCount, line);
                }
            } else if (line.starts_with("f ")) {
                if (!parseFace(line, faceIndices, uvIndices)) {
                    std::print(std::cerr, "Failed to parse face line {}: {}", lineCount, line);
                }
            } else if (line.starts_with("vt ")) {
                if (glm::vec2 uv; std::sscanf(line.c_str(), "vt %f %f", &uv[0], &uv[1]) == 2) {
                    uvs.emplace_back(uv);
                } else {
                    std::print(std::cerr, "Failed to parse uv line {}: {}", lineCount, line);
                }
            }
        }

        // Tighten vectors
        vertices.shrink_to_fit();
        uvs.shrink_to_fit();
        faceIndices.shrink_to_fit();
        uvIndices.shrink_to_fit();

        file.close();

        return {.vertices = vertices, .uvs = uvs, .faceIndices = faceIndices, .uvIndices = uvIndices};
    }
}
