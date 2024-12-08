#include <SDL2/SDL.h>
#include <cstdio>

SDL_Window* initializeWindow() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::printf("SDL_Init Error: %s\n", SDL_GetError());
        return nullptr;
    }

    // Query SDL version
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    std::printf("Compiled against SDL version: %d.%d.%d\n",
                compiled.major, compiled.minor, compiled.patch);
    std::printf("Linked SDL version: %d.%d.%d\n",
                linked.major, linked.minor, linked.patch);

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("Hello Rasterizer",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }

    return window;
}

SDL_Renderer* initializeRenderer(SDL_Window* window) {
    // Create SDL window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == nullptr) {
        std::printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }

    return renderer;
}

bool processInput() {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
            if (SDL_SCANCODE_ESCAPE == event.key.keysym.scancode) {
                return false;
            }
        default:
            break;
    }

    return true;
}

void update() {
    // TODO: Implement
}

void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 124, 58, 237, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_Window* window = initializeWindow();
    if (window == nullptr) {
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = initializeRenderer(window);
    if (renderer == nullptr) {
        return EXIT_FAILURE;
    }

    bool isRunning = true;
    while (isRunning) {
        isRunning = processInput();
        update();
        render(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
