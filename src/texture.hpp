#pragma once

#include <memory>

#include <SDL2/SDL_image.h>

#include "color.hpp"

namespace rasterizer {
    // Differentiate Surface from Texture
    // Surface => CPU --- Texture => GPU
    struct Surface {
        const std::uint32_t width, height;
        const std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface;

        void lock() const {
            if (SDL_LockSurface(surface.get()) != EXIT_SUCCESS) {
                throw std::runtime_error(std::format("Surface could not be locked {}", SDL_GetError()));
            }
        }

        void unlock() const {
            SDL_UnlockSurface(surface.get());
        }

        // Assumes surface lock is in correct state, user-managed
        color_t operator[](const std::size_t index) const {
            const color_t* pixels = static_cast<color_t*>(surface->pixels);
            return pixels[index];
        }
    };

    static Surface* loadPngSurface(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path) || path.extension() != ".png") {
            rasterizer::print("File does not exist or is not .png: {}", path.string());
            return nullptr;
        }

        SDL_Surface* originalSurface = IMG_Load(path.string().c_str());
        if (originalSurface == nullptr) {
            rasterizer::print("Unable to load png image: {}\n", path.string());
            rasterizer::print("IMG_Load Error: {}\n", IMG_GetError());
            return nullptr;
        }

        // Enforce app format surface
        SDL_Surface* surface = SDL_ConvertSurfaceFormat(originalSurface, rasterizer::colorFormat, 0);
        if (surface == nullptr) {
            rasterizer::print("Unable to convert png to RGBA: {}\n", path.string());
            rasterizer::print("SDL_ConvertSurfaceFormat Error: {}\n", SDL_GetError());
            SDL_FreeSurface(originalSurface);
            return nullptr;
        }

        // No longer need the original surface
        SDL_FreeSurface(originalSurface);

        const std::uint32_t width = surface->w;
        const std::uint32_t height = surface->h;

        return new Surface{
            .width = width, .height = height,
            .surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(surface, SDL_FreeSurface)
        };
    }

    static Surface* loadDataSurface(const std::uint32_t* data, const std::uint32_t width, const std::uint32_t height) {
        // Create an SDL_Surface from the data
        SDL_Surface* originalSurface = SDL_CreateRGBSurfaceWithFormatFrom(
            const_cast<std::uint32_t*>(data), width, height,
            sizeof(std::uint32_t) * 8, // Bits per pixel
            width * sizeof(std::uint32_t), // Pitch (row size in bytes)
            SDL_PIXELFORMAT_ARGB8888 // Pixel format
        );

        if (originalSurface == nullptr) {
            rasterizer::print("Unable to load data surface");
            rasterizer::print("SDL_Surface Error: {}\n", SDL_GetError());
            return nullptr;
        }

        // Enforce app format surface
        SDL_Surface* surface = SDL_ConvertSurfaceFormat(originalSurface, rasterizer::colorFormat, 0);
        if (surface == nullptr) {
            rasterizer::print("Unable to convert data to RGBA\n");
            rasterizer::print("SDL_ConvertSurfaceFormat Error: {}\n", SDL_GetError());
            SDL_FreeSurface(originalSurface);
            return nullptr;
        }

        // No longer need the original surface
        SDL_FreeSurface(originalSurface);

        return new Surface{
            .width = width, .height = height,
            .surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(surface, SDL_FreeSurface)
        };
    }
}
