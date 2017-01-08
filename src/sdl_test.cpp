#include "SDL2/SDL.h"
#include "imageReader.h"

int main(int argc, char* argv[])
{
	SDL_Renderer *renderer = NULL;
    SDL_Texture *bitmapTex = NULL;
    SDL_Surface *bitmapSurface = NULL;
    SDL_Window* window = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_Log("Fail to init SDL!\n");
		return 1;
	}

	imageReader* bmpReader = imageReader::createInstance();
	string fileName = "test.bmp";
	char* pixel = NULL;

	pixel = bmpReader->loadImage(fileName);
	unsigned long width = bmpReader->getImageWidth();
	unsigned long height = bmpReader->getImageHeight();

	window = SDL_CreateWindow("test", 0, 0, width, height, SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		SDL_Log("Fail to create SDL window!\n");
		return 1;
	}

	Uint32 rmask, gmask, bmask, amask;
	rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	bitmapSurface = SDL_CreateRGBSurfaceFrom(pixel, width, height, 24, 3*width, rmask, gmask, bmask, amask);

	bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
	SDL_FreeSurface(bitmapSurface);

	SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);

	SDL_RenderPresent(renderer);

	SDL_Delay(5000);

	return 0;

}