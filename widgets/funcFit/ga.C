#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <map>
#include <algorithm>
using namespace std;


#include "ga.h"
#include "main.h"
#include "model.h"

//#define    MIN(A,B) ((A) < (B) ? (A) : (B))

void initBitVector(GAMODEL* gamodel)
{
    srand(time(NULL));    
    int i,j;
    gamodel->model->bitVector = (int **)malloc(gamodel->sets*sizeof(int*));
    gamodel->model->tbitVector = (int **)malloc(gamodel->sets*sizeof(int*));
    for(i = 0; i < gamodel->sets; i++)
    {
        gamodel->model->bitVector[i] = (int*)calloc(gamodel->model->cp, sizeof(int));
        gamodel->model->tbitVector[i] = (int*)calloc(gamodel->model->cp, sizeof(int));
        for(j=0; j < gamodel->model->cp; j++)
        {
            int tval = rand()%2;
            //printf("Random value: %d\n", tval);
         gamodel->model->bitVector[i][j] = tval;
         //printf("Random value: %d \n", gamodel->model->bitVector[i][j]);
        }
    }
    gamodel->chisq = (double *)calloc(gamodel->sets, sizeof(double));
    gamodel->AIC = (double *)calloc(gamodel->sets, sizeof(double));
}
//void printBitVector(int numm, int n, int **bitVector)
void printBitVector(FILE* log, GAMODEL* gamodel)
{
  if(log==NULL) return;
 int numm = gamodel->sets;
int n = gamodel->model->cp;
int** bitVector = gamodel->model->bitVector;
 fprintf(log, "\n----BitVectors----\n");
  int i,j;
  for(i = 0 ; i < numm; i++)
    {
      for(j = 0; j < n; j++)
	{
	  fprintf(log, "%d ", bitVector[i][j]);
	}
      fprintf(log, "\n");
    }
  fflush(log);
}

// Initialize the GSL matrix input and vector y with the observed data and set the vector c to 0.
// N: number of observations
// n: number of input parameters in each observation
// bitVector: indicates which parameters are included in this model
void fillMtxWithData(int N, int n, double** data, double* ydata, int* bitVector,
                     gsl_matrix *input, gsl_vector *y, gsl_vector *c)
{
      /*Init Data Input*/
      for(int iN=0; iN < N; iN++)
	{
	  for(int in=0; in < n; in++)
	    {
	      if(bitVector[in] == 1)
		{
		 
		  gsl_matrix_set(input, iN, in, data[iN][in]);
		}
	      else
		{
		  gsl_matrix_set(input, iN, in, 0.0);
		}
	    }
	  gsl_vector_set(y, iN, ydata[iN]);
	}
      for(int in = 0 ; in<n; in++)
	  gsl_vector_set(c,in,0.0);
}

// Trains the linear model c to be the best solution to equation y=input*c.
// N: number of observations
// n: number of input parameters in each observation
// cov: matrix to which the covariance will be written
// work: workspace for the linear solve
// chi: double to which the chi-squared error will be written
void trainLinModel(FILE* log, int N, int n,
                   gsl_matrix *input, gsl_vector *y, gsl_vector *c,
                   gsl_matrix *cov, gsl_multifit_linear_workspace *work, double& chi) {
    /*Linear Regression on all the data */
    int gslFlg = gsl_multifit_linear(input, y, c, cov, &chi, work);
    //if(log) fprintf(log, "chi1=%le\n", chi);
    if(gslFlg != GSL_SUCCESS){fprintf(log?log:stderr, "Regression Failed\n"); exit(1);}
      
    // Find the x% of the data elements that have the highest error, 
    // exclude them and repeat the regression without them.
    int numHighError = floor(N * .1);
    if(numHighError>0)
      {
        // Maps errors to the indexes with this error
        multimap<double, int> highErrIdx;
        for(int iN=0; iN < N; iN++)
          {
            // Compute the predicted value of this observation
            double pred=0;
            for(int in=0; in < n; in++)
                pred += gsl_vector_get(c,in) * gsl_matrix_get(input, iN, in);
            
            double error = fabs(pred - gsl_vector_get(y, iN));// / max(fabs(pred), fabs(gsl_vector_get(y, iN)));
            //if(log) fprintf(log, "        Error: iN=%d, pred=%le, real=%le, error=%le\n", iN, pred, gsl_vector_get(y, iN), error);
            
              
            // If we haven't yet placed at least numHighError entries into 
            // highErrIdx, add this observation to it
            if(highErrIdx.size()<numHighError)
            highErrIdx.insert(make_pair(error, iN));
            
            // If highErrIdx is full and the error in this observation is 
            // larger than the smallest error in highErrIdx, add it to highErrIdx 
            // and remove from highErrIdx the observation with the current smallest error.
            else if(highErrIdx.begin()->first < error) {
              highErrIdx.erase(highErrIdx.begin()->first);
              highErrIdx.insert(make_pair(error, iN));
            }
          }
        
        // Remove all the observations with high errors from input and y
        //if(log) fprintf(log, "    Erasing ");
        for(multimap<double, int>::iterator i=highErrIdx.begin(); i!=highErrIdx.end(); i++) {
          //if(log) fprintf(log, "    %d|%le|%le ", i->second, i->first, gsl_vector_get(y, i->second));
          for(int in=0; in < n; in++)
            gsl_matrix_set(input, i->second, in, 0.0);
          gsl_vector_set(y, i->second, 0.0);
        }
        //if(log) fprintf(log, "\n");
        
        /* Repeat the Linear Regression on just the normal data */
        gslFlg = gsl_multifit_linear(input, y, c, cov, &chi, work);
        //if(log) fprintf(log, "chi2=%le\n", chi);
        if(gslFlg != GSL_SUCCESS){fprintf(log?log:stderr, "Regression Failed\n"); exit(1);}
      }
}

//void findModels(int N,  int n, int **bitVector, double **data, double *ydata, double *sq)
void findModels(FILE* log, GAMODEL* gamodel)
{
  int n,N, NUMM;
  N=gamodel->model->N;
  n=gamodel->model->cp;
  NUMM=gamodel->sets;
  int** bitVector;
  bitVector = gamodel->model->bitVector;
  double* ydata;
  ydata = gamodel->model->yfunc;
  double *sq;
  sq = gamodel->chisq;
  double** data;
  data = gamodel->model->parm;
  int i; int iN ,in; int gslFlg;
  gsl_multifit_linear_workspace *work;
  gsl_matrix *input, *cov;
  gsl_vector *y, *c;
  double chi;  

  input = gsl_matrix_alloc(N,n);
  cov = gsl_matrix_alloc(n,n);
  y = gsl_vector_alloc(N); c = gsl_vector_alloc(n);
  work = gsl_multifit_linear_alloc(N, n);

  /*For Each Model*/
  for(i = 0 ; i < NUMM; i++)
    {
      fillMtxWithData(N, n, data, ydata, bitVector[i], input, y, c);

      trainLinModel(log, N, n, input, y, c, cov, work, chi);
      sq[i] = chi;
      //if(log) fprintf(log, "ChiSq: %le\n", chi);

    } /*End Each Model*/

  
  gsl_matrix_free(input); gsl_matrix_free(cov);
  gsl_vector_free(y); gsl_vector_free(c);
  gsl_multifit_linear_free(work);

}/*End Find Models*/

//void AIC(int NUMM, int N, int n, int **bitVector, double *sq)
void AIC(FILE* log, GAMODEL* gamodel)
{    
    int NUMM = gamodel->sets;
    //int N = gamodel->model->N;
    int n = gamodel->model->cp;
    int** bitVector = gamodel->model->bitVector;
    double* sq = gamodel->chisq;
    double* AIC = gamodel->AIC;
    /*AIC = SSE + 2*<Num of elements used>*/
    int i, in;
    int k; //Num of elemenets
    for(i = 0 ; i < NUMM; i ++)
    {
      k = 0;
      for(in = 0 ; in < n; in++)
	{
	  if(bitVector[i][in] == 1)
	    {
	      k++;
	    }
	}
      // Models with 1 parameter are not penalized relative to models with 0 parameters
      if(k>0) k--;
      AIC[i] = sq[i] + 10*k;
      if(log) fprintf(log, "%d: ChiSq=%le, NumBits=%d, AIC=%le\n", i, sq[i], k, AIC[i]);
    }
}
//void MateSinglePoint(int NUMM, int n, int **bitVector, int **tbitVector, double *sq)
void MateSinglePoint(FILE* log, GAMODEL* gamodel)
{
  int NUMM = gamodel->sets;
 // int N = gamodel->model->N;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  int** tbitVector = gamodel->model->tbitVector;
  double* sq = gamodel->chisq;
  double* AIC = gamodel->AIC;
  
  int i, iN, in;
  int numElit, numpair; /*numElit are ones to keep and not mate*/
  //int crosspoint=0;
  numElit = (int)ceil(((double)NUMM)*(0.10));
  if((NUMM-numElit)%2 == 1){numElit++;} /*if odd take one more to keep*/
  numpair = (NUMM-numElit)/2;

  //if(log) fprintf(log, "Number Kept: %d \n ", numElit);
  //if(log) fprintf(log, "Number Changed: %d \n", numpair);

  int *top; int topindex = 0 ; int addindex = 0;
  top = (int *)calloc(numElit, sizeof(double));
  if(top == NULL){fprintf(log?log:stderr, "NULL pointer\n"); exit(1);}

  /*Rank Models*/ /*smallest to largest*/
  //qsort(sq, NUMM, sizeof(double), compare);
  for(i = 0 ; i < numpair; i++)
    { 
      int splitpoint = rand()%(n-1);
      //if(log) fprintf(log, "Split Point: %d\n", splitpoint);

      /*Find smallest*/
      double one = DBL_MAX, two = DBL_MAX;
      int ione = 0, itwo = 0;
      for(iN = 0 ; iN < NUMM; iN++)
	{
	  if(AIC[iN] < one)
	    {
	      one = AIC[iN];
	      AIC[iN] = DBL_MAX;
	      ione = iN;
	    }
	  else if (AIC[iN] < two)
	    {
	      two = AIC[iN];
	      AIC[iN] = DBL_MAX;
	      itwo = iN;
	    }

	 }
      
      // if(log) fprintf(log, "topindex: %d %d\n", topindex, topindex+1);
      // if(log) fprintf(log, "ione: %d itwo: %d \n", ione, itwo);
      if(topindex < numElit)
	{
	  top[topindex] = ione; top[topindex+1] = itwo;
	  topindex = topindex+2;
	}
      /*Add first half*/
      for(in = 0 ; in <= splitpoint; in++)
	{
	  tbitVector[addindex][in]   = bitVector[ione][in];
	  tbitVector[addindex+1][in] = bitVector[itwo][in]; 
	} 
      /*Add second half*/
      for(in = splitpoint+1; in < n; in++)
	{
	  tbitVector[addindex][in]   = bitVector[itwo][in];
	  tbitVector[addindex+1][in] = bitVector[ione][in];
	}
      addindex = addindex+2;
    }
  for(i = 0 ; i < numElit; i ++)
    {
      for(in = 0 ; in < n; in++)
	{
	  tbitVector[addindex][in] = bitVector[top[i]][in];
	}
      addindex++;    
    }
  free(top);
}
int compare (const void * a, const void * b)
{
  double xx = *(double*)a, yy = *(double*)b;
  if(xx < yy) return -1;
  if(xx > yy) return 1;
  return 0;
}
//void copyBitVector(int NUMM, int n, int **bitVector, int **tbitVector)
void copyBitVector(GAMODEL* gamodel)
{
    
  int NUMM = gamodel->sets;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  int** tbitVector = gamodel->model->tbitVector;
  int i, in;
  
  /*Zero bitVector*/
  for(i=0; i < NUMM; i++)
    {
      for(in=0; in<n; in++)
	{
	  bitVector[i][in] = 0;
	}
    }
  /*Copy*/
  for(i=0; i < NUMM; i++)
    {
      for(in=0; in<n; in++)
	{
	  bitVector[i][in] = tbitVector[i][in];
	}
    }
  /*Zero Tmp*/
  for(i=0; i < NUMM; i++)
    {
      for(in=0; in<n; in++)
	{
	  tbitVector[i][in] = 0;
	}
    }
}
/*Come back and add mitgration*/
void mitgration(int NUMM, int N, int n, int **bitVector, double **data, double *y, double MUTP)
{
  int i,in;
  int numMUTT = (int) ceil(NUMM *((double)MUTP/100.0));
  int **tbitVector;
  double *sq1, *sq2;

  /*-----------------------INIT----------------------------*/
  tbitVector  = (int **)malloc(numMUTT * sizeof(int *));
  for(i = 0 ; i < numMUTT; i ++)
    {
      tbitVector[i] = (int *) calloc(n , sizeof(int));
    }
  sq1 = (double *) calloc(NUMM, sizeof(double));
  sq2 = (double *) calloc(numMUTT, sizeof(double));

  /*----------------RANDOM PERM--------------------------*/
  srand(time(NULL));
  for(i=0; i < numMUTT; i++)
    {
      for(in=0; in<n; in++)
	{
	  tbitVector[i][in] = rand() % 2;
	}
    }
  
  /*NOTE: MAKE CHANGES HERE*/
  //findModels(log, N, n, bitVector, data, y, sq1);
  //findModelsSQ(log)
  //AIC(NUMM, N, n, bitVector, sq1);
  //findModels(log, N, n, tbitVector, data, y, sq2);
  //AIC(NUMM, N, n, tbitVector, sq2);

  /*------------------50% Best from each----------------*/
  for(i=0; i < numMUTT; i++)
    {
      free(tbitVector[i]);
    }
  free(tbitVector);
  free(sq1); free(sq2);
}
//void mutation(int NUMM, int N, int n, int **bitVector, double MUTP)
void mutation(GAMODEL* gamodel)
{
  int NUMM = gamodel->sets;
 // int N = gamodel->model->N;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  int MUTP = gamodel->tmutep;
  int i, in;
  int k;

  for(i = 0; i < NUMM; i++)
    {
      for(in = 0 ; in < n; in++)
	{
	  k = rand() % 100;
	  if(k < MUTP)
	    {
	      int b = rand()%2;
	      bitVector[i][in] = b; 
	    }
	}

    }
}
//void PrintModels(int NUMM, int N,  int n, int **bitVector, double **data, d
//ouble *ydata, double *sq)
void PrintModels(FILE* log, GAMODEL* gamodel)
{
  
  int NUMM = gamodel->sets;
  int N = gamodel->model->N;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  double** data = gamodel->model->parm;
  double* ydata = gamodel->model->yfunc;
  double* sq = gamodel->chisq;
  
  int i; int iN ,in; int gslFlg;
  gsl_multifit_linear_workspace *work;
  gsl_matrix *input, *cov;
  gsl_vector *y, *c;
  double chi;  

  input = gsl_matrix_alloc(N,n);
  cov = gsl_matrix_alloc(n,n);
  y = gsl_vector_alloc(N); c = gsl_vector_alloc(n);
  work = gsl_multifit_linear_alloc(N, n);

  /*For Each Model*/
  for(i = 0 ; i < NUMM; i++ )
  {
      fillMtxWithData(N, n, data, ydata, bitVector[i], input, y, c);

      trainLinModel(log, N, n, input, y, c, cov, work, chi);
      
      //gsl_multifit_linear_free(work); work = NULL;
      sq[i] = chi;
     
      if(log) {
          fprintf(log, "\n\n");
          for(in=0; in < n; in++)
              fprintf(log, "%le ", gsl_vector_get(c,in));
          fprintf(log, "|| %le \n", chi);
	
          fprintf(log, "\n");
      }
      chi = 0;

    } /*End Each Model*/

  gsl_matrix_free(input); gsl_matrix_free(cov);
  gsl_vector_free(y); gsl_vector_free(c);
  gsl_multifit_linear_free(work);
} /*End Find Models*/

