#include "solver.h"
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


void tqli(double *d, double *e, double **z, int n)
{
    // `diagonal`: n-length array representing the diagonal
    // `subdiagonal`: n-length array representing the subdiagonal. subdiagonal[n-1] is arbitrary
    // 'z': initially 2d identity matrix. Out parameter for eigenvectors
    const double EPS = 0.0000000001; // 1E-10
    int m, l, iter, i, k;
    double s,r,p,g,f,dd,c,b;

    e[n-1] = 0.0;

    for (l=0;l<n;l++) {
        iter=0;
        do {
            for (m=l; m<n-1;m++) {
                dd = fabs(d[m]) + fabs(d[m+1]);
                if (fabs(e[m]) <= EPS * dd)
                    break;
            }
            if (m!=l) {
                if (iter++ == 30) {
                    fprintf(stderr, "tqli: Too many iterations\n");
                    exit(1);
                }
                g = (d[l+1] - d[l]) / (2.0 * e[l]);
                r = pythag(g, 1.0);
                g = d[m] - d[l] + e[l] / (g + sign(r, g));

                s=c=1.0;
                p=0.0;
                for (i=m-1;i>=l;i--) {
                    f = s*e[i];
                    b = c*e[i];
                    e[i+1] = (r=pythag(f,g));
                    if (r==0.0) {
                        d[i+1] -= p;
                        e[m] = 0.0;
                        break;
                    }
                    s = f/r;
                    c = g/r;
                    g = d[i+1] - p;
                    r = (d[i]-g)*s + 2.0*c*b;
                    d[i+1] = g+(p=s*r);
                    g = c*r - b;
                    for (k=0; k<n; k++) {
                        f = z[k][i+1];
                        z[k][i+1] = s*z[k][i] + c *f;
                        z[k][i] = c * z[k][i] - s*f;
                    }
                }
                if (r == 0.0 && i>= l) continue;
                d[l] -= p;
                e[l] = g;
                e[m] = 0.0;
            }
        } while(m!= l);
    }
}

void show_2D(double **arr, int n)
{
    for(int i=0; i<n;i++)
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