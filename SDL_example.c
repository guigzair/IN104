/* Set Window and a few window attributes
 * If you compile and run this you should see an
 * uninitialized Window.
 * 
 */

/* DEFINED PROGRESS GOALS
 * 
 * Initialize SDL2
 * Bring Window up
 * 
 */

#include <stdlib.h>

// on Linux we usually include SDL2 like this
#include <SDL2/SDL.h>

// typedef'ed struct from SDL_video
/* Declare a a pointer to struct surface which holds a bitmap
 * and Metadata, the struct is declared in SDL_surface.h
 */
SDL_Window   *Window;

int main(int argc, char *argv[])
{

// silence compiler warnings about unused vars
(void)argc;
(void)argv;

// https://wiki.libsdl.org/SDL_Init
// video subsystem; automatically initializes the events subsystem
SDL_Init(SDL_INIT_VIDEO);

// https://wiki.libsdl.org/SDL_WindowFlags
// going to hide everything until all set
Window = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_HIDDEN);
SDL_SetWindowSize(Window,800,800);
SDL_SetWindowTitle(Window, "Window");
SDL_ShowWindow(Window);
SDL_SetWindowPosition(Window,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);

// Don't bother about the events atm. We gonna get there
SDL_Event event;

// Setting the loop condition
int running=1;

// starting loop that catches the quit event if you press the closing button
while(running){

	// consume events
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			running=0;
		}
	}
	SDL_Delay(16);

}

// Need to clean up before we exit

// https://wiki.libsdl.org/SDL_DestroyWindow
SDL_DestroyWindow(Window);

// https://wiki.libsdl.org/SDL_Quit
// Use this function to clean up all initialized subsystems.
SDL_Quit();

return EXIT_SUCCESS;

}