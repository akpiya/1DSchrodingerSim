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
const int NUM_COMPUTE_EVECTORS = 30;

const Color GUI_COLOR = (Color) {112, 128, 144, 150};
const Color UNSELECTED = (Color) {229, 228, 226, 255};
const Color SELECTED = (Color) {128, 128, 128, 255};

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

} SimConfig;

typedef struct GuiConfig
{
    Rectangle gui_background;
    Rectangle cursor_btn; // cursor is selected
    Rectangle paint_btn;  // paintbrush is selected
    Rectangle textbox;    // textbox is selected
    Rectangle efunc_btn;  // efunc_btn is pressed. Unpressed on mouse release

    int slot_capacity;
    float gui_offset;
    float button_offset;
    float gui_height;

    unsigned char selected_cursor;
    unsigned char selected_paint;
    unsigned char selected_text;
    unsigned char selected_evalue;
} GuiConfig;

typedef struct EigenPackage
{
    double *evalues;
    Vector2 **efunctions;
} EigenPackage;

typedef struct WaveFunction
{
    int size;
    double complex *points;
} WaveFunction;

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
    free(subdiagonal);
    for(int i=0;i<n-2;i++) {
        free(z[i]);
    }
    free(z);
    EigenPackage *ret = malloc(sizeof(EigenPackage));
    ret->evalues = diagonal;
    ret->efunctions = wavefunctions;
    return ret;
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

SimConfig *init_simconfig()
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
    config->t = 0;
    return config;
}

GuiConfig *init_guiconfig()
{
    GuiConfig *config = malloc(sizeof(GuiConfig));

    config->slot_capacity = 6;
    config->button_offset = 7.0;
    config->gui_offset = 40.0;
    config->gui_height = 50.0;
    
    float box_width = ((GetScreenWidth() - 2*config->gui_offset - (config->slot_capacity+1)*config->button_offset) )
        / ((float) config->slot_capacity);

    config->selected_cursor = 1;
    config->selected_paint = 0;
    config->selected_evalue = 0;
    config->selected_text = 0;

    config->gui_background = (Rectangle) {
        config->gui_offset,
        config->gui_offset/2.0,
        GetScreenWidth() - 2*config->gui_offset,
        config->gui_height
    };

    config->cursor_btn = (Rectangle) {
        config->gui_offset + config->button_offset, 
        config->gui_offset / 2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->paint_btn = (Rectangle) {
        config->gui_offset + 2*config->button_offset + box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->textbox = (Rectangle) {
        config->gui_offset + 4*config->button_offset + 3*box_width,
        config->gui_offset/2 + config->button_offset,
        2*box_width + config->button_offset,
        config->gui_height - 2*config->button_offset
    };

    config->efunc_btn = (Rectangle) {
        config->gui_offset + 6*config->button_offset + 5*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    return config; 
}

void draw_gui(GuiConfig *config)
{
    int screen_width = GetScreenWidth();
    

    // Draw the translucent box;
    DrawRectangleRounded(
        config->gui_background,
        0.3,
        5,
        GUI_COLOR
    );
    
    Color cursor_btn_color;
    Color paint_btn_color;
    Color textbox_color;
    Color evalue_btn_color;

    if (config->selected_cursor)
        cursor_btn_color = SELECTED;
    else
        cursor_btn_color = UNSELECTED;

    if (config->selected_paint)
        paint_btn_color = SELECTED;
    else
        paint_btn_color = UNSELECTED;

    if (config->selected_text)
        textbox_color = SELECTED;
    else
        textbox_color = UNSELECTED;

    if (config->selected_evalue)
        evalue_btn_color = SELECTED;
    else
        evalue_btn_color = UNSELECTED;

    // Draw the cursor box
    DrawRectangleRounded(
        config->cursor_btn,
        0.2,
        5,
        cursor_btn_color
    );

    // Draw the paint box
    DrawRectangleRounded(
        config->paint_btn,
        0.2,
        5,
        paint_btn_color
    );

    // Draw textbox
    DrawRectangleRounded(
        config->textbox,
        0.2,
        5,
        textbox_color
    );

    // Draw Number of Evalue colors
    DrawRectangleRounded(
        config->efunc_btn,
        0.2,
        5,
        evalue_btn_color
    );
}

void clear_btn_selections(GuiConfig *config)
{
    config->selected_cursor = 0;
    config->selected_evalue = 0;
    config->selected_paint = 0;
    config->selected_text = 0;
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screen_width = 1000;
    const int screen_height = 700;

    InitWindow(screen_width, screen_height, "1D Schrodinger Equation Solver");
    SimConfig *config = init_simconfig();
    GuiConfig *gui_config = init_guiconfig();
    double *x = create_domain(0, 1);
    Vector2 *potential = apply_potential(x, N+1, &gaussian); 
    Color eig_colors[6] = {RED, GREEN, ORANGE, PURPLE, BROWN, BLUE};
    EigenPackage *epkg = solve_spectrum(
        potential,
        N+1,
        config->num_eigenfunctions
    );
    WaveFunction psi_0;
    psi_0.size = N-1;
    psi_0.points = malloc(sizeof(double complex)*psi_0.size);
    for (int i=0; i<N-1; i++)
        psi_0.points[i] = (double complex) epkg->efunctions[0][i].y;
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        Vector2 mouse_point = GetMousePosition();


        if (CheckCollisionPointRec(mouse_point, gui_config->cursor_btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clear_btn_selections(gui_config);
            gui_config->selected_cursor = 1;
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->paint_btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clear_btn_selections(gui_config);
            gui_config->selected_paint = 1; 
            SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->textbox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clear_btn_selections(gui_config);
            gui_config->selected_text= 1; 
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->efunc_btn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clear_btn_selections(gui_config);
            gui_config->selected_evalue = 1; 
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0 / config->camera.zoom);
            config->camera.target = Vector2Add(config->camera.target, delta);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
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
            display_points(epkg->efunctions[i], N+1, eig_colors[i%6], config->horizontal_axis, config->vertical_axis);
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
        draw_gui(gui_config);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    return 0;
}
