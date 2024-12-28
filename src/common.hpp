#pragma once

namespace rasterizer {

    constexpr bool isDebugMode() {
#ifdef NDEBUG
        return false; // Not Debug mode
#else
        return true; // Debug mode
#endif
    }
}