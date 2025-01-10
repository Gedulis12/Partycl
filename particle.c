#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include "particle.h"
#include "common.h"

void particle_init(Particle *p, float x, float y, int radius, SDL_Color color) {
    p->current.x = x;
    p->current.y = y;
    p->previous.x = x;
    p->previous.y = y;
    p->acceleration.x = 0;
    p->acceleration.y = 0;
    p->radius = radius;
    p->color = color;
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

void particle_update_position(Particle *p, float dt)
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

void particle_accelerate(Particle *p, Vec2 acc)
{
    p->acceleration.x += acc.x;
    p->acceleration.y += acc.y;
}

void particle_apply_gravity(Particle *p)
{
    Vec2 gravity = {0.0f, 1000.0f};
    particle_accelerate(p, gravity);
}

void particle_add_pos_force(Particles *particles, int x, int y)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        Vec2 disp = {.x = x - p->current.x, .y = y - p->current.y};
        float dist = sqrt((disp.x*disp.x)+ disp.y+disp.y);
        if (dist > 0)
        {
            float nx = disp.x/dist;
            float ny = disp.y/dist;
            p->acceleration.x += p->acceleration.x + nx * 5000.0f;
            p->acceleration.y += p->acceleration.y + ny * 5000.0f;
        }
    }
}

void particle_add_neg_force(Particles *particles, int x, int y)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        Vec2 disp = {.x = x - p->current.x , .y = y - p->current.y};
        float dist = sqrt((disp.x*disp.x)+ disp.y+disp.y);
        if (dist > 0)
        {
            float nx = disp.x/dist;
            float ny = disp.y/dist;
            p->acceleration.x += p->acceleration.x - nx * 10000.0f;
            p->acceleration.y += p->acceleration.y - ny * 10000.0f;
        }
    }
}

void particle_apply_constraint(Particle *p)
{
    const float damp = 0.3f;
    const Vec2 velocity = {
        .x = p->current.x - p->previous.x,
        .y = p->current.y - p->previous.y
    };

    if (p->current.y + p->radius >= SCREEN_H)
    {
        if (velocity.y > 0.05)
        {
            p->current.y = SCREEN_H - p->radius;
            p->previous.y = p->current.y + velocity.y * damp;
        } else {
            p->current.y = SCREEN_H - p->radius;
            p->previous.y = p->current.y;
        }
    }

    if (p->current.y + p->radius <= 0)
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

void particle_solve_collision(Particle *a, Particle *b)
{
    if (!a || !b) return;
    if (a == b) return;

    const Vec2 axis = {
        .x = a->current.x - b->current.x,
        .y = a->current.y - b->current.y
    };

    float dist = sqrt((axis.x * axis.x) + (axis.y * axis.y));

    if (dist < a->radius + b->radius)
    {
        float nx = axis.x / dist;
        float ny = axis.y / dist;

        float delta = a->radius + b->radius - dist;

        a->current.x += 0.5f * delta * nx;
        a->current.y += 0.5f * delta * ny;

        b->current.x -= 0.5f * delta * nx;
        b->current.y -= 0.5f * delta * ny;
    }
}

void particles_solve_collisions(Particles *p)
{
    for (size_t i = 0; i < p->used; i++)
    {
        for (size_t j = i+1; j < p->used; j++)
        {
            particle_solve_collision(&p->array[i], &p->array[j]);
        }
    }
}

void particle_colorize_velocity(Particle *p)
{
    float velocity_x = p->current.x - p->previous.x;
    float velocity_y = p->current.y - p->previous.y;
    float velocity = velocity_x + velocity_y;
    p->color.r = fabs(velocity*2*255);
    p->color.g = 0;
    p->color.b = 0;
}

void particles_update(Particles *particles, Grid *g, float dt)
{
    for (size_t i = 0; i < particles->used; i++)
    {
        Particle *p = &particles->array[i];
        particle_apply_gravity(p);
        particle_apply_constraint(p);
        particle_update_position(p, dt);
        //change_color(p);
    }
    particles_solve_collisions(particles);
    //grid_clean_tiles(g);
    //grid_populate_tiles(g, particles);
    //grid_find_collisions(g);
}

void particles_render(SDL_Renderer *r, Particles *p, Grid *g, float dt)
{
    const int sub_steps = VERLET_SUB_STEPS;
    const float sub_dt = dt / (float)sub_steps;
    for (int i = 0; i < sub_steps; i++)
    {
        particles_update(p, g, sub_dt);
    }
    for (size_t i = 0; i < p->used; i++)
    {
        particle_draw(r, &p->array[i]);
    }
}

void particle_draw(SDL_Renderer *r, Particle *p)
{
    SDL_SetRenderDrawColor(r, p->color.r, p->color.g, p->color.b, p->color.a);
    for (int y = -p->radius; y <= p->radius; ++y)
    {
        int x_max = (int) sqrt((p->radius * p->radius) - (y*y));
        SDL_RenderDrawLine(r, (int)(p->current.x - x_max), (int)(p->current.y + y),
                              (int)(p->current.x + x_max), (int)(p->current.y + y));
    }
}

void particles_draw(SDL_Renderer *r, Particles *p)
{
    for (size_t i = 0; i < p->used; i++)
    {
        particle_draw(r, &p->array[i]);
    }

}

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

            g->tiles[id].id = id;
            g->tiles[id].pos.x = j * g->tile_len;
            g->tiles[id].pos.y = i * g->tile_len;
            g->tiles[id].p[0] = (Particle*)malloc(sizeof(Particle) * 8);
            grid_clean_tiles(g);

            //printf("id: %d, x: %d, y: %d\n", id, j*g->tile_len, i*g->tile_len);
        }
    }
    //printf("rows: %d, cols: %d\n", g->rows, g->cols);
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
    int id = g->cols * y + x;
    return &g->tiles[id];
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
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            particle_solve_collision(t1->p[i], t2->p[j]);
        }

    }

}

void grid_populate_tiles(Grid *g, Particles *p)
{
    for (size_t i = 0; i < p->used; i++)
    {
        Particle *check = &p->array[i];
        int x = check->current.x / g->tile_len;
        int y = check->current.y / g->tile_len;

        Tile *t = grid_get_tile_by_pos(g, (int)x, (int)y);
        bool assigned = false;
        int idx = 0;

        while (!assigned)
        {
            if (idx > 7)
            {
                break;
            }

            if (t->p[idx] == NULL)
            {
                t->p[idx] = check;
                assigned = true;
            }
            idx++;
        }
    }
}

void grid_find_collisions(Grid *g)
{
    int start = SDL_GetTicks();
    for (int x = 0; x < g->cols; x++)
    {
        for (int y = 0; y < g->rows; y++)
        {
            Tile *t = grid_get_tile_by_pos(g, x, y);

            int dx_min = -1;
            int dx_max = 1;

            int dy_min = -1;
            int dy_max = 1;

            if (x == 0) dx_min = 0;
            if ((int)x == g->cols-1) dx_max = 0;

            if (y == 0) dy_min = 0;
            if ((int)y == g->rows-1) dy_max = 0;

            for (int dx = dx_min; dx <= dx_max; dx++)
            {
                for (int dy = dy_min; dy <= dy_max; dy++)
                {
                    Tile *t2 = grid_get_tile_by_pos(g, x + dx, y + dy);
                    tiles_check_collisions(t, t2);
                }
            }
        }
    }
}

void grid_clean_tiles(Grid *g)
{
    for (int i = 0; i < g->rows * g->cols; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            g->tiles[i].p[j] = NULL;
        }
    }
}
