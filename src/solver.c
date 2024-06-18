#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "hashmap.h"
#include "solver.h"
#include "raylib.h"

// Matrix is assumed to be tridiagonal.
// Algorithm from "Numerical Recipes in C"
//
double pythag(const double a, const double b)
{
    double absa = fabs(a), absb = fabs(b);
    return (absa > absb ? absa*sqrt(1.0+(absb/absa) * (absb/absa)):
        (absb == 0.0 ? 0.0 : absb*sqrt(1.0+(absa/absb) * (absa/absb))));
}


double sign(double a, double b)
{
    return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}


int min(int a, int b)
{
    return ((a < b) ?  a : b);
}

int max(int a, int b)
{
    return ((a > b) ? a : b);
}

EigenPackage *init_eigenpackage(int num_evalues, int n, double *domain)
{
    EigenPackage *pkg = malloc(sizeof(EigenPackage));
    pkg->subdiagonal = calloc((n-1), sizeof(double));
    pkg->evalues = calloc((n-1), sizeof(double));
    pkg->num_efunctions = num_evalues;
    pkg->n = n;
    pkg->z = create_identity(n-1);

    pkg->efunctions = malloc(sizeof(Vector2*)*num_evalues);

    for(int j=0;j<num_evalues;j++) {
        pkg->efunctions[j] = malloc(sizeof(Vector2)*(n+1));

        // Applying the boundary conditions
        pkg->efunctions[j][0].x = domain[0];
        pkg->efunctions[j][0].y = 0.0;

        pkg->efunctions[j][n].x = domain[n];
        pkg->efunctions[j][n].y = 0.0;

        for(int i=1;i<n-1;i++) {
            pkg->efunctions[j][i].x = domain[i];
            pkg->efunctions[j][i].y = 0.0; 
        }
    }
    pkg->displayable = 1;

    return pkg;
}

double *create_domain(int l_bound, int r_bound, int n)
{
    double dl = (r_bound - l_bound) / ((double) n);
    double *domain = malloc(sizeof(double)*(n+1));
    for(int i=0;i<=n;i++)
    {
         domain[i] = l_bound + i * dl; 
    }
    return domain;
}

Vector2* apply_potential(double *domain, int n, double (*f) (double)) 
{
    Vector2 *points = malloc(sizeof(Vector2)*(n+1));
    for (int i=0;i<n+1;i++) {
        points[i].x = domain[i];
        points[i].y = (*f)(domain[i]);
    }
    return points;
}

void tqli(double *d, double *e, double **z, int n)
{
    // `diagonal`: n-length array representing the diagonal
    // `subdiagonal`: n-length array representing the subdiagonal. subdiagonal[n-1] is arbitrary
    // 'z': initially 2d identity matrix. Out parameter for eigenvectors
    const double EPS = DBL_EPSILON;
    int m,l,iter,i,k;
    double s,r,p,g,f,dd,c,b;

    e[n-1] = 0.0;

    for (l = 0; l < n; l++) {
        iter = 0;
        do {
            for (m = l; m < n - 1; m++) {
                dd = fabs(d[m]) + fabs(d[m+1]);
                if (fabs(e[m]) <= EPS * dd)
                    break;
            }
            if (m != l) {
                if (iter++ == 30) {
                    fprintf(stderr, "tqli: Too many iterations\n");
                    exit(1);
                }
                g = (d[l+1] - d[l]) / (2.0 * e[l]);
                r = pythag(g, 1.0);
                g = d[m] - d[l] + e[l] / (g + sign(r, g));

                s = c = 1.0;
                p = 0.0;
                for (i = m - 1; i >= l; i--) {
                    f = s * e[i];
                    b = c * e[i];
                    e[i+1] = (r=pythag(f,g));
                    if (r == 0.0) {
                        d[i+1] -= p;
                        e[m] = 0.0;
                        break;
                    }
                    s = f/r;
                    c = g/r;
                    g = d[i+1] - p;
                    r = (d[i] - g) * s + 2.0 * c * b;
                    d[i+1] = g + (p=s * r);
                    g = c * r - b;
                    for (k = 0; k < n; k++) {
                        f = z[k][i+1];
                        z[k][i+1] = s * z[k][i] + c * f;
                        z[k][i] = c * z[k][i] - s * f;
                    }
                }
                if (r == 0.0 && i>= l) continue;
                d[l] -= p;
                e[l] = g;
                e[m] = 0.0;
            }
        } while(m != l);
    }
}

void show_2D(double **arr, int n)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            printf("%.3f ",arr[i][j]);
        }
        printf("\n");
    }
}

