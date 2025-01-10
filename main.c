#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "particle.h"
#include "grid.h"
#include "common.h"

int main() {

    // sdl window setup
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    if (sdl_setup(&window, &renderer, &font) != 0) {
        fprintf(stderr, "SDL setup failed\n");
        return 1;
    }

    SDL_Color pcol = {0, 0, 0, 255};
    SDL_Color bg_c = {25, 25, 25, 255};

    int mouse_x, mouse_y;
    Uint32 last_time = SDL_GetTicks();
    Particles particles;
    particles_init(&particles, 512);
    Grid *grid = grid_init();
    int idx = 0;
    bool quit = false;

    while (!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type)
            {
                case SDL_QUIT:
                {
                    quit = true;
                    break;
                }
                case SDL_KEYDOWN:
                {
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        Particle p;
                        pcol.r = f_randi(idx + 1) % 256;
                        idx ++;
                        pcol.g = f_randi(idx + 2) % 256;
                        idx ++;
                        pcol.b = f_randi(idx + 3) % 256;
                        particle_init(&p, mouse_x, mouse_y, 15, pcol);
                        particles_add(&particles, p);
                    }
                }
            }
        }

        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_R])
        {
            particles_free(&particles);
            particles_init(&particles, 512);
        }

        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(2)) {
            Particle p;
            pcol.r = f_randi(idx + 1) % 256;
            idx ++;
            pcol.g = f_randi(idx + 2) % 256;
            idx ++;
            pcol.b = f_randi(idx + 3) % 256;
            idx ++;
            int radius = f_randi(idx) % MAX_RADIUS;
            if (radius < MIN_RADIUS) radius = MIN_RADIUS;
            particle_init(&p, mouse_x, mouse_y, radius, pcol);
            particles_add(&particles, p);
        }
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(1)) {
            particle_add_pos_force(&particles, mouse_x, mouse_y);
        }
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(3)) {
            particle_add_neg_force(&particles, mouse_x, mouse_y);
        }

        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        SDL_SetRenderDrawColor(renderer, bg_c.r, bg_c.g, bg_c.b, bg_c.a);
        SDL_RenderClear(renderer);
        draw_stats(renderer, delta_time, particles.used, font);

        particles_render(renderer, &particles, delta_time);
        //grid_debug_draw(renderer, grid);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, SCREEN_H, SCREEN_W, SCREEN_H);
        SDL_RenderPresent(renderer);

    }

    particles_free(&particles);
    grid_free(grid);
    sdl_cleanup(&window, &renderer, &font);
    return 0;
}
