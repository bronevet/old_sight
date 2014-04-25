
#include "main.h"
#include "model.h"

void dataInput(char* fName, MODEL* model)
{
    int i,j;
    FILE *fp = fopen(fName, "r");
    if(fp == NULL){printf("Error: Cannot open data file\n"); exit(0);}
    /*Malloc space for data*/
    model->yfunc = (double *)calloc(model->N, sizeof(double));
    model->parm = (double **)malloc(model->N*sizeof(double *));
    for(i=0; i<model->N; i++)
    {
        model->parm[i] = (double *)calloc(model->p, sizeof(double));
    }
    //Collect data
    for(i=0; i < model->N; i++)
    {
        //double tfloat = 0;
        //fscanf(fp,"%lg", &tfloat);
        //printf("yvalue %")
        fscanf(fp, "%lg", &(model->yfunc[i]));
        printf("yvalue: %f \n", model->yfunc[i]);
        for(j=0; j<model->p; j++)
        {
            fscanf(fp, "%lg", &(model->parm[i][j]));
        }
    }
}
