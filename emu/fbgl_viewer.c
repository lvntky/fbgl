#include <stdio.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_TITLE "FBGL Viewer"

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Window *window = SDL_CreateWindow(SCREEN_TITLE,
					      SDL_WINDOWPOS_UNDEFINED,
					      SDL_WINDOWPOS_UNDEFINED,
					      SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!window) {
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH,
		       SCREEN_HEIGHT, SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 250);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	SDL_Delay(2000);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	return 0;
}
