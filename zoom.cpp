#include <SDL/SDL.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <arm_neon.h>

#define FIXED_POINT_t   int32_t // 24bit + 8 bit

static FIXED_POINT_t toFP(double value)
{
	return (FIXED_POINT_t)(value * 256);
}

// Bi Linear interpolation by Fixed Point
void BiLinear24_FP_NEON(SDL_Surface *src, FIXED_POINT_t X, FIXED_POINT_t Y, SDL_Surface *dst, int x, int y);

bool _SDL_Zoom_FP(SDL_Surface *src, SDL_Surface *dst, double scale)
{
        int bitsPerPixel = src->format->BitsPerPixel;
	FIXED_POINT_t dX = toFP(1 / scale);
	FIXED_POINT_t dY = toFP(1 / scale);
	FIXED_POINT_t Y = toFP(0);

        if (bitsPerPixel != dst->format->BitsPerPixel)
        {
                return false;
        }
        switch (bitsPerPixel)
        {
        case 24:
                for (int y = 0; y < dst->y; y++)
                {
                        FIXED_POINT_t X = toFP(0);
                        for (int x = 0; x < dst->x; x++)
                        {
                                BiLinear24_FP_NEON(src, X, Y, dst, x, y);
                                X += dX;
                        }
                        Y += dY;
                }
                return true;
        }
        return false;
}

int main(int argc, char *argv[])
{
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_Surface *screen = SDL_SetVideoMode(100, 100, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_Surface *src = SDL_CreateRGBSurface(SDL_SWSURFACE, 40, 40, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
	SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, 40, 40, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
	SDL_Rect srcRect = {0, 0, src->w, src->h};
        SDL_Rect dstRect = {0, 0};
	SDL_BlitSurface(dst, &srcRect, screen, &dstRect);
	SDL_Flip(screen);
	bool quit = false;
	while (!quit)
	{
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				quit = true;
		}
	}
	SDL_SaveBMP(dst, "dst.bmp");
	SDL_Quit();
}
