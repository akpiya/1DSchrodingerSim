#ifndef SOLVER_H
#define SOLVER_H

#include "raylib.h"

typedef struct EigenPackage
{
    double *evalues;
    Vector2 **efunctions;
    int num_efunctions;
    int n; // discretization
    double *subdiagonal;
    double **z;
} EigenPackage;

EigenPackage *init_eigenpackage(int num_evalues, int n, double *domain);

double **create_identity(int n);

double *create_domain(int l_bound, int r_bound, int n);

void show_2D(double **arr, int n);

Vector2* apply_potential(double *domain, int n, double (*f) (double));

void tqli(double *d, double *e, double **z, int n);

void free_square_matrix(double **z, int n);

double **sort_e_vectors(double *evalues, double **evectors, int N);

int min(int a, int b);

int max(int a, int b);

void solve_spectrum(Vector2 *potential, int n, int k, EigenPackage *epkg);

#endif