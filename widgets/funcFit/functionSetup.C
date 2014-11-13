#include "main.h"
#include "model.h"
#include "ga.h"
#include <math.h>
#include <sstream>
using namespace std;

/*Go through do the needed transformation and put into ga*/
void functionSetup(FILE* log, MODEL *model, GAMODEL *gamodel)
{
    int i;
    /*Copy n and p*/
    gamodel->model->N = model->N;
    gamodel->model->p = model->p;
    
    /*Scan through and find number of end parameters*/
    int tcp = 0;
    for(i = 0; i < model->p; i++)
    {
        int j;
        int numFunc = 0;
        for(j=0; j < NFUN; j++)
        {
            if(log) fprintf(log, "p: %d  f: %d  val: %d ", i, j, model->llist->flist[i][j]);
            if(model->llist->flist[i][j] > 0)
            {
                if(log) fprintf(log, "YES");
                tcp++;
                numFunc++;                
            }
            if(log) fprintf(log, "\n");
        }
    }
    /*init the model*/
    gamodel->model->cp =  tcp;
    if(log) fprintf(log, "Number of new parameters: %d\n", tcp);
    
    /*Create flist that will keep the mapping*/
    gamodel->model->llist->flist  =  (int **)malloc(model->p*sizeof(int*));
    for(i=0; i<model->p; i++)
    {
        gamodel->model->llist->flist[i] = (int*) calloc(NFUN,sizeof(int));          
    }
    
    /*Find how many links there will now be*/
    int totallinks = 0;
    int* linkcopy = (int *)calloc(model->L, sizeof(int));
    for(i=0; i < model->L; i++)
    {
       /*only need to test if first parm in a link has multiple functions*/
        int tperm = model->llist->links[i][0];
        int j;
        for(j=0; j < NFUN; j++)
        {
            if(model->llist->flist[tperm][j] > 0)
            {
                linkcopy[i]++;
            }
        }
        totallinks += linkcopy[i];
    }
    
    int index = 0;
    if(log) fprintf(log, "total links: %d\n", totallinks);
    gamodel->model->L = totallinks;
    
    
    /*make map*/
    int glindex = 0;
    for(i=0; i<model->p;i++)
    {
        int j;
        for(j=0; j < NFUN; j++)
        {
            if(model->llist->flist[i][j]>0)
            {
                gamodel->model->llist->flist[i][j] = glindex;
                glindex++;
            }   
        }
     
    }
    
   
    gamodel->model->llist->nlinks = (int*)calloc(totallinks,sizeof(int));
    gamodel->model->llist->links = (int **)malloc(totallinks*sizeof(int*));
    for(i=0; i < model->L; i++)
    {
        int linkcount = linkcopy[i];
        int j;
        for(j =0; j < linkcount; j++)
        {
           gamodel->model->llist->links[index] = (int *)malloc(model->llist->nlinks[i]*sizeof(int));
           gamodel->model->llist->nlinks[index] = model->llist->nlinks[i];
           
           int k;
           for(k = 0 ; k < model->llist->nlinks[i]; k++)
           {
               int oldmap = model->llist->links[i][k];
               int newmap =0;
               int nonzc = 0;
               int ii;
               for(ii = 0 ; ii <NFUN; ii++)
               {
                   if(model->llist->flist[oldmap][ii]>0)
                   {
                      if(nonzc == j)
                      {
                          nonzc = ii;
                          break;
                      }
                      nonzc++;
                   }
               }
               newmap = gamodel->model->llist->flist[oldmap][nonzc];
               gamodel->model->llist->links[index][k] = newmap; 
                       
           }
           index++;
           glindex++;
        }  
        glindex++;
    }
    if(log) fprintf(log, "Global index %d\n", glindex);
    free(linkcopy);
    
 
    gamodel->model->funcName = new string[gamodel->model->cp];//(char**)malloc(gamodel->model->cp * sizeof(char*));
    
    gamodel->model->parm = (double**)malloc(model->N*sizeof(double*));
    for(i=0; i<gamodel->model->N; i++)
    {
        gamodel->model->parm[i] = (double*)calloc(gamodel->model->cp,sizeof(double));
    }
    gamodel->model->yfunc = (double*)malloc(model->N*sizeof(double));
    for(i=0; i<gamodel->model->N; i++)
    {
        gamodel->model->yfunc[i] = model->yfunc[i];
    }
    
    
    
    /*Now do the transformation*/
    /*loop through parm and trans and copy */
   
    index = 0;
    for(i=0; i<model->p; i++)
    {
        int j;
        for(j=0; j<NFUN; j++)
        {
            if(j==0 && model->llist->flist[i][j] > 0) //no poly
            {
                int k;
                for(k=0; k<gamodel->model->N; k++)
                {
                    gamodel->model->parm[k][index] = model->parm[k][i];
                }
                gamodel->model->funcName[index] = model->parmName[i];
                index++;
            }
            if(j==1 && model->llist->flist[i][j] >0 ) //pow
            {
                int k;
                for(k=0; k<gamodel->model->N; k++)
                {
                    gamodel->model->parm[k][index] = pow(model->parm[k][i], (double)model->llist->flist[i][j]);
                }
                ostringstream s; s << model->parmName[i]<<"^"<<model->llist->flist[i][j];
                gamodel->model->funcName[index] = s.str();
                index++;
            }
            if(j==2 && model->llist->flist[i][j]>0)
            {
                int k;
                for(k=0; k<(gamodel->model->N);k++)
                {
                    gamodel->model->parm[k][index] = exp(((double)model->llist->flist[i][j])*model->parm[k][i]);
                }
                ostringstream s; s << "exp("<<model->parmName[i]<<", "<<model->llist->flist[i][j]<<")";
                gamodel->model->funcName[index] = s.str();
                index++;
            }
            if(j==3 && model->llist->flist[i][j]>0)
            {
                int k;
                for(k=0; k<gamodel->model->N;k++)
                {
                    gamodel->model->parm[k][index] = (1.0/(model->parm[k][i]));
                }
                ostringstream s; s << "1/"<<model->parmName[i];
                gamodel->model->funcName[index] = s.str();
                index++;
            }
        }
    }
    
}
