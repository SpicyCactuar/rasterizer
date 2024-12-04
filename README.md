# Rasterizer

```
// TODO: Write
```

## Project Structure

```plaintext
rasterizer/
├── src/           # Source code
└── README.md      # Project README
```

## Environment

```
// TODO: Write
```

## Build

```shell
cmake -DCMAKE_BUILD_TYPE={Debug|Release} -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S rasterizer -B cmake-build-{debug|release}
cmake --build cmake-build-{debug|release} --target rasterizer -j 14
```

## Run

```shell
bin/rasterizer-{debug|release}
```

## Controls

```
// TODO: Re-write
```

| Key(s)        | Action             |
|---------------|--------------------|
| `Placeholder` | Placeholder action |

## Technologies

* **C++**: `>= C++23`
* **Cmake**: `>= 3.28`
* **SDL**: `>= 2.30.9`
