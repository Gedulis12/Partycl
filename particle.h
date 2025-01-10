#ifndef _PARTICLE_
#define _PARTICLE_
#include <stdint.h>
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

typedef struct tile {
    int id;
    Vec2 pos;
    Particle* p[8];
} Tile;

typedef struct {
    Tile *tiles;
    int tile_len;
    int rows, cols;
} Grid;

void particle_init(Particle *p, float x, float y, int radius, SDL_Color color);
void particle_update_position(Particle *p, float dt);
void particle_accelerate(Particle *p, Vec2 acc);
void particle_apply_gravity(Particle *p);
void particle_add_pos_force(Particles *particles, int x, int y);
void particle_add_neg_force(Particles *particles, int x, int y);
void particle_apply_constraint(Particle *p);
void particle_solve_collision(Particle *a, Particle *b);
void particle_colorize_velocity(Particle *p);
void particle_draw(SDL_Renderer *r, Particle *p);

void particles_init(Particles *p, size_t size);
void particles_add(Particles *a, Particle p);
void particles_update(Particles *particles, Grid *g, float dt);
void particles_solve_collisions(Particles *p);
void particles_render(SDL_Renderer *r, Particles *p, Grid *g, float dt);
void particles_draw(SDL_Renderer *r, Particles *p);
void particles_free(Particles *p);

Grid* grid_init();
void grid_get_tile_len(Grid *g, int w, int h, int max_radius);
Tile* grid_get_tile_by_pos(Grid *g, int x, int y);
void grid_debug_draw(SDL_Renderer *r, Grid *g);
void tiles_check_collisions(Tile *t1, Tile *t2);
void grid_populate_tiles(Grid *g, Particles *p);
void grid_clean_tiles(Grid *g);
void grid_find_collisions(Grid *g);
void grid_free(Grid *g);

#endif // _PARTICLE_
