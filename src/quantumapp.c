/******************************************************************************
 * Main game loop. Initializes all data-fields and runs simulation.
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <pthread.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "guiconfig.h"
#include "simconfig.h"
#include "solver.h"
#include "potential.h"

const int N = 500; // LENGTH. NUM POINTS WILL BE 501
const Vector2 ORIGIN = {0.0, 0.0};
const int NUM_COMPUTE_EVECTORS = 30; //
const int MAX_INPUT_CHARS = 3; // Textbox input size

const Color GUI_COLOR = (Color) {112, 128, 144, 150};
const Color UNSELECTED_COLOR = (Color) {229, 228, 226, 255};
const Color SELECTED_COLOR = (Color) {128, 128, 128, 255};
// only 6 distinct colors to display eigenvalues. More functions cluter the plot.
const Color EIG_COLORS[6] = {RED, GREEN, ORANGE, PURPLE, BROWN, BLUE};


typedef struct WaveFunction
{
    int size;
    double complex *points;
} WaveFunction;

// Draws points.width and height are the lengths of the horizontal and vertical axes respectively
void display_points(Vector2 *points, int n, Color color, int width, int height) 
{
    Vector2 *scaled_points = malloc(sizeof(Vector2)*n);
    double max_val = 1.0;
    for (int i=0;i<n;i++) {
        if (points[i].y > max_val)
            max_val = points[i].y;
    }

    for (int i=0;i<n;i++)
    {
        scaled_points[i].x = points[i].x * width;
        scaled_points[i].y = -1 * (points[i].y / max_val) * height;
    }

    for (int i=0; i<n-1; i++) 
    {
        DrawLineEx(scaled_points[i], scaled_points[i+1], 2.5, color);
    }
}

// Draws all information from a GuiConfig
void draw_gui(GuiConfig *config)
{
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

    // Sets the color of buttons depending on what is selected
    if (config->selected_cursor)
        cursor_btn_color = SELECTED_COLOR;
    else
        cursor_btn_color = UNSELECTED_COLOR;

    if (config->selected_paint)
        paint_btn_color = SELECTED_COLOR;
    else
        paint_btn_color = UNSELECTED_COLOR;

    if (config->selected_text)
        textbox_color = SELECTED_COLOR;
    else
        textbox_color = UNSELECTED_COLOR;

    if (config->selected_evalue)
        evalue_btn_color = SELECTED_COLOR;
    else
        evalue_btn_color = UNSELECTED_COLOR;

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

    // Draw button images
    DrawTexture(
        config->cursor_btn_texture,
        config->cursor_btn.x+5*config->button_offset,
        config->cursor_btn.y,
        WHITE
    );

    DrawTexture(
        config->paint_btn_texture,
        config->paint_btn.x+5*config->button_offset,
        config->cursor_btn.y,
        WHITE
    );

    // Draw text on the button
    DrawText(
        TextFormat("Find Eigenfunctions"),
        config->efunc_btn.x + 10,
        config->efunc_btn.y + config->efunc_btn.height / 3,
        14,
        BLACK 
    );
}

void clear_btn_selections(GuiConfig *config)
{
    config->selected_cursor = 0;
    config->selected_evalue = 0;
    config->selected_paint = 0;
    config->selected_text = 0;
}


// Program main entry point
int main()
{
    // Initialization
    // These constants are set regardless of system
    // Windows cannot be resized
    const int screen_width = 1000;
    const int screen_height = 700;

    InitWindow(screen_width, screen_height, "Schrodinger Sim");

    SimConfig *config = init_simconfig(N);
    GuiConfig *gui_config = init_guiconfig();
    EigenPackage *epkg = init_eigenpackage(config->num_eigenfunctions, N, config->domain);
    struct SolverPkg *solverpkg = malloc(sizeof(struct SolverPkg));

    pthread_t solver_thread_id;

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        Vector2 mouse_point = GetMousePosition();

        // right-click panning behavior
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0 / config->camera.zoom);
            config->camera.target = Vector2Add(config->camera.target, delta);
        }

        Vector2 cursor_pos = GetScreenToWorld2D(GetMousePosition(), config->camera);
        cursor_pos.y *= -1;
        
        // Check whether the cursor is hovering over a specific button and color it accordingly.
        // Further check if the button is actually clicked
        if (CheckCollisionPointRec(mouse_point, gui_config->cursor_btn))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                clear_btn_selections(gui_config);
                gui_config->selected_cursor = 1;
            }
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->paint_btn))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                clear_btn_selections(gui_config);
                gui_config->selected_paint = 1; 
            }
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->textbox))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                clear_btn_selections(gui_config);
                gui_config->selected_text= 1; 
            }
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
        }
        else if (CheckCollisionPointRec(mouse_point, gui_config->efunc_btn))
        {
            // all the logic for recomputing the eigenvalues once the potential has been modified
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                clear_btn_selections(gui_config);
                gui_config->selected_evalue = 1; 

                if (epkg->displayable)
                {
                    epkg->displayable = 0;
                    solverpkg->epkg = epkg;
                    solverpkg->n = config->n;
                    solverpkg->num_eigenfunctions = config->num_eigenfunctions;
                    solverpkg->potential = config->potential;

                    pthread_create(&solver_thread_id, NULL, &solve_spectrum, (void *)(solverpkg));
                }
            }
            else
            {
                gui_config->selected_evalue = 0;
            }
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } 
        else if (Vector2Distance(cursor_pos,
            (Vector2) {config->horizontal_axis, 0}) < config->click_radius * 1.0 / config->camera.zoom)
        {
            SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                config->horizontal_axis = cursor_pos.x;
        }
        else if (Vector2Distance(cursor_pos, 
            (Vector2) {0, config->vertical_axis}) < config->click_radius * 1.0 / config->camera.zoom)
        {
            // vertical_axis = Vector2Add(vertical_axis, (Vector2) {0, delta.y});
            SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                config->vertical_axis = cursor_pos.y;
        }
        else if (gui_config->selected_paint && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 mouse_world = GetScreenToWorld2D(mouse_point, config->camera);
            if (0 < mouse_world.x && mouse_world.x < config->horizontal_axis
             && 0 < -1*mouse_world.y && -1*mouse_world.y < config->vertical_axis)
            {
                Vector2 delta = GetMouseDelta();
                // delta = GetScreenToWorld2D(delta, config->camera);

                Vector2 prev_mouse_world = Vector2Subtract(mouse_world, delta);

                int index = (int) ((mouse_world.x / config->horizontal_axis) * (config->n-1));
                int index2 = (int) (((prev_mouse_world.x) / config->horizontal_axis) * (config->n-1));
                

                int index_low, index_high;
                double start, end;
                int smoothing = (int) ((2.25) * 1 / config->camera.zoom);

                if (index < index2)
                {
                    index_low = max(index-smoothing, 0);
                    index_high = min(index2+smoothing, config->n-2);
                    start = -1*mouse_world.y;
                    end = -1*prev_mouse_world.y;
                }
                else
                {
                    index_low = max(index2-smoothing, 0);
                    index_high = min(index+smoothing, config->n-2);
                    start = -1*prev_mouse_world.y;
                    end = -1*mouse_world.y;
                }
                start /= config->vertical_axis;
                end /= config->vertical_axis;

                double diff = (end - start) / (index_high - index_low);                

                for (int i=index_low; i <= index_high; i++)
                {
                    config->potential[i].y = (start + (i-index_low) * diff);
                }
            }
        }
        else
        {
            if (gui_config->selected_paint)
                SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
            else
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
        BeginMode2D(config->camera);
        // Draw the 3d grid, rotated 90 degrees and centered around 0,0 
        // just so we have something in the XY plane
        rlPushMatrix();
            rlTranslatef(0, 25*50, 0);
            rlRotatef(90, 1, 0, 0);
            // DrawGrid(100, 50.0);
            rlPopMatrix();
        display_points(config->potential, N+1, BLACK, config->horizontal_axis, config->vertical_axis);
        // displaying desired potential
        if (epkg->displayable)
        {
            for(int i=0;i<epkg->num_efunctions;i++)
            {
                display_points(epkg->efunctions[i], N, EIG_COLORS[i%6], config->horizontal_axis, config->vertical_axis);
            }
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
    }

    CloseWindow();
    return 0;
}
