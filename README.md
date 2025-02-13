# Rasterizer

Software 3D rasterizer based on the course [3D Programming from Scratch](https://pikuma.com/courses/learn-3d-computer-graphics-programming) taught by [Pikuma](https://pikuma.com/).

![rasterizer](https://github.com/user-attachments/assets/67fcb181-eaff-48aa-b986-2c858d8ae891)

The rasterizer features:

* [SDL2](https://github.com/libsdl-org/SDL) based renderer
* Complete graphics pipeline transformation
* Perspective-correct texture interpolation
* Top-left and DDA rasterization algorithms
* `.obj` + `.png` loading for scene population
* Visual debugging tools through [ImGui](https://github.com/ocornut/imgui)
* Cross-platform compilation support, including WASM

## Project Structure

```plaintext
rasterizer/
├── assets/        # Provided assets
├── src/           # Source code
├── wasm/          # WASM specific files
├── external/      # Included external libraries
├── external/      # Included external libraries
└── README.md      # Project README
```

## Environment - CMake

A CMake + Ninja setup is assumed. Both SDL2 and GLM are required to be available as CMake packages. On Windows you can use [MSYS2](https://www.msys2.org/), on MacOS [Homebrew](https://formulae.brew.sh/) and on Linux the package manager of choice (eg: [apt](https://ubuntu.com/server/docs/package-management) for Ubuntu).

## Build - CMake

The following commands were tested using [Clang](https://clang.llvm.org/), though all compilers with good C++23 coverage (eg: GCC) should work.

```shell
cmake -DCMAKE_BUILD_TYPE={Debug|Release} -DCMAKE_MAKE_PROGRAM=Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja -S . -B ./cmake-build-{debug|release}
cmake --build cmake-build-{debug|release} --target rasterizer -j 14
```

## Run - CMake

```shell
./bin/rasterizer-{debug|release}
```

## Environment - WASM

[Emscripten](https://emscripten.org/) is used to build the [WebAssembly](https://webassembly.org/) (WASM) target. SDL2 is provided as a built-in port. Copying GLM into `<emcc-folder>/system/include` is recommended as it simplifies the build step. Otherwise, it can be added to the `/external` folder and manually reference it.

Can be built using MSYS2's Emscripten package.

## Build - WASM

```shell
em++ src/main.cpp -std=c++23 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_WEBGL2=1 -s FULL_ES2=1 -I./external/imgui $(find ./external/imgui -name "*.cpp") -O2 -DNDEBUG -fexceptions -s EXPORTED_RUNTIME_METHODS="['FS']" -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -DRESOLUTION_SCALE=1 -o ./bin/rasterizer-wasm.html --preload-file ./assets --shell-file ./wasm/index.html
```

If GLM was not included as part of Emscripten's system libraries, it can be manually copied into `./external/glm` and included by adding `-I./externa/glm` to the build command.

## Run - WASM

```shell
cd bin
python3 -m http.server 8000
```

Afterwards, navigate to `http://localhost:8000/rasterizer-wasm.html` with the browser of choice.

## Controls

| Key(s)          | Action                           |
|-----------------|----------------------------------|
| `↑ / ↓ / ← / →` | Move frustum eye around          |
| `↑ / ↓ / ← / →` | Rotate frustum forward direction |
| `C`             | Toggle backface culling          |
| `X / Z`         | DDA / Top-Left rasterization     |
| `Esc`           | Close app (WASM simply stops)    |

## Technologies

* **C++**: `>= C++23`
* **CMake**: `>= 3.28`
* **SDL**: `>= 2.30.9`
* **GLM**: `>= 1.0.1`
* **ImGui**: `>= v1.91.8` (Bundled)

## TODOs

* [ ] Implement a floating-point rasterization algorithm
