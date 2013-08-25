#ifndef ACTOR_H
#define ACTOR_H

typedef struct Actor {
	struct Actor *next;
	SDL_Surface *costume;
	SDL_Rect offset, clip;
	int frame, ticks, reel, face;
	void *(*animate)(struct Actor *);
} Actor;

typedef int (*Constructor)(Actor *);

enum {
	RIGHT = 0,
	LEFT
};

Actor *new_Actor(Constructor init);
void free_Actor(Actor *a);
void draw_Actors();
void purge_Actors();
SDL_Surface *load_costume(char *file);
void jumpreel(Actor *a, int r);

#endif

