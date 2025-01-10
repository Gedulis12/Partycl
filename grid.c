#include "grid.h"
#include "particle.h"

Grid* grid_init()
{
    Grid *g = (Grid*)malloc(sizeof(Grid));
    grid_get_tile_len(g, SCREEN_W, SCREEN_H, MAX_RADIUS);
    g->cols = (SCREEN_W / g->tile_len);
    g->rows = (SCREEN_H / g->tile_len);
    g->tiles = (Tile*)malloc(sizeof(Tile) * (g->rows * g->cols));
    for (int i = 0; i < g->rows; i++)
    {
        for (int j = 0; j < g->cols; j++)
        {
            int id = i * g->cols + j;

            g->tiles->id = id;
            g->tiles[id].pos.x = j * g->tile_len;
            g->tiles[id].pos.y = i * g->tile_len;
            printf("id: %d, x: %d, y: %d\n", id, j*g->tile_len, i*g->tile_len);
        }
    }
    printf("rows: %d, cols: %d\n", g->rows, g->cols);
    return g;
}

void grid_get_tile_len(Grid *g, int w, int h, int max_radius)
{
    int size = max_radius;
    while ((w % size != 0 || h % size != 0) && (size < w && size < h))
    {
        size++;
    }
    g->tile_len = size;
}

Tile* grid_get_tile_by_pos(Grid *g, int x, int y)
{

    int id = (g->cols - 1) * y + x;
    if (id == g->tiles->id)
    {
        return &g->tiles[id];
    }
    return NULL;
}

void grid_free(Grid *g)
{
    free(g->tiles);
    g->tiles = NULL;
    free(g);
}

void grid_debug_draw(SDL_Renderer *r, Grid *g)
{
    if (g == NULL) return;

    SDL_SetRenderDrawColor(r, 255, 255, 255, 0);
    int x = g->tile_len;
    int y = g->tile_len;
    while (x < SCREEN_W)
    {
        SDL_RenderDrawLine(r, x, 0, x, SCREEN_H);
        x += g->tile_len;
    }
    while (y < SCREEN_H)
    {
        SDL_RenderDrawLine(r, 0, y, SCREEN_W, y);
        y += g->tile_len;
    }
}

void tiles_check_collisions(Tile *t1, Tile *t2)
{
    Particles p;
    particles_init(&p, 8);

    for (size_t i = 0; i < t1->p->used; i++)
    {
        particles_add(&p, t1->p->array[i]);
    }

    for (size_t i = 0; i < t2->p->used; i++)
    {
        particles_add(&p, t2->p->array[i]);
    }

    particles_solve_collisions(&p);
    particles_free(&p);
}

void grid_populate_tiles_with_particles(Grid *g, Particles *p)
{
    for (size_t i = 0; i < p->used; i++)
    {
        Particle *check = &p->array[i];
        int x = check->current.x / (g->cols * g->tile_len);
        int y = check->current.y / (g->rows * g->tile_len);

        Tile *t = grid_get_tile_by_pos(g, x, y);

        if (!t->p->array)
        {
            particles_init(t->p, 4);
        }
        particles_add(t->p, *check);
    }
}

/*
-int grid_get_id_by_pos(Grid *g, int x, int y)
-{
-    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
-        return -1;
-    }
-
-    int colid = (int)(x / g->tile_len);
-    int rowid = (int)(y / g->tile_len);
-    int id = rowid * g->cols + colid;
-    printf("id: %d, x: %d, y: %d\n", id, x, y);
-    return id;
-}
*/

void grid_find_collisions(Grid *g)
{
    for (int x = 0; x < g->cols; x++)
    {
        for (int y = 0; y < g->rows; x++)
        {
            Tile *t = grid_get_tile_by_pos(g, x, y);

            int dx_min, dx_max = -1;
            int dy_min, dy_max = 1;

            if (x == 0) dx_min = 0;
            if ((int)x == g->cols-1) dx_max = 0;

            if (y == 0) dy_min = 0;
            if ((int)y == g->rows-1) dy_max = 0;

            for (int dx = dx_min; dx <= dx_max; dx++)
            {
                for (int dy = dy_min; dy <= dy_max; dy++)
                {
                    Tile *t2 = grid_get_tile_by_pos(g, x + dx, y + dy);
                }
            }
        }
    }
}
