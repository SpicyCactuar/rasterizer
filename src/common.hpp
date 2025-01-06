#pragma once

namespace rasterizer {
    typedef std::uint32_t color_t;

    constexpr bool isDebugMode() {
#ifdef NDEBUG
        return false; // Not Debug mode
#else
        return true; // Debug mode
#endif
    }
}
