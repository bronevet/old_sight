#ifndef MODEL_H
#define	MODEL_H
#include <string>

/*#ifdef	__cplusplus
extern "C" {
#endif*/

typedef struct flist{
    int *nlinks; //number of links
    int **links; //array of links
    int **flinks;
    int **flist; 
} FLIST;

typedef struct model{
   int N;
   int p;
   int cp;
   int L; 
   int **bitVector;
   int **tbitVector;
   double* yfunc;
   double** parm;
   char** parmName;
   std::string* funcName;
   FLIST *llist;
} MODEL;

typedef struct gamodel {
    int runs;
    int sets;
    int seed;
    int fitfunction;
    int tmate;
    int tmute;
    int tmutep;
    char decho;
    double *chisq;
    double *AIC;
    MODEL* model;
} GAMODEL;

/*#ifdef	__cplusplus
}
#endif*/

#endif	/* MODEL_H */

