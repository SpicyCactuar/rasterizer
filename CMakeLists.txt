cmake_minimum_required(VERSION 3.28)
project(rasterizer)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add rasterizer executable
add_executable(rasterizer src/main.cpp)

# Define the constant value as a preprocessor macro
set(RESOLUTION_SCALE 1)
target_compile_definitions(rasterizer PRIVATE RESOLUTION_SCALE=${RESOLUTION_SCALE})

# Set runtime output directory for builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find SDL2 and GLM packages
# TODO: Research how to fallback to FetchContent if local package is not found
# TODO: Not as trivial as it seems since there are platform dependant flags/macros/etc that must be accounted for
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(glm CONFIG REQUIRED)

# Link SDL2 library
target_link_libraries(rasterizer PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

# Add target to executable name
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
set_target_properties(rasterizer PROPERTIES OUTPUT_NAME "rasterizer-${BUILD_TYPE_LOWER}")
