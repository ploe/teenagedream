#include "global.h"

static Actor *top;

Actor *new_Actor(Constructor init) {
	Actor *a = calloc(1, sizeof(Actor));
	if(a) {
		a->next = top;
		top = a;
		if(init) init(a);
	}
	return a;
}

void free_Actor(Actor *a) {
	Actor *this, *prev = NULL;
	for(this = top; this != NULL; a = a->next) {
		if(a == this) {
			if(this->next) {
				if(prev) prev->next = this->next;
				else top = this->next;				
			}
			else
			{
				if(prev) prev->next = NULL;
				else top = NULL;
			}
			SDL_FreeSurface(this->costume);
			free(this);
			return;
		}
		prev = this;
	}
}

void draw_Actors() {
	Actor *a;
	for(a = top; a != NULL; a = a->next) {
		if(a->animate) a->animate = a->animate(a);
		if(a->costume) SDL_BlitSurface(a->costume, &(a->clip), scene, &(a->offset));
	}
}

void purge_Actors() {
	Actor *a;
	for(a = top; a != NULL; a = a->next) free_Actor(a);
}

SDL_Surface *load_costume(char *file) {
	SDL_Surface *tmp = IMG_Load(file);
	if(tmp) {
		SDL_Surface *t = tmp;
		tmp = SDL_DisplayFormatAlpha(tmp);
		SDL_FreeSurface(t);
	}
	return tmp;
}

void jumpreel(Actor *a, int r) {
	a->clip.y = a->clip.w * r;
}
