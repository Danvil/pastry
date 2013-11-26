
#include <iostream>
#include <pastry/pastry.hpp>
#include <SDL2/SDL.h>

int main(int argc, char** argv)
{
	std::cout << "Starting engine" << std::endl;

	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		std::cout << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* win = nullptr;
	win = SDL_CreateWindow(
		"Hello World!",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_OPENGL);
	if(win == nullptr) {
		std::cout << SDL_GetError() << std::endl;
		return 1;
	}

	while(true) {
		SDL_Event e;
		if(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				break;
			}
		}
	}

	SDL_DestroyWindow(win);

	SDL_Quit();

	std::cout << "Graceful quit" << std::endl;
	return 0;
}
