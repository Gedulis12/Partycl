#include "common.h"


int sdl_setup(SDL_Window **w, SDL_Renderer **r, TTF_Font **f)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    *w = SDL_CreateWindow("SDL2 test",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      SCREEN_W, SCREEN_H,
                                      SDL_WINDOW_RESIZABLE);

    if (!*w)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    *r = SDL_CreateRenderer(*w, -1, SDL_RENDERER_ACCELERATED);
    if (!*r)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }

    *f = TTF_OpenFont("/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf", 24);
    if (!*f) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*r);
        SDL_DestroyWindow(*w);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    return 0;
}

void sdl_cleanup(SDL_Window **w, SDL_Renderer **r, TTF_Font **f)
{
    TTF_CloseFont(*f);
    SDL_DestroyRenderer(*r);
    SDL_DestroyWindow(*w);
    TTF_Quit();
    SDL_Quit();
}

void draw_stats(SDL_Renderer *r, float dt, int particle_count, TTF_Font *font) {
    SDL_Color white = {255, 255, 255, 255};
    char stats_text[64];
    float fps = 1.0f / dt;
    snprintf(stats_text, sizeof(stats_text), "Particles: %d, FPS: %.2f", particle_count, fps);

    SDL_Surface* text_surface = TTF_RenderText_Solid(font, stats_text, white);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(r, text_surface);
    SDL_Rect text_rect = {10, 10, text_surface->w, text_surface->h};
    SDL_RenderCopy(r, text_texture, NULL, &text_rect);
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

uint f_randi(uint index) {
    index = (index << 13) ^ index;
    return ((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff);
}
