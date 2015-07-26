#include <SDL/SDL.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include "omxcam.h"

void on_data (omxcam_buffer_t buffer){
  //buffer: the data
  //length: the length of the buffer
}

int main (){
        SDL_Init(SDL_INIT_EVERYTHING);
//        SDL_Surface *screen = SDL_SetVideoMode(src->w * scale, src->h * scale, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
        //SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w * scale, src->h * scale, src->format->BitsPerPixel,
        //                src->format->Rmask, src->format->Gmask, src->format->Bmask, 0);

//        SDL_Rect srcRect = {0, 0, dst->w, dst->h};
//        SDL_Rect dstRect = {0, 0};
        //_SDL_Zoom_FP(src, dst, scale);
        //SDL_BlitSurface(dst, &srcRect, screen, &dstRect);
        //SDL_Flip(screen);
        int quit = 0;
        while (!quit)
        {
                SDL_Event event;
                if (SDL_PollEvent(&event))
                {
                        if (event.type == SDL_QUIT)
                                quit = 1;
                }
	}
  //The settings of the image capture
  omxcam_still_settings_t settings;
  
  //Initialize the settings with default values (jpeg, 2592x1944)
  omxcam_still_init (&settings);
  
  //Set the buffer callback, this is mandatory
  settings.on_data = &on_data;
  
  //Start the image streaming
  omxcam_still_start (&settings);
  
  //Then, from anywhere in your code you can stop the image capture
  //omxcam_stop_still ();

	SDL_Quit();
	return 0;
}

