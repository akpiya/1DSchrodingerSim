#include <stdlib.h>
#include "simconfig.h"
#include "solver.h"
#include "potential.h"

SimConfig *init_simconfig(int discretization)
{
    SimConfig *config = malloc(sizeof(SimConfig));
    config->camera = (Camera2D) { .offset={0.0, GetScreenHeight()}, .zoom=1.0f };
    config->click_radius = 15.0;
    config->arrow_side_length = 25.0;
    config->zoom_mode = 0;
    config->paused = 0;
    config->num_eigenfunctions = 3;
    config->horizontal_axis = GetScreenWidth();
    config->vertical_axis = GetScreenHeight();
    config->axis_thickness = 4.0;
    config->t = 0;
    config->n = discretization;
    config->domain = create_domain(0, 1, config->n); // domain has size n+1
    config->potential = apply_potential(config->domain, config->n, &quadratic); // potential has size n+1
    return config;
}