#include "../src/solver.h"
#include <criterion/criterion.h>

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