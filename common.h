#ifndef _COMMON_
#define _COMMON_
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_W 800
#define SCREEN_H 600

#define MIN_RADIUS 8
#define MAX_RADIUS 8

typedef struct Vec2 {
    float x, y;
} Vec2;

int sdl_setup(SDL_Window **w, SDL_Renderer **r, TTF_Font **f);
void sdl_cleanup(SDL_Window **w, SDL_Renderer **r, TTF_Font **f);
void draw_stats(SDL_Renderer *r, float dt, int particle_count, TTF_Font *font);
uint f_randi(uint index);

#endif //_COMMON_
