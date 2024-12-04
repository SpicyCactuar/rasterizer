#include <SDL2/SDL.h>
#include <cstdio>

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Query SDL version
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    std::printf("Compiled against SDL version: %d.%d.%d\n",
                compiled.major, compiled.minor, compiled.patch);
    std::printf("Linked SDL version: %d.%d.%d\n",
                linked.major, linked.minor, linked.patch);

    // Show Hello Rasterizer window for 2 seconds
    SDL_Window* window = SDL_CreateWindow("Hello Rasterizer", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Delay(2000);


    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
