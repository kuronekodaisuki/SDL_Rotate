//#include "stdafx.h"
#include <stdio.h>
#include <sys/time.h>
#include <SDL/SDL.h>

#pragma comment(lib, "SDL.lib")

// tick in microsec
uint64_t getTick() {
    struct timeval ts;
    uint64_t theTick = 0U;
    gettimeofday(&ts, NULL);	//clock_gettime( CLOCK_REALTIME, &ts );
    theTick  = ts.tv_usec;
    theTick += ts.tv_sec * 1000000;
    return theTick;
}

bool _SDL_Rotate(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, double degree, SDL_Rect *bound);

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	const char *filename = "neko.bmp";
	SDL_Surface *src = SDL_LoadBMP(filename);
	SDL_Surface *screen = SDL_SetVideoMode(src->w, src->h, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (src != NULL) {
		SDL_Rect srcRect = {0, 0, src->w, src->h};
		SDL_Rect dstRect = {0, 0};
		SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, src->format->BitsPerPixel,
			src->format->Rmask, src->format->Gmask, src->format->Bmask, 0);

		SDL_Rect	bound = {0, 0, src->w, src->h};
		
		SDL_LockSurface(src);
		SDL_LockSurface(dst);
		_SDL_Rotate(src, dst, src->w / 2, src->h / 2, 5, &bound);
		SDL_UnlockSurface(src);
		SDL_UnlockSurface(dst);
		SDL_BlitSurface(src, &srcRect, screen, &dstRect);
		SDL_Flip(screen);
		SDL_SaveBMP(src, "src.bmp");
		SDL_SaveBMP(dst, "skewed.bmp");
		
		uint64_t start = getTick();
		for (int angle = 0; angle <= 100; angle++)
		{
			_SDL_Rotate(src, dst, src->w / 2, src->h / 2, angle, &bound);
	//		SDL_BlitSurface(dst, &srcRect, screen, &dstRect);
	//		SDL_Flip(screen);
		}
		uint64_t end = getTick();
		printf("%ld micro sec\n", end- start);
		SDL_FreeSurface(src);
	}
	SDL_Quit();
	return 0;
}
