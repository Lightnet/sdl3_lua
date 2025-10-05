#include <SDL3/SDL.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "SDL3 Window",
        800, 600,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Main event loop
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: // Window close button
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) { // ESC key
                        running = false;
                    }
                    break;
            }
        }
        // Add rendering or other updates here if needed
        //SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF ); // 
        SDL_SetRenderDrawColor( renderer, 255, 255, 0, 255 ); // return bool
        SDL_RenderClear( renderer );

        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255);
        SDL_RenderDebugText( renderer, 10.0f, 10.0f, "TEST");

        SDL_RenderPresent( renderer );
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}