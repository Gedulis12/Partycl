#ifndef _GRID_
#define _GRID_

#include <stdint.h>
#include "common.h"
#include "particle.h"

typedef struct tile {
    int id;
    Vec2 pos;
    Particles *p;
} Tile;

typedef struct {
    Tile *tiles;
    int tile_len;
    int rows, cols;
} Grid;

Grid* grid_init();
void grid_get_tile_len(Grid *g, int w, int h, int max_radius);
Tile* grid_get_tile_by_pos(Grid *g, int x, int y);
void tiles_check_collisions(Tile *t1, Tile *t2);
void grid_find_collisions(Grid *g);
void grid_debug_draw(SDL_Renderer *r, Grid *g);
void grid_free(Grid *g);

#endif //_GRID_
