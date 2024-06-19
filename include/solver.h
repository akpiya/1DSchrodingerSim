/******************************************************************************
 * Defines structs and functions that are used for the mathematical computation
 * for the eigenvalues and eigenvectors
 *
 * Spectrum solver method, tqli(), is taken from "Numerical Reciples in C". It
 * finds the spectrum of a symmetrical tridiagonal matrix.
******************************************************************************/

#ifndef SOLVER_H
#define SOLVER_H

#include "raylib.h"

// Contains information about the solving for eigenvalues/eigenvectors
typedef struct EigenPackage
{
    double *evalues; // diagonal value in the tridiagonal matrix
    Vector2 **efunctions; // packaged representation of eigenvectors that is displayable
    int num_efunctions; // Number of eigenfunctions to display.
    int n; // discretization
    int displayable; // State variable to know when the solver is done running
    double *subdiagonal; // the subdiagonal of the matrix
    double **z; // Out-parameter for the spectrum solver. Contains all eigenvectors
} EigenPackage;

// Interal struct that is passed into the pthread for parallelization
struct SolverPkg
{
    Vector2 *potential;
    double n;
    unsigned char num_eigenfunctions;
    EigenPackage *epkg;
};


// Called at the beginning of the run. New eigenpackages overwrite the one initialized here.
EigenPackage *init_eigenpackage(int num_evalues, int n, double *domain);

// Called at the end
void free_eigenpackage(EigenPackage *pkg);

// Create nxn identity matrix
double **create_identity(int n);

// Disretizes domain and returns an array of values
double *create_domain(int l_bound, int r_bound, int n);

// for debugging purpose, displays the double** as a 2d array 
void show_2D(double **arr, int n);

// Used in the initialization for a default potential
Vector2* apply_potential(double *domain, int n, double (*f) (double));

// Function from Numerical Recipes in C
void tqli(double *d, double *e, double **z, int n);

void free_square_matrix(double **z, int n);

// pthread function that takes in a SolverPkg and does operations in-place
void *solve_spectrum(void *);

// Sorts the eigenvalues and eigenvectors correspondingly. Needed to extract the least eigenvalues/vectors
double **sort_e_vectors(double *evalues, double **evectors, int N);

int min(int a, int b);

int max(int a, int b);

#endif
