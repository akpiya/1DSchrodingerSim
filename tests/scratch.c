#include "../src/solver.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() 
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

    printf("Output Eigenvalues\n"); 
    for(int i = 0; i < n; i++) {
        printf("%.4f ", evalues[i]);
    }
    printf("\n");

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++)
            printf("%.4f ", evectors[i][j]);
        printf("\n");
    }

    free_square_matrix(evectors, n);

    return 0;
}