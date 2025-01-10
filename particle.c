#include "particle.h"

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

void particle_solve_collision(Particle *a, Particle *b)
{
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

void particles_update(Particles *particles, float dt)
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
}

void particles_render(SDL_Renderer *r, Particles *p, float dt)
{
    const int sub_steps = 8;
    const float sub_dt = dt / (float)sub_steps;
    for (int i = 0; i < sub_steps; i++)
    {
        particles_update(p, sub_dt);
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