double **create_identity(int n)
{
    // creates nxn identity matrix
    double **outer = malloc(sizeof(double*) * n);
    if (outer == NULL)
    {
        fprintf(stderr, "create_identity: malloc failed\n");
        exit(1);
    }

    for(int i=0;i<n;i++)
    {
        outer[i] = calloc(n, sizeof(double));
        if (outer[i] == NULL)
        {
            fprintf(stderr, "create_identity: calloc failed\n");
            exit(1);
        }
        outer[i][i] = 1.0;
    }
    return outer;
}

// Entry in the hashmap
struct evalue {
    double value;
    int index;
};

uint64_t evalue_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
    struct evalue *ev = (struct evalue*) item;
    return hashmap_sip(&(ev->value), 1, seed0, seed1);
}

int evalue_compare(const void *a, const void *b, void *udata)
{
    struct evalue *ev1 = (struct evalue*) a;
    struct evalue *ev2 = (struct evalue*) b;

    // possibly the other way.
    if (ev1->value < ev2->value)
        return -1;
    else if (ev1->value > ev2->value)
        return 1;
    else
        return 0;
}

int compare_doubles(const void *a, const void *b)
{
    double *num1 = (double*) a;
    double *num2 = (double*) b;

    if (*num1 < *num2) 
        return -1;
    else if (*num1 > *num2)
        return 1;
    else
        return 0;
}

void free_square_matrix(double **z, int n)
{
    for(int i = 0; i < n; i++) {
        free(z[i]);
    }
    free(z);
}

double **sort_e_vectors(double *evalues, double **evectors, int n)
{
    struct hashmap *map = hashmap_new(sizeof(struct evalue), 0, 0, 0,
                                &evalue_hash, &evalue_compare, NULL, NULL);

    for(int i = 0; i < n-1; i++) {
        hashmap_set(map, &(struct evalue) { .value=evalues[i], .index=i});
    }

    
    qsort(evalues, n-1, sizeof(double), &compare_doubles);

    double **out = create_identity(n);

    for(int i = 0; i < n-1; i++) {
        struct evalue *eigv = (struct evalue*) (hashmap_get(map, &(struct evalue){ .value=evalues[i]}));

        for(int j = 0; j < n-1; j++) {
            out[j][i] = evectors[j][eigv->index];
        }    
    }
    free_square_matrix(evectors, n-1);
    hashmap_free(map);
    return out;
}

void *solve_spectrum(void *pkg)
{
    struct SolverPkg *solverpkg = (struct SolverPkg*) (pkg);
    Vector2 *potential = solverpkg->potential;
    int n = solverpkg->n;
    int k = solverpkg->num_eigenfunctions;
    EigenPackage *epkg = solverpkg->epkg;

    // User switches the number of eigenvalues to display
    if (k != epkg->num_efunctions)
    {
        epkg->num_efunctions = k;
    }

    double dl = potential[1].x - potential[0].x;

    for(int i=0;i<n-1;i++)
    {
        epkg->evalues[i] = 1.0 / (dl * dl) + 2000*potential[i+1].y;
        epkg->subdiagonal[i] = -1.0 / (2 * dl * dl);
        for (int j=0; j<n-1; j++)
        {
            if (i == j)
                epkg->z[i][j] = 1.0;
            else
                epkg->z[i][j] = 0.0;
        }
    }
    //tqli() is only exception to size input as pure
    tqli(epkg->evalues, epkg->subdiagonal, epkg->z, epkg->n-1);

    epkg->z = sort_e_vectors(epkg->evalues, epkg->z, epkg->n);

    // extract wavefunctions from z
    Vector2 **wavefunctions = malloc(sizeof(Vector2*)*k); // MEMORY LEAK HERE, NEED TO CLEAN
    for(int j=0;j<k;j++) {
        wavefunctions[j] = malloc(sizeof(Vector2)*(n+1));
        double area = 0;

        // Applying the boundary conditions
        wavefunctions[j][0].x = potential[0].x;
        wavefunctions[j][0].y = 0.0;

        wavefunctions[j][n].x = potential[n].x;
        wavefunctions[j][n].y = 0.0;

        for(int i=1;i<n;i++) {
            wavefunctions[j][i].x = potential[i].x;
            wavefunctions[j][i].y = epkg->z[i-1][j];
            area += epkg->z[i-1][j] * epkg->z[i-1][j];
        }
        area *= dl;
        // normalize the wavefunction
        for(int i=0;i<n+1;i++) {
            wavefunctions[j][i].y /= sqrt(area);
        }

        for(int i=0;i<n+1;i++) {
            wavefunctions[j][i].y = wavefunctions[j][i].y * wavefunctions[j][i].y;
        }
    }
    epkg->efunctions = wavefunctions;
    epkg->displayable = 1;
    return (void *) 1;
}
