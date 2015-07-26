#include <SDL/SDL.h>

#define _USE_MATH_DEFINES
#include <unistd.h>
#include <math.h>
#include "omxcam.h"

#define WIDTH 640
#define HEIGHT 480
#define SIZE_OF_FRAME (WIDTH * HEIGHT * 3)

static SDL_Surface *screen;
static uint8_t frame[SIZE_OF_FRAME];
static int current = 0;

void on_data (omxcam_buffer_t buffer){
        printf("%d %d ", current, buffer.length);

	memcpy(buffer.data, frame + current, buffer.length);
	current += buffer.length;
	if (SIZE_OF_FRAME <= current)
	{
		printf("%d ", current);
		SDL_Surface *image = SDL_CreateRGBSurfaceFrom(buffer.data, WIDTH, HEIGHT, 24, WIDTH * 3, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0);
		SDL_Rect srcRect = {0, 0, WIDTH, HEIGHT};
		SDL_Rect dstRect = {0, 0};
		SDL_BlitSurface(image, &srcRect, screen, &dstRect);
		SDL_Flip(screen);
		current = 0; 
	}
}

int main (){
        int quit = 0;
	SDL_Init(SDL_INIT_EVERYTHING);
        screen = SDL_SetVideoMode(WIDTH, HEIGHT, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
        //SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w * scale, src->h * scale, src->format->BitsPerPixel,
        //                src->format->Rmask, src->format->Gmask, src->format->Bmask, 0);

//        SDL_Rect srcRect = {0, 0, dst->w, dst->h};
//        SDL_Rect dstRect = {0, 0};
        //_SDL_Zoom_FP(src, dst, scale);
        //SDL_BlitSurface(dst, &srcRect, screen, &dstRect);
        SDL_Flip(screen);
  //The settings of the image capture
  omxcam_still_settings_t settings;
  
  //Initialize the settings with default values (jpeg, 2592x1944)
  settings.camera.width = WIDTH;
  settings.camera.height = HEIGHT;
  settings.format = OMXCAM_FORMAT_RGB888;
  omxcam_still_init (&settings);
  settings.camera.width = WIDTH;
  settings.camera.height = HEIGHT;
  settings.format = OMXCAM_FORMAT_RGB888;
  
  //Set the buffer callback, this is mandatory
  settings.on_data = &on_data;
  
  //Start the image streaming
  omxcam_still_start (&settings);

        //int quit = 0;
        while (!quit)
        {
                SDL_Event event;
                if (SDL_PollEvent(&event))
                {
                        if (event.type == SDL_QUIT)
                                quit = 1;
                }
        }
  
  //Then, from anywhere in your code you can stop the image capture
  //omxcam_stop_still ();

	SDL_Quit();
	return 0;
}

