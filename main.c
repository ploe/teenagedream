#include "global.h"

SDL_Surface *screen;
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define SCREEN_BPP 16
#define SCREEN_FPS (1000/25)
#define IN_SECONDS(n) (n * SCREEN_FPS)
SDL_Rect SCREEN_EVERYTHING = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

#define SCENE_WIDTH 720
#define SCENE_HEIGHT SCREEN_HEIGHT

SDL_Surface *scene;
static SDL_Rect viewport = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
void update_viewport(int x) {
	if(x < 0) x = 0;
	else if(x > SCENE_WIDTH-SCREEN_WIDTH) x = SCENE_WIDTH - SCREEN_WIDTH;
	viewport.x = x;
}

static Uint8 *key;

static Uint32 WHITE;
static int fade = SDL_ALPHA_OPAQUE;
int FADEOUT(Actor *f) {
	WHITE = SDL_MapRGB(screen->format, 255, 255, 255);
	void *animate(Actor *f) {
		SDL_FillRect(screen, &SCREEN_EVERYTHING, WHITE);
		SDL_SetAlpha(scene, SDL_SRCALPHA, fade -= 3);
		if(!fade) return NULL;
		return animate;
	}
	f->animate = animate;
}

int BACKDROP(Actor *b) {
	b->costume = load_costume("twilight.png");
	SDL_Rect offset = {0, 0}, clip = {0, 0, b->costume->w, b->costume->h};
	b->offset = offset; b->clip = clip;
}

int BEACH(Actor *b) {
	b->costume = load_costume("sandy.png");
	SDL_Rect offset = {0, 0}, clip = {0, 0, b->costume->w, b->costume->h};
	b->offset = offset; b->clip = clip;
}

int SEA(Actor *b) {
	b->costume = load_costume("sea.png");
	SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, b->costume->w, b->costume->h, SCREEN_BPP, 0, 0, 0, 0);
	SDL_Rect clip = {0, 0, b->costume->w, b->costume->h};
	SDL_BlitSurface(b->costume, &clip, tmp, &SCREEN_EVERYTHING);
	SDL_SetAlpha(tmp, SDL_SRCALPHA, 180);
	SDL_FreeSurface(b->costume);
	b->costume = tmp;

	SDL_Rect offset = {0, 85};
	clip. w = b->costume->w;
	clip.h = b->costume->h;
	b->offset = offset; b->clip = clip;

}


Actor *player, *titlecards;
static void *move(Actor *);

void *update(Actor *b) {
	static int i = 0, pressed = NO;
	if(key[SDLK_SPACE] && !pressed) {
		b->clip.y = 160 * ++i;
		pressed = YES;
	}
	else if(!key[SDLK_SPACE]) pressed = NO;

	if(i == 9) {
		b->offset.y = SCREEN_HEIGHT + 100;
		player->animate = move;
	}
	return update;
}

int TITLECARDS(Actor *b) {
	b->costume = load_costume("titlecards.png");
	SDL_Rect offset = {0, 0}, clip = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	b->offset = offset; b->clip = clip;	

	b->animate = update;
}

#define MAX_WADE 112
#define SCROLL_X 8
#define SCROLL_Y 2
#define START_TOP 22
#define SURFACE 50

enum {
	WALK_RIGHT = 0,
	WALK_LEFT,
	SWIM_RIGHT,
	SWIM_LEFT,
	DEAD
};

