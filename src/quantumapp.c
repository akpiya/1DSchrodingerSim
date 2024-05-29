#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "solver.h"

const int N = 100;
double constant(double x)
{
    return 0.0;
}
double linear(double x)
{
    return x;
}

double quadratic(double x)
{
    return  100 * (x-0.5)*(x-0.5);
}

double sinusodial(double x)
{
    return 0.5 * sin(25*x) + 0.5;
}

Vector2* apply_potential(double *domain, int n, double (*f) (double)) 
{
    Vector2 *points = malloc(sizeof(Vector2)*n);
    for (int i=0;i<n;i++) {
        points[i].x = domain[i];
        points[i].y = (*f)(domain[i]);
    }
    return points;
}

void display_points(Vector2 *points, int n, Color color) 
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    Vector2 *scaled_points = malloc(sizeof(Vector2)*n);
    for (int i=0;i<n;i++)
    {
        scaled_points[i].x = points[i].x * width;
        scaled_points[i].y = -1 * points[i].y * height + height;
    }
    DrawLineStrip(scaled_points, N, color);
}

Vector2 **find_eigenstates(Vector2 *potential, int n, int k)
{
    double dl = potential[1].x - potential[0].x;
    double *diagonal = malloc(sizeof(double)*n);
    double *subdiagonal = malloc(sizeof(double)*n);
    for(int i=0;i<n;i++) {
        diagonal[i] = 1.0 / (dl * dl) + potential[i].y;
        subdiagonal[i] = -1.0 / (2 * dl * dl);
    }
    double **z = create_identity(n);
    tqli(diagonal, subdiagonal, z, n);
    
    Vector2 **wavefunctions = malloc(sizeof(Vector2*)*k); 
    for(int j=0;j<k;j++) {
        wavefunctions[j] = malloc(sizeof(Vector2)*n);
        for(int i=0;i<n;i++) {
            wavefunctions[j][i].x = potential[i].x;
            printf("%.3f ", potential[i].x);
            wavefunctions[j][i].y = 10 * z[i][j] * z[i][j];
        }
    }

    free(diagonal);
    free(subdiagonal);
    for(int i=0;i<n;i++) {
        free(z[i]);
    }
    free(z);

    return wavefunctions;
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
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "1D Schrodinger Equation Solver");

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;
    double *x = create_domain(0, 1);
    Vector2 *potential = apply_potential(x,N, &constant); 
    int zoomMode = 0;   // 0-Mouse Wheel, 1-Mouse Move
    int paused = 0; // 0-unpaused, 1-paused
    int num_eigenstates = 1;

    Vector2 **eigenstates = find_eigenstates(potential, N, num_eigenstates);
    for(int i=0;i<1;i++) {
        for(int j=0;j<N;j++) {
            printf("%.3f ", eigenstates[i][j].y);
        }
    }
    printf("\n");
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
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
                    DrawGrid(100, 50.0);
                 rlPopMatrix();
                display_points(potential, N, BLACK);
                for(int i=0;i<num_eigenstates;i++) {
                    display_points(eigenstates[i], N, RED);
                }
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