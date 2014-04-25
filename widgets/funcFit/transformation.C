#include "main.h"
#include "model.h"
#include <math.h>

/*
  0 - No Transformation
  1 - Pow
  2 - EXP
  3 - INV
 */

void powTrans(double* in, int n, double exp, double* out)
{
    int i=0;
    for(i=0; i < n; i++)
    {
        out[i] = pow(in[i],exp);
          
    }
}//End powTrans
void  expTrans(double *in, int n, double* out)
{
    int i=0;
    for(i=0; i<n; i++)
    {
        out[i] = exp(in[i]);
    }
}//End exp Trans
void invTrans(double *in, int n, double *out)
{
    int i = 0; 
    for(i =0; i<n; i++)
    {
        out[i] = (1.0/in[i]);
    }
}
