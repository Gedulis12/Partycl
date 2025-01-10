#ifndef _PARTICLE_
#define _PARTICLE_
#include <SDL2/SDL.h>
#include "common.h"

typedef struct {
    Vec2 current;
    Vec2 previous;
    Vec2 acceleration;
    float radius;
    SDL_Color color;
} Particle;

typedef struct {
    Particle *array;
    size_t used;
    size_t size;
} Particles;

void particle_init(Particle *p, float x, float y, int radius, SDL_Color color);
void particle_update_position(Particle *p, float dt);
void particle_accelerate(Particle *p, Vec2 acc);
void particle_apply_gravity(Particle *p);
void particle_add_pos_force(Particles *particles, int x, int y);
void particle_add_neg_force(Particles *particles, int x, int y);
void particle_apply_constraint(Particle *p);
void particle_solve_collisions(Particle *a, Particle *b);
void particle_colorize_velocity(Particle *p);
void particle_draw(SDL_Renderer *r, Particle *p);

void particles_init(Particles *p, size_t size);
void particles_add(Particles *a, Particle p);
void particles_update(Particles *particles, float dt);
void particles_solve_collisions(Particles *p);
void particles_render(SDL_Renderer *r, Particles *p, float dt);
void particles_draw(SDL_Renderer *r, Particles *p);
void particles_free(Particles *p);

#endif // _PARTICLE_