void RunGA(FILE* log, GAMODEL* gamodel)
{    
    if(log) fprintf(log, "INIT BIT VECTOR\n");
    printBitVector(log, gamodel);
    int rindex;
    for(rindex = 0; rindex < gamodel->runs; rindex++)
    {
        if(log) fprintf(log, "STEP %d \n", rindex);   
        findModels(log, gamodel);
        AIC(log, gamodel);
        //MateSinglePoint(log, gamodel);
        MateSinglePointLink(log, gamodel);
        copyBitVector(gamodel);
        //if(log) printBitVector(log, gamodel);
        
        if(((rindex+1)%gamodel->tmute==0)&&(rindex<(gamodel->runs-2)))
        {
            //if(log) fprintf(log, "Mutation\n");
            //mutation(gamodel);
            mutationLink(gamodel);
            //if(log) printBitVector(log, gamodel);
        }
    }
    // Find the parameters of the final models and compute their AIC
    findModels(log, gamodel);
    AIC(log, gamodel);
    
    if(log) printBitVector(log, gamodel);
    PrintModels(log, gamodel);
}
void MateSinglePointLink(FILE* log, GAMODEL* gamodel)
{
  
  int NUMM = gamodel->sets;
 // int N = gamodel->model->N;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  int** tbitVector = gamodel->model->tbitVector;
  double* sq = gamodel->chisq;
  double* AIC = gamodel->AIC;
  
  int i, iN, in;
  int numElit, numpair; /*numElit are ones to keep and not mate*/
  //int crosspoint=0;
  numElit = (int)ceil(((double)NUMM)*(0.10));
  if((NUMM-numElit)%2 == 1){numElit++;} /*if odd take one more to keep*/
  numpair = (NUMM-numElit)/2;

  //if(log) fprintf(log, "Number Kept: %d \n ", numElit);
  //if(log) fprintf(log, "Number Changed: %d \n", numpair);

  int *top; int topindex = 0 ; int addindex = 0;
  top = (int *)calloc(numElit, sizeof(double));
  if(top == NULL){fprintf(log?log:stderr, "NULL pointer\n"); exit(1);}

  /*Rank Models*/ /*smallest to largest*/
  //qsort(sq, NUMM, sizeof(double), compare);
  for(i = 0 ; i < numpair; i++)
    { 
      int splitpoint = rand()%(n-1);
      //if(log) fprintf(log, "Split Point: %d\n", splitpoint);

      /*Find smallest*/
      double one = DBL_MAX, two = DBL_MAX;
      int ione = 0, itwo = 0;
      for(iN = 0 ; iN < NUMM; iN++)
	{
	  
	  if(AIC[iN] < one)
	    {
	      one = AIC[iN];
	      //AIC[iN] = DBL_MAX;
	      ione = iN;
	    }
	  else if (AIC[iN] < two)
	    {
	      two = AIC[iN];
	      //AIC[iN] = DBL_MAX;
	      itwo = iN;
	    }

	 }//find top two to mate
      
      
      // if(log) fprintf(log, "topindex: %d %d\n", topindex, topindex+1);
      //if(log) fprintf(log, "ione: %d[%le] itwo: %d[%le] \n", ione, sq[ione], itwo, sq[itwo]);
      AIC[ione] = DBL_MAX;
      AIC[itwo] = DBL_MAX;
      
      if(topindex < numElit)
	{
	  top[topindex] = ione; top[topindex+1] = itwo;
	  topindex = topindex+2;
	}
      /*Add first half*/
      for(in = 0 ; in <= splitpoint; in++)
	{
	  tbitVector[addindex][in]   = bitVector[ione][in];
	  tbitVector[addindex+1][in] = bitVector[itwo][in]; 
	} 
      /*Add second half*/
      for(in = splitpoint+1; in < n; in++)
	{
	  tbitVector[addindex][in]   = bitVector[itwo][in];
	  tbitVector[addindex+1][in] = bitVector[ione][in];
	}
      addindex = addindex+2;
    }//over number of pairs
  
  /*Fix pair-links.  Scan to find parameter with link and update the latter to follow*/
  for(i = 0; i < gamodel->model->L; i++)
  {
      int pkey = gamodel->model->llist->links[i][0];
      int linksize = gamodel->model->llist->nlinks[i];
      int* keylinks = gamodel->model->llist->links[i];
      
      for(in=0; in < addindex; in++)
      {
          int k = 0;
          int ppkey = tbitVector[in][pkey];
          for(k = 0; k < linksize; k++)
          {
              tbitVector[in][keylinks[k]]=ppkey;
          }
          
      }
 
  }
  
  for(i = 0 ; i < numElit; i ++)
    {
      for(in = 0 ; in < n; in++)
	{
	  tbitVector[addindex][in] = bitVector[top[i]][in];
	}
      addindex++;    
    }
  free(top);
}
void mutationLink(GAMODEL* gamodel)
{
  int NUMM = gamodel->sets;
 // int N = gamodel->model->N;
  int n = gamodel->model->cp;
  int** bitVector = gamodel->model->bitVector;
  int MUTP = gamodel->tmutep;
  int i, in;
  int k;

  for(i = 0; i < NUMM; i++)
    {
      for(in = 0 ; in < n; in++)
	{
	  k = rand() % 100;
	  if(k < MUTP)
	    {
	      int b = rand()%2;
	      bitVector[i][in] = b; 
	    }
	}

    }
  /*Clean up links--- this time we do by last */
  for(i = 0; i < gamodel->model->L; i++)
  {
      int pkey = gamodel->model->llist->links[i][0];
      int linksize = gamodel->model->llist->nlinks[i];
      int* keylinks = gamodel->model->llist->links[i];
      
      for(in=0; in < NUMM; in++)
      {
          int k = 0;
          int ppkey = bitVector[in][pkey];
          for(k = 0; k < linksize; k++)
          {
             bitVector[in][keylinks[k]]=ppkey;
          }
          
      }
 
  }
  
}