static int update_player(Actor *p) {
	int i = NO;
	if(key[SDLK_RIGHT]) {
		if(p->offset.x < MAX_WADE) {
			p->offset.y += SCROLL_Y;
			p->offset.x += SCROLL_X;
			jumpreel(p, WALK_RIGHT);
		}
		else {
			p->offset.x += SCROLL_X / 2;
			jumpreel(p, SWIM_RIGHT);
		}
		i = YES;
	}
	
	if(key[SDLK_LEFT]) {
		if(p->offset.x < MAX_WADE) {
			p->offset.x -= SCROLL_X;
			if(p->offset.y > START_TOP) p->offset.y -= SCROLL_Y;
			jumpreel(p, WALK_LEFT);
		}
		else { 
			p->offset.x -= SCROLL_X / 2;
			jumpreel(p, SWIM_LEFT);
		}
		i = YES;
	}

	static int diving = NO, bob = 0;
	if(p->offset.x >= MAX_WADE) {
		if(key[SDLK_SPACE] && (!diving)) {
			p->offset.y += 3;
			diving = YES;
		}
		else if(!key[SDLK_SPACE] && diving) diving = NO;
		else {
			++bob;
			if(!(bob % 30)) p->offset.y += 4;
		}

		if(p->offset.y > SURFACE) p->offset.y -= 1;
		
	}

	if(p->offset.x < 0) p->offset.x = 0;
	else if(p->offset.x > SCENE_WIDTH - p->clip.w) p->offset.x = SCENE_WIDTH - p->clip.w;
	update_viewport((p->offset.x + p->clip.w / 2) - (SCREEN_WIDTH / 2));

	return i;
}


static void boil(Actor *p) {
	if(p->ticks++ == 3) {
		p->frame = !p->frame;
		p->clip.x = p->clip.w * p->frame;
		p->ticks = 0;
}

}

static void *move(Actor *p) {
	update_player(p);
	boil(p);
	return move;
}

static void *still(Actor *p) {
	return still;
}

static void *dead(Actor *p) {
	jumpreel(p, DEAD);
	boil(p);
	static int fading = NO;
	if(p->offset.y > SURFACE) p->offset.y--;
	else if (!fading) {
		new_Actor(FADEOUT);
		fading = YES;
	}
	return dead;
}


static void o2gauge() {
	if(player->animate == dead) return;

	static SDL_Rect bar = {8, 8, 100, 16};
	static Uint32 color;

	if(bar.w > 60) color = SDL_MapRGB(screen->format, 0, 255, 0);
	else if((bar.w < 60) && (bar.w > 30)) color = SDL_MapRGB(screen->format, 255, 255, 0);
	else if(bar.w < 30) color = SDL_MapRGB(screen->format, 255, 0, 0);

	static int ticks = 4;

	if((player->offset.y >= SURFACE + 12) && bar.w) {
		if(!ticks--) {
			bar.w -= 2;
			ticks = 4;
		}
		SDL_FillRect(screen, &bar, color);
	}
	else bar.w = 100;
	
	if(!bar.w) player->animate = dead;
}


int PLAYER(Actor *p) {
	p->costume = load_costume("lilguy.png");
	SDL_Rect offset = {0, START_TOP}, clip = {0, 0, 64, 64};

	p->offset = offset;
	p->clip = clip;

	p->animate = still;
}

static int input() {
	if(!fade) return 0;

	static int start;
	start = SDL_GetTicks();
	key = SDL_GetKeyState(NULL);

	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		switch(e.type) {
			case SDL_QUIT:
				return 0;
			break;
		}
	}

	draw_Actors();	
	SDL_BlitSurface(scene, &viewport, screen, &SCREEN_EVERYTHING);
	o2gauge();
	SDL_Flip(screen);
	if(SCREEN_FPS > (SDL_GetTicks() - start)) 
		SDL_Delay(SCREEN_FPS - (SDL_GetTicks() - start));
	return 1;
}


int main(int argc, char *argv[]) {
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
	scene = SDL_CreateRGBSurface(SDL_HWSURFACE, SCENE_WIDTH, SCENE_HEIGHT, SCREEN_BPP, 0, 0, 0, 0);

	titlecards = new_Actor(TITLECARDS);
	new_Actor(BEACH);	
	new_Actor(SEA);
	player = new_Actor(PLAYER);
	new_Actor(BACKDROP);

	while(input()) continue;

	purge_Actors();
	SDL_Quit();
	return 0;
}
