#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_W 1000
#define SCREEN_H 1000

typedef struct Vec2 {
    double x, y;
} Vec2;

typedef struct {
    Vec2 current;
    Vec2 previous;
    Vec2 acceleration;
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
    p->current.x = x;
    p->current.y = y;
    p->previous.x = x;
    p->previous.y = y;
    p->acceleration.x = 0;
    p->acceleration.y = 0;
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

void draw_filled_circle(SDL_Renderer *r, Particle *p)
{
    SDL_SetRenderDrawColor(r, p->color.r, p->color.g, p->color.b, p->color.a);
    for (int y = -p->radius; y <= p->radius; ++y)
    {
        int x_max = (int) sqrt((p->radius * p->radius) - (y*y));
        SDL_RenderDrawLine(r, (int)(p->current.x - x_max), (int)(p->current.y + y),
                              (int)(p->current.x + x_max), (int)(p->current.y + y));
    }
}

void update_position(Particle *p, float dt)
{
    const Vec2 velocity = {
        .x = p->current.x - p->previous.x,
        .y = p->current.y - p->previous.y
    };

    p->previous = p->current;

    p->current.x = p->current.x + velocity.x + p->acceleration.x * dt * dt;
    p->current.y = p->current.y + velocity.y + p->acceleration.y * dt * dt;

    p->acceleration.x = 0;
    p->acceleration.y = 0;
}

void accelerate(Particle *p, Vec2 acc)
{
    p->acceleration.x += acc.x;
    p->acceleration.y += acc.y;
}

void apply_gravity(Particle *p)
{
    Vec2 gravity = {0.0f, 1000.0f};
    accelerate(p, gravity);
}

void add_positive_force(Particles *particles, int x, int y)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        Vec2 disp = {.x = x - p->current.x, .y = y - p->current.y};
        double dist = sqrt((disp.x*disp.x)+ disp.y+disp.y);
        if (dist > 0)
        {
            double nx = disp.x/dist;
            double ny = disp.y/dist;
            p->acceleration.x += p->acceleration.x + nx * 5000.0f;
            p->acceleration.y += p->acceleration.y + ny * 5000.0f;
        }
    }
}

void add_negative_force(Particles *particles, int x, int y)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        Vec2 disp = {.x = x - p->current.x , .y = y - p->current.y};
        double dist = sqrt((disp.x*disp.x)+ disp.y+disp.y);
        if (dist > 0)
        {
            double nx = disp.x/dist;
            double ny = disp.y/dist;
            p->acceleration.x += p->acceleration.x - nx * 10000.0f;
            p->acceleration.y += p->acceleration.y - ny * 10000.0f;
        }
    }
}

void apply_constraint(Particle *p)
{
    const double damp = 0.3f;
    const Vec2 velocity = {
        .x = p->current.x - p->previous.x,
        .y = p->current.y - p->previous.y
    };

    if (p->current.y + p->radius >= SCREEN_H)
    {
        if (velocity.y > 0.01)
        {
            p->current.y = SCREEN_H - p->radius;
            p->previous.y = p->current.y + velocity.y * damp;
        } else {
            p->current.y = SCREEN_H - p->radius;
            p->previous.y = p->current.y;
        }
    }
    if (p->current.y - p->radius <= 0)
    {
        p->current.y = 0 + p->radius;
        p->previous.y = p->current.y + velocity.y * damp;
    }
    if (p->current.x + p->radius >= SCREEN_W)
    {
        p->current.x = SCREEN_W - p->radius;
        p->previous.x = p->current.x + velocity.x * damp;
    }
    if (p->current.x + p->radius <= 0)
    {
        p->current.x = 0 - p->radius;
        p->previous.x = p->current.x + velocity.x * damp;
    }
}

void solve_collision(Particle *a, Particle *b)
{
    const Vec2 axis = {
        .x = a->current.x - b->current.x,
        .y = a->current.y - b->current.y
    };
    double dist = sqrt((axis.x * axis.x) + (axis.y * axis.y));
    if (dist < a->radius + b->radius)
    {
        double nx = axis.x / dist;
        double ny = axis.y / dist;

        double delta = a->radius + b->radius - dist;

        a->current.x += 0.5f * delta * nx;
        a->current.y += 0.5f * delta * ny;

        b->current.x -= 0.5f * delta * nx;
        b->current.y -= 0.5f * delta * ny;
    }
}

void solve_collisions(Particles *p)
{
    for (size_t i = 0; i < p->used; i++)
    {
        for (size_t j = i+1; j < p->used; j++)
        {
            solve_collision(&p->array[i], &p->array[j]);
        }
    }
}

void change_color(Particle *p)
{
    double velocity_x = p->current.x - p->previous.x;
    double velocity_y = p->current.y - p->previous.y;
    double velocity = velocity_x + velocity_y;
    p->color.r = fabs(velocity*10*255);
    p->color.g = 0;
    p->color.b = 0;
}

void particles_update(Particles *particles, double dt)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        apply_gravity(p);
        apply_constraint(p);
        update_position(p, dt);
    }
    solve_collisions(particles);
//    change_color(p);
}

void render_particles(SDL_Renderer *r, Particles *p, double dt)
{
    const int sub_steps = 8;
    const double sub_dt = dt / (double)sub_steps;
    for (int i = 0; i < sub_steps; i++)
    {
        particles_update(p, sub_dt);
    }
    for (size_t i = 0; i < p->used; i++)
    {
        draw_filled_circle(r, &p->array[i]);
    }
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
//                        Particle p;
//                        pcol.r = f_randi(idx + 1) % 256;
//                        idx ++;
//                        pcol.g = f_randi(idx + 2) % 256;
//                        idx ++;
//                        pcol.b = f_randi(idx + 3) % 256;
//                        particle_init(&p, mouse_x, mouse_y, 15, pcol);
//                        particles_add(&particles, p);
                        //apply_attraction_force(&particles, mouse_x, mouse_y);
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
            int radius = f_randi(idx) % 20;
            if (radius < 5) radius = 5;
            particle_init(&p, mouse_x, mouse_y, radius, pcol);
            particles_add(&particles, p);
        }
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(1)) {
            add_positive_force(&particles, mouse_x, mouse_y);
        }
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(3)) {
            add_negative_force(&particles, mouse_x, mouse_y);
        }

        Uint32 current_time = SDL_GetTicks();
        double delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        SDL_SetRenderDrawColor(renderer, bg_c.r, bg_c.g, bg_c.b, bg_c.a);
        SDL_RenderClear(renderer);
        draw_stats(renderer, delta_time, particles.used, font);

        render_particles(renderer, &particles, delta_time);

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
