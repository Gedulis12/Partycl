#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define GRAVITY (9.8f * 200.0f)
#define COR 0.7f

typedef struct {
    double x, y;
    double prev_x, prev_y;
    double radius;
    double accumulated_time;
    SDL_Color color;
} Particle;

typedef struct {
    Particle *array;
    size_t used;
    size_t size;
} Particles;

void particle_init(Particle *p, double x, double y, int radius, SDL_Color color) {
    p->x = x;
    p->y = y;
    p->prev_x = x;
    p->prev_y = y;
    p->radius = radius;
    p->color = color;
    p->accumulated_time = 0;
}

void particles_init(Particles *p, size_t size)
{
    p->array = malloc(size * sizeof(Particle));
    p->used = 0;
    p->size = size;
}

void particles_add(Particles *a, Particle p)
{
    if (a->used == a->size)
    {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(Particle));
    }
    a->array[a->used++] = p;
}

void particles_free(Particles *p)
{
    free(p->array);
    p->array = NULL;
    p->used = p->size = 0;
}

int setup(SDL_Window **w, SDL_Renderer **r, TTF_Font **f)
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

void draw_filled_circle(SDL_Renderer *r, Particle p)
{
    SDL_SetRenderDrawColor(r, p.color.r, p.color.g, p.color.b, p.color.a);
    for (int y = -p.radius; y <= p.radius; ++y)
    {
        int x_max = (int) sqrt((p.radius * p.radius) - (y*y));
        SDL_RenderDrawLine(r, (int)(p.x - x_max), (int)(p.y + y),
                              (int)(p.x + x_max), (int)(p.y + y));
    }
}

double get_velocity_y(Particle *p) {
    return (p->y - p->prev_y);
}

double get_velocity_x(Particle *p) {
    return (p->x - p->prev_x);
}

void handle_bounds(Particle *p)
{
    if (p->y + p->radius >= SCREEN_H) {

        double vy = get_velocity_y(p);

        if (fabs(vy) < 0.01 && (SCREEN_H - (p->y + p->radius)) < 0.01) {
            p->y = SCREEN_H - p->radius;
            p->prev_y = p->y;
            return;
        }
        p->y = SCREEN_H - p->radius;
        double new_vy = -vy * COR;
        p->prev_y = p->y - new_vy;
    }

    if (p->y + p->radius <= 0) {
        double vy = get_velocity_y(p);
        p->y = 0 - p->radius;
        double new_vy = -vy * COR;
        p->prev_y = p->y - new_vy;
    }
    if (p->x + p->radius <= 0) {
        double vx = get_velocity_x(p);
        p->x = 0 - p->radius;
        double new_vx = -vx * COR;
        p->prev_x = p->x - new_vx;
    }
    if (p->x + p->radius >= SCREEN_W) {
        double vx = get_velocity_x(p);
        p->x = SCREEN_W - p->radius;
        double new_vx = -vx * COR;
        p->prev_x = p->x - new_vx;
    }
}

void handle_particle_collision(Particle *p1, Particle *p2) {

    double dx = p2->x - p1->x;
    double dy = p2->y - p1->y;
    double dist = sqrt(dx * dx + dy * dy);


    double min_dist = p1->radius + p2->radius;
    if (dist < min_dist) {

        double nx = dx / dist;
        double ny = dy / dist;

        double overlap = min_dist - dist;

        double move_x = nx * overlap * 0.5;
        double move_y = ny * overlap * 0.5;

        p1->x -= move_x;
        p1->y -= move_y;
        p2->x += move_x;
        p2->y += move_y;
    }
}

void handle_all_collisions(Particles *particles) {
    for (size_t i = 0; i < particles->used; i++) {
        for (size_t j = i + 1; j < particles->used; j++) {
            handle_particle_collision(&particles->array[i], &particles->array[j]);
        }
    }
}

void verlet_integration(Particle *p, double dt)
{
    const double fixed_dt = 1.0/240.0;
    p->accumulated_time += dt;
    while (p->accumulated_time >= fixed_dt) {
        double temp_x = p->x;
        double temp_y = p->y;

        double dx = (p->x - p->prev_x);
        double dy = (p->y - p->prev_y);

        p->x = p->x + dx;
        p->y = p->y + dy + GRAVITY * fixed_dt * fixed_dt;

        p->prev_x = temp_x;
        p->prev_y = temp_y;

        handle_bounds(p);
        p->accumulated_time -= fixed_dt;
    }
}

void render_particle(SDL_Renderer *r, Particle *p, double dt)
{
    verlet_integration(p, dt);
    draw_filled_circle(r, *p);
}

void draw_stats(SDL_Renderer *r, double dt, int particle_count, TTF_Font *font) {
    SDL_Color white = {255, 255, 255, 255};
    char stats_text[64];
    double fps = 1.0f / dt;
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

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    if (setup(&window, &renderer, &font) != 0) {
        fprintf(stderr, "SDL setup failed\n");
        return 1;
    }

    SDL_Color pcol = {0, 0, 0, 255};
    SDL_Color bg_c = {25, 25, 25, 255};

    int mouse_x, mouse_y;
    Uint32 last_time = SDL_GetTicks();
    Particles particles;
    particles_init(&particles, 512);

    bool quit = false;
    int idx = 0;
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
                        particle_init(&p, mouse_x, mouse_y, 5, pcol);
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
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(1)) {
            Particle p;
            pcol.r = f_randi(idx + 1) % 256;
            idx ++;
            pcol.g = f_randi(idx + 2) % 256;
            idx ++;
            pcol.b = f_randi(idx + 3) % 256;
            idx ++;
            particle_init(&p, mouse_x, mouse_y, 5, pcol);
            particles_add(&particles, p);
        }

        Uint32 current_time = SDL_GetTicks();
        double delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        SDL_SetRenderDrawColor(renderer, bg_c.r, bg_c.g, bg_c.b, bg_c.a);
        SDL_RenderClear(renderer);
        draw_stats(renderer, delta_time, particles.used, font);

        for (size_t i = 0; i < particles.used; i++) {
            render_particle(renderer, &particles.array[i], delta_time);
        }
        handle_all_collisions(&particles);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, SCREEN_H, SCREEN_W, SCREEN_H);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
