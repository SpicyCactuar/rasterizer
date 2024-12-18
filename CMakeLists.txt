cmake_minimum_required(VERSION 3.28)
project(rasterizer)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set runtime output directory to the project's bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find packages
find_package(SDL2 REQUIRED)

# Add your executable
add_executable(rasterizer src/main.cpp)

# Add target to executable name
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
set_target_properties(rasterizer PROPERTIES
        OUTPUT_NAME "rasterizer-${BUILD_TYPE_LOWER}"
)

# Link SDL2 library
target_link_libraries(rasterizer ${SDL2_LIBRARIES})
