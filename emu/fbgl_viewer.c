#include <SDL2/SDL.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FBGL_SHM_KEY     0x1234FBCD
#define FBGL_WIDTH       800
#define FBGL_HEIGHT      600
#define FBGL_SIZE        (FBGL_WIDTH * FBGL_HEIGHT * 4)

int main(void) {
    printf("FBGL Viewer - Connecting to shared memory...\n");
    
    // Attach to shared memory
    int shm_id = shmget(FBGL_SHM_KEY, FBGL_SIZE, 0666);
    if (shm_id < 0) {
        fprintf(stderr, "Error: Cannot access shared memory (key: 0x%08X)\n", FBGL_SHM_KEY);
        fprintf(stderr, "Make sure the FBGL program is running first!\n");
        return 1;
    }
    
    void* shared_mem = shmat(shm_id, NULL, SHM_RDONLY);
    if (shared_mem == (void*)-1) {
        fprintf(stderr, "Error: Cannot attach to shared memory\n");
        return 1;
    }
    
    printf("Connected to shared memory (SHM ID: %d)\n", shm_id);
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL Init failed: %s\n", SDL_GetError());
        shmdt(shared_mem);
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("FBGL Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        FBGL_WIDTH, FBGL_HEIGHT, SDL_WINDOW_SHOWN);
    
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        shmdt(shared_mem);
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        FBGL_WIDTH, FBGL_HEIGHT);
    
    printf("FBGL Viewer running at %dx%d\n", FBGL_WIDTH, FBGL_HEIGHT);
    printf("Press ESC or close window to exit.\n");
    
    int running = 1;
    SDL_Event event;
    Uint32 frame_count = 0;
    Uint32 last_time = SDL_GetTicks();
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }
        
        // Update texture with framebuffer data
        SDL_UpdateTexture(texture, NULL, shared_mem, FBGL_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        // Calculate FPS every second
        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000) {
            char title[256];
            snprintf(title, sizeof(title), "FBGL Viewer - %u FPS", frame_count);
            SDL_SetWindowTitle(window, title);
            frame_count = 0;
            last_time = current_time;
        }
        
        SDL_Delay(16); // ~60 FPS
    }
    
    printf("Shutting down...\n");
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    shmdt(shared_mem);
    
    return 0;
}
