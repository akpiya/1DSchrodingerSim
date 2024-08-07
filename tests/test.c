// Tests are primarily for the eigenvector/eigenvalue solver
// And the sorting function
#include "../src/solver.h"
#include <criterion/criterion.h>
#include <math.h>

const double eps = 0.005;

int within(double a, double b, double EPS)
{
    return fabs(a - b) < EPS;
}

Test(solver_tests, I_creation)
{
    double **I = create_identity(5);
    for(int i=0;i<5;i++)
        for(int j=0;j<5;j++)
        {
            if (i==j) {
                cr_assert(within(I[i][j], 1.0, eps));
            } else {
                cr_assert(within(I[i][j], 0.0, eps));
            }
        }
}

Test(solver_tests, two_01)
{
    double *d = malloc(sizeof(double)*2);
    double *e = malloc(sizeof(double)*2);
    double **z = create_identity(2);
    d[0] = 2.0;
    d[1] = 1.0;
    e[0] = 1.0;
    e[1] = 1.0;
    tqli(d,e,z,2);
    double a[2] = {2.618, 0.382};
    double b[2][2] = {
        {-0.851, 0.526},
        {-0.526, -0.851}
    };

    for (int i=0;i<2;i++) {
        cr_assert(within(d[i], a[i], eps));
    }

    for(int i=0;i<2;i++) {
        for(int j=0;j<2;j++) {
            cr_assert(within(z[i][j], b[i][j],eps));
        }
    }
}

Test(solver_test, two_02)
{
    double *d = malloc(sizeof(double)*2);
    double *e = malloc(sizeof(double)*2);
    double **z = create_identity(2);
    d[0] = 0.0;
    d[1] = 0.0;
    e[0] = 1.0;
    e[1] = 1.0;
    tqli(d,e,z,2);
    double a[2] = {1.0, -1.0}; // Expected eigenvalues
    double b[2][2] = {
        {-1.0 / sqrt(2), 1.0 / sqrt(2)},
        {1.0 / sqrt(2), 1.0 / sqrt(2)}
    };

    for(int i=0;i<2;i++) {
        cr_assert(within(fabs(d[i]), fabs(a[i]), eps));
    }

    for(int i=0;i<2;i++) {
        for(int j=0;j<2;j++) {
            cr_assert(within(fabs(z[i][j]), fabs(b[i][j]),eps));
        }
    }
}

Test(solver_test, three_01)
{
    double *d = malloc(sizeof(double)*3);
    double *e = malloc(sizeof(double)*3);
    double **z = create_identity(3);
    d[0] = 1.0;
    d[1] = 2.0;
    d[2] = 3.0;
    e[0] = 1.0;
    e[1] = 1.5;
    e[2] = -10;
    tqli(d,e,z,3);
    double a[3] = {0.10519, 1.6995, 4.19531}; // Expected eigenvalues

    for(int i=0;i<3;i++) {
        cr_assert(within(fabs(d[i]), fabs(a[i]), eps));
    }
    //show_2D(z,3); // Manually verified that eigenvectors are correct
}

// Test(solver_test, large_01)
// {
//     int N = 100;
//     double *d = malloc(sizeof(double)*N);
//     double *e = malloc(sizeof(double)*N);
//     double **z = create_identity(N);
    
//     for(int i=0; i<N; i++) {
//         d[i] = i;
//         e[i] = i;
//     }
//     tqli(d,e,z,N);
//     for(int i=0;i<N;i++) {
//         printf("%f ", d[i]);
//     }
//     printf("\n");
// }

Test(sort_test_eigs, sort_01)
{
    int n = 5;
    double *evalues = malloc(sizeof(double)*n);

    evalues[0] = 3.0;
    evalues[1] = 2.0;
    evalues[2] = 1.5;
    evalues[3] = 1.0;
    evalues[4] = 4.0;
    double **evectors = create_identity(n);

    double expected_evals[5] = {1.0, 1.5, 2.0, 3.0, 4.0};
    double expected_evecs[5][5] = {
        {0, 0, 0, 1.0, 0},
        {0, 0, 1.0, 0, 0},
        {0, 1.0, 0, 0, 0},
        {1.0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1.0},
    };

    evectors = sort_e_vectors(evalues, evectors, n);
    
    for(int i = 0; i < n; i++) {
        cr_assert(evalues[i] == expected_evals[i]);
    }

    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            cr_assert(within(evectors[i][j], expected_evecs[i][j], eps));

    free_square_matrix(evectors, n);
}
