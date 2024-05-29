#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "solver.h"

const int N = 10000;

double linear(double x)
{
    return x;
}

double quadratic(double x)
{
    return 1000 * (x-0.5)*(x-0.5);
}

Vector2* apply_potential(double *domain, int n, double (*f) (double)) 
{
    Vector2 *points = malloc(sizeof(Vector2)*n);
    for (int i=0;i<n;i++) {
        points[i].x = domain[i];
        points[i].y = -1 * (*f)(domain[i]);
    }
    return points;
}

void display_potential(Vector2* points, int n) 
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    Vector2 *scaled_points = malloc(sizeof(Vector2)*n);
    for (int i=0;i<n;i++)
    {
        scaled_points[i].x = points[i].x * width - width / 2;
        scaled_points[i].y = points[i].y * height - height / 2;
    }
    DrawLineStrip(scaled_points, N, BLACK);
}

double *create_domain(int l_bound, int r_bound)
{
    double dl = (r_bound - l_bound) / ((double) N);
    double *domain = malloc(sizeof(double)*(N));
    for(int i=0;i<N;i++)
    {
       domain[i] = l_bound + i * dl; 
    }
    return domain;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main ()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 500;

    InitWindow(screenWidth, screenHeight, "1D Schrodinger Equation Solver");

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;
    double *x = create_domain(0, 1);
    Vector2 *potential = apply_potential(x, N, &quadratic); 
    int zoomMode = 0;   // 0-Mouse Wheel, 1-Mouse Move

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Translate based on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        float wheel = GetMouseWheelMove();

        if (wheel != 0)
        {
            // Get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point 
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode2D(camera);

                // Draw the 3d grid, rotated 90 degrees and centered around 0,0 
                // just so we have something in the XY plane
                rlPushMatrix();
                    rlTranslatef(0, 25*50, 0);
                    rlRotatef(90, 1, 0, 0);
                 rlPopMatrix();
                // Plot potential
                display_potential(potential, N);
            EndMode2D();
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}