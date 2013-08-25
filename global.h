#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "Actor.h"

extern SDL_Surface *screen, *scene;

enum {
	YES = -1,
	NO
};

#endif
