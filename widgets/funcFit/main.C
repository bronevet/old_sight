#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "model.h"
#include "ga.h"

extern void parseInput(char* fName, MODEL*, GAMODEL*);
extern void dataInput(char* fName, MODEL *);
extern void functionSetup(FILE* log, MODEL*, GAMODEL*);
extern void printParm(FILE* log, double **, int , int);
MODEL* initModel(void);
GAMODEL* initGAModel(void);
void printY(FILE* log, double *y, int n);

int main(int argc, char** argv) 
{
    if(argc<3) { fprintf(stderr, "Usage: ga modelFName dataFName\n"); exit(-1); }
    
    MODEL* model;
    model = initModel();
    GAMODEL* gamodel;
    gamodel = initGAModel();
    parseInput(argv[1], model,gamodel);
    dataInput(argv[2], model);
    printParm(stdout, model->parm, model->N, model->p);
    
    //printf("Link Size: %d Links: %d %d\n", model->llist->nlinks[0], model->llist->links[0][0], model->llist->links[0][1]);
    //printf("Functions 0: %d %d   Functions 1: %d %d \n", model->llist->flist[0][0], model->llist->flist[0][1], model->llist->flist[1][0], model->llist->flist[1][1]);
    
    functionSetup(stdout, model, gamodel);

    //printf("Link Size: %d Links: %d %d\n", model->llist->nlinks[0], model->llist->links[0][0], model->llist->links[0][1]);
    //printf("Functions 0: %d %d   Functions 1: %d %d \n", model->llist->flist[0][0], model->llist->flist[0][1], model->llist->flist[1][0], model->llist->flist[1][1]);
    
    //printParm(model->parm, model->N, model->p);
    //printParm(gamodel->model->parm, gamodel->model->N, gamodel->model->cp);
    //printY(model->yfunc, model->N);
    //printY(gamodel->model->yfunc, gamodel->model->N);
    
    
    printf("Number of Links: %d  %d\n", model->L, gamodel->model->L);
    printf("Size of GALINKS: %d %d\n", gamodel->model->llist->nlinks[0], gamodel->model->llist->nlinks[1]);
    printf("Link 1: %d  Link 2: %d \n", gamodel->model->llist->links[0][0], gamodel->model->llist->links[0][1] );
    printf("Link 1: %d  Link 2: %d \n", gamodel->model->llist->links[1][0], gamodel->model->llist->links[1][1] );
    /*Done loading*/
    
    initBitVector(gamodel);
    printf("bitvector: %d %d\n", gamodel->model->bitVector[0][0], gamodel->model->bitVector[0][1]);
    RunGA(stdout, gamodel);

    /*Setup Functions on Var*/
    return (EXIT_SUCCESS);
}
MODEL* initModel(void)
{
    MODEL* model = (MODEL*) malloc(sizeof(MODEL));
    model->llist = (FLIST*) malloc(sizeof(FLIST));
    model->N = -1;
    model->p = -1;
    model->cp = -1;
    return model;
}
GAMODEL* initGAModel(void)
{
    GAMODEL* gamodel = (GAMODEL*) malloc(sizeof(GAMODEL));
    gamodel->runs = 200;
    gamodel->sets = 20;
    gamodel->seed=0;
    gamodel->fitfunction = 0;
    gamodel->tmate=0;
    gamodel->tmute=1; //Mutation every two steps
    gamodel->tmutep = 20;
    gamodel->decho='y';
    gamodel->model = initModel();
    return gamodel;
}

void printParm(FILE* log, double **parm, int n, int p)
{
    int i,j;
    for(i=0; i<n; i++)
    {
        for(j=0;j<p;j++)
        {
            fprintf(log, "%f ", parm[i][j]);
        }
        fprintf(log, "\n");
    }
}
void printY(FILE* log, double *y, int n)
{
    fprintf(log, "-------------Y-VALUES ---------------\n");
    int i=0;
    for(i=0; i<n; i++)
    {
       fprintf(log, "%f \n", y[i]);
    }
    fprintf(log, "\n");
}

