/******************************************************************************
 * SimConfig contains simulation-specific data.
******************************************************************************/
#ifndef SIMCONFIG_H 
#define SIMCONFIG_H

#include "raylib.h"

// Simulation-level data. Changeable throughout program execution
typedef struct SimConfig
{
    Camera2D camera;
    unsigned char zoom_mode;
    unsigned char paused;
    unsigned char num_eigenfunctions;
    double dt;
    double t;
    double arrow_side_length;
    double click_radius;
    double horizontal_axis;
    double axis_thickness;
    double vertical_axis;
    double n;

    Vector2 *potential;
    double *domain;
} SimConfig;

// Runs once at program initialization
SimConfig *init_simconfig(int discretization);

void free_simconfig(SimConfig *config);

#endif
