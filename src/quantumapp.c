#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "solver.h"
#include "potential.h"
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>

const int N = 500;
const Vector2 ORIGIN = {0.0, 0.0};

typedef struct SimConfig
{
    Camera2D camera;
    unsigned char zoom_mode;
    unsigned char paused;
    unsigned char num_eigenfunctions;
    unsigned char paint;
    double dt;
    double arrow_side_length;
    double click_radius;
    double horizontal_axis;
    double axis_thickness;
    double vertical_axis;
} SimConfig;

typedef struct GuiOverlay
{
    Rectangle background;
    Rectangle paint_button;
} GuiOverlay;

typedef struct EigenPackage
{
    double *evalues;
    double **efunctions;
} EigenPackage;

Vector2* apply_potential(double *domain, int n, double (*f) (double)) 
{
    Vector2 *points = malloc(sizeof(Vector2)*n);
    for (int i=0;i<n;i++) {
        points[i].x = domain[i];
        points[i].y = (*f)(domain[i]);
    }
    return points;
}

void display_points(Vector2 *points, int n, Color color, int width, int height) 
{
    Vector2 *scaled_points = malloc(sizeof(Vector2)*n);
    double max_val = -1.0;
    for (int i=0;i<n;i++) {
        if (points[i].y > max_val)
            max_val = points[i].y;
    }

    for (int i=0;i<n;i++)
    {
        scaled_points[i].x = points[i].x * width;
        scaled_points[i].y = -1 * (points[i].y / max_val) * height;
    }
    DrawLineStrip(scaled_points, N, color);
}

EigenPackage *solve_spectrum(Vector2 *potential, int n, int k)
{
    double dl = potential[1].x - potential[0].x;
    double *diagonal = malloc(sizeof(double)*(n-2));
    double *subdiagonal = malloc(sizeof(double)*(n-2));
    for(int i=0;i<n-2;i++) {
        diagonal[i] = 1.0 / (dl * dl) + potential[i+1].y;
        subdiagonal[i] = -1.0 / (2 * dl * dl);
    }
    double **z = create_identity(n-2);
    tqli(diagonal, subdiagonal, z, n-2);
    z = sort_e_vectors(diagonal, z, n-2);

    // extract wavefunctions from z
    Vector2 **wavefunctions = malloc(sizeof(Vector2*)*k); 
    for(int j=0;j<k;j++) {
        wavefunctions[j] = malloc(sizeof(Vector2)*(n));
        double area = 0;

        // Applying the boundary conditions
        wavefunctions[j][0].x = potential[0].x;
        wavefunctions[j][0].y = 0.0;

        wavefunctions[j][n-1].x = potential[n-1].x;
        wavefunctions[j][n-1].y = 0.0;

        for(int i=1;i<n-1;i++) {
            wavefunctions[j][i].x = potential[i].x;
            wavefunctions[j][i].y = z[i-1][j];
            area += z[i-1][j] * z[i-1][j];
        }
        area *= dl;
        // normalize the wavefunction
        for(int i=0;i<n;i++) {
            wavefunctions[j][i].y /= sqrt(area);
        }

        for(int i=0;i<n;i++) {
            wavefunctions[j][i].y = wavefunctions[j][i].y * wavefunctions[j][i].y;
        }
    }
    free(diagonal);
    free(subdiagonal);
    for(int i=0;i<n-2;i++) {
        free(z[i]);
    }
    free(z);
    return wavefunctions;
}

double *create_domain(int l_bound, int r_bound)
{
    double dl = (r_bound - l_bound) / ((double) N);
    double *domain = malloc(sizeof(double)*(N+1));
    for(int i=0;i<=N;i++)
    {
         domain[i] = l_bound + i * dl; 
    }
    return domain;
}

SimConfig *init_configuration()
{
    SimConfig *config = malloc(sizeof(SimConfig));
    config->camera = (Camera2D) { .offset={0.0, GetScreenHeight()}, .zoom=1.0f };
    config->click_radius = 20.0;
    config->arrow_side_length = 25.0;
    config->zoom_mode = 0;
    config->paused = 0;
    config->num_eigenfunctions = 3;
    config->horizontal_axis = GetScreenWidth();
    config->vertical_axis = GetScreenHeight();
    config->axis_thickness = 4.0;
    config->paint = 0;
    return config;
}

