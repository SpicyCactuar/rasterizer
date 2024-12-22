#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "mesh.hpp"

namespace {
    bool parseFace(const std::string& line, glm::uvec3& face) {
        // Discard 'f '
        std::stringstream ss(line.substr(2));
        std::string faceIndices;

        // Parse each token into face indices, separated by spaces
        for (std::uint32_t f = 0; ss >> faceIndices && f < 3; f++) {
            std::uint32_t v = 0, vt = 0, vn = 0;

            const char* cFaceIndices = faceIndices.c_str();
            // Indices are mapped 1-indexed -> 0-indexed
            if (std::sscanf(cFaceIndices, "%u/%u/%u", &v, &vt, &vn) == 3) {
                face[f] = v - 1;
            } else if (std::sscanf(cFaceIndices, "%u//%u", &v, &vn) == 2) {
                face[f] = v - 1;
            } else if (std::sscanf(cFaceIndices, "%u/%u", &v, &vt) == 2) {
                face[f] = v - 1;
            } else if (std::sscanf(cFaceIndices, "%u", &v) == 1) {
                face[f] = v - 1;
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
        std::vector<glm::uvec3> faces;

        std::string line;
        for (std::uint32_t lineCount = 0; std::getline(file, line); ++lineCount) {
            if (line.empty() || line.starts_with('#')) {
                // Skip empty lines and comments
                continue;
            }

            if (line.starts_with("v ")) {
                if (glm::vec3 vertex;
                    std::sscanf(line.c_str(), "v %f %f %f", &vertex[0], &vertex[1], &vertex[2]) == 3) {
                    vertices.push_back(vertex);
                } else {
                    std::print(std::cerr, "Failed to parse vertex line {}: {}", lineCount, line);
                }
            } else if (line.starts_with("f ")) {
                if (glm::uvec3 face; parseFace(line, face)) {
                    faces.push_back(face);
                } else {
                    std::print(std::cerr, "Failed to parse face line {}: {}", lineCount, line);
                }
            }
        }

        file.close();

        return {vertices, faces};
    }
}
