#pragma once

namespace rasterizer {
    typedef std::uint32_t color_t;
    static constexpr auto colorFormat = SDL_PIXELFORMAT_RGBA8888;

    constexpr bool isDebugMode() {
#ifdef NDEBUG
        return false; // Not Debug mode
#else
        return true; // Debug mode
#endif
    }
}