GuiOverlay *init_gui()
{
    GuiOverlay *gui = malloc(sizeof(GuiOverlay));
}

void draw_gui()
{
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screen_width = 800;
    const int screen_height = 600;

    InitWindow(screen_width, screen_height, "1D Schrodinger Equation Solver");
    SimConfig *config = init_configuration();
    double *x = create_domain(0, 1);
    Vector2 *potential = apply_potential(x, N+1, &gaussian); 
    Color eig_colors[6] = {RED, GREEN, ORANGE, PURPLE, BROWN, BLUE};
    Vector2 **eigenstates = find_eigenstates(
        potential,
        N+1,
        config->num_eigenfunctions
    );
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0 / config->camera.zoom);
            config->camera.target = Vector2Add(config->camera.target, delta);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && config->paint)
        {
            SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0 / config->camera.zoom);

            Vector2 cursor_pos = GetScreenToWorld2D(GetMousePosition(), config->camera);
            cursor_pos.y *= -1;

            if (Vector2Distance(cursor_pos,
                (Vector2) {config->horizontal_axis, 0}) < config->click_radius * 1.0 / config->camera.zoom)
            {
                // horizontal_axis = Vector2Add(horizontal_axis, (Vector2) {-1 * delta.x, 0});
                config->horizontal_axis = cursor_pos.x;
                SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
            }
            if (Vector2Distance(cursor_pos, 
                (Vector2) {0, config->vertical_axis}) < config->click_radius * 1.0 / config->camera.zoom)
            {
                // vertical_axis = Vector2Add(vertical_axis, (Vector2) {0, delta.y});
                config->vertical_axis = cursor_pos.y;
                SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            // Get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), config->camera);

            // Set the offset to where the mouse is
            config->camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point 
            // under the cursor to the screen space point under the cursor at any zoom
            config->camera.target = mouseWorldPos;

            // Zoom increment
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            config->camera.zoom = Clamp(config->camera.zoom*scaleFactor, 0.125f, 64.0f);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_gui();

        BeginMode2D(config->camera);
        // Draw the 3d grid, rotated 90 degrees and centered around 0,0 
        // just so we have something in the XY plane
        rlPushMatrix();
            rlTranslatef(0, 25*50, 0);
            rlRotatef(90, 1, 0, 0);
            // DrawGrid(100, 50.0);
            rlPopMatrix();
        display_points(potential, N+1, BLACK, config->horizontal_axis, config->vertical_axis);
        // displaying desired potential
        for(int i=0;i<config->num_eigenfunctions;i++) {
            display_points(eigenstates[i], N+1, eig_colors[i%6], config->horizontal_axis, config->vertical_axis);
        }

        // display resizeable axes
        config->vertical_axis *= -1;
        DrawLineEx(
            ORIGIN,
            (Vector2) {config->horizontal_axis, 0},
            config->axis_thickness,
            BLACK
        );

        DrawLineEx(
            ORIGIN,
            (Vector2) {0, config->vertical_axis},
            config->axis_thickness,
            BLACK
        );

        DrawTriangle(
            Vector2Add(
                (Vector2) {config->horizontal_axis, 0.0}, 
                (Vector2) {sqrt(3)*config->arrow_side_length / 2.0, 0.0}
            ),
            Vector2Add(
                (Vector2) {config->horizontal_axis, 0.0},
                (Vector2) {0.0, -1*config->arrow_side_length / 2.0}
            ),
            Vector2Add(
                (Vector2) {config->horizontal_axis, 0.0}, 
                (Vector2){ 0.0, config->arrow_side_length / 2.0}
            ),
            BLACK
        );
        DrawTriangle(
            Vector2Add(
                (Vector2) {0.0, config->vertical_axis},
                (Vector2) {0.0, -1*sqrt(3)*config->arrow_side_length / 2.0}
            ),
            Vector2Add(
                (Vector2) {0.0, config->vertical_axis},
                (Vector2) {-1*config->arrow_side_length / 2.0, 0.0}
            ),
            Vector2Add(
                (Vector2) {0.0, config->vertical_axis},
                (Vector2) {config->arrow_side_length / 2.0, 0.0}
            ),
            BLACK
        );
        config->vertical_axis *= -1;
                
        EndMode2D();
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}
