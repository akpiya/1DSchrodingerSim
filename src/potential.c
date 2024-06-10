#include <potential.h>
#include <math.h>

double constant(double x)
{
    return 0.0; 
}
double linear(double x)
{
    return 1000*x;
}

double quadratic(double x)
{
    return  100 * (x-0.5)*(x-0.5);
}

double step(double x)
{
    if (x < 0.3)
        return 0.0;
    else if (x > 0.7)
        return 0.0;
    else
        return 100;
}

double gaussian(double x)
{
    return 100 * exp(-(x - 0.75) * (x - 0.75) / 0.001);
}

double sinusodial(double x)
{
    return 1000* sin(25*x) + 0.5;
}
