#include "RenderSystem.hpp"

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include <SDL.h>
#include <SDL_image.h>

#include "Camera.hpp"
#include "Platform.hpp"
#include "Scene.hpp"
#include "Types.hpp"

RenderSystem::RenderSystem(uint32_t width, uint32_t height, uint32_t bwScale)
	: width(width), height(height), bwScale(bwScale)
{
	// intentionally empty
	// initialization occurs in RenderSystem::init()
}

RenderSystem::~RenderSystem()
{
	// Destroy frame timer
	if (frameTimer != nullptr) {
		delete frameTimer;
	}

	// Destroy window
	if (window != nullptr) {
		SDL_DestroyWindow(window);
	}
	
	// Destroy renderer
	if (renderer != nullptr) {
		SDL_DestroyRenderer(renderer);
	}
	
	// Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool
RenderSystem::init(const char* title,
                   bool sw)
{
	// Set title
	this->title = title;
	
	// Attempt to initialize SDL
	// If unsuccessful, print error code and return
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize: %s\n", SDL_GetError());
		return false;
	}
	
    // Attempt to initialize IMG library
    // If unsuccessful, print error code and return
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n", 
		       IMG_GetError());
		return false;
	}
	
	// Create window
	window = SDL_CreateWindow(title,
	                          SDL_WINDOWPOS_CENTERED,
	                          SDL_WINDOWPOS_CENTERED,
	                          width,
	                          height,
	                          SDL_WINDOW_FULLSCREEN);
	
	// If unsuccessful: print error code and return
	if (window == NULL) {
		printf("Window could not be created: %s\n", SDL_GetError());
		return false;
	}
	
	// Create renderer for window
	if (sw) {
		renderer = SDL_CreateRenderer(window,
	                                  -1,
	                                  SDL_RENDERER_SOFTWARE);
	}
	else {
		renderer = SDL_CreateRenderer(window,
	                                  -1,
	                                  SDL_RENDERER_ACCELERATED);
	}
	// If unsuccessful: print error code and return
	if (renderer == NULL) {
		printf("Renderer could not be created: %s\n", SDL_GetError());
		return false;
	}

	// Create frame timer
	frameTimer = new FrameTimer(100);

	// Start the frame timer
	frameTimer->start();
	
	return true;
}

void
RenderSystem::render(const Scene& scene, const Camera& cam)
{
	SDL_Rect intersectRect;
	SDL_Rect worldRect;

	auto blockToWorld = [&] (SDL_Rect& rect) {
		rect.x *= bwScale;
		rect.y *= bwScale;
		rect.w *= bwScale;
		rect.h *= bwScale;
	};

	auto worldToSDL = [&] (SDL_Rect& rect) {
		rect.y = height - rect.y - bwScale;
	};

	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	// Draw all platforms
	SDL_SetRenderDrawColor(renderer, 0x7d, 0xc1, 0xf0, 0xFF);
	for (Platform p : scene.getPlatforms()) {
		worldRect = p.getRect();
		blockToWorld(worldRect);

		if (SDL_IntersectRect(&worldRect, &cam.getRect(), &intersectRect) ==
		    SDL_TRUE) {
			worldToSDL(intersectRect);
			SDL_RenderDrawRect(renderer, &intersectRect);
		}
	}

	SDL_RenderPresent(renderer);
	
	// Update frame time
	frameTimer->tick();

	if (frameTimer->getFrameCount() % 100 == 0) {
		cout << frameTimer->getFps() << endl;
	}
}

SDL_Texture* 
RenderSystem::loadTexture(const char* path, SDL_Renderer* renderer)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
		       path,
		       IMG_GetError());
	}
	else {
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to create texture from %s! SDL Error: %s\n", 
			       path,
			       SDL_GetError());
		}
		
		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}
	
	return newTexture;
}

