#include <stdio.h>
#include <string.h>

#include "main.h"
#include "model.h"

void updateLinkedFunc(MODEL*, int, int,int);
void parseInput(char*, MODEL*, GAMODEL*);

void parseInput(char* fName, MODEL* model, GAMODEL* gamodel)
{
    int i=0;    
    FILE *fp = NULL;
    fp = fopen(fName, "r");
    if(fp == NULL)
    {
        printf("Error: Input File is NULL\n");
        return;
    }
    
    while(!feof(fp))
    {
        char sstring[8];
        fscanf(fp, "%s", sstring);
        if(strcmp(sstring,"N") == 0){fscanf(fp, "%d", &(model->N));}
        if(strcmp(sstring,"p") == 0){ /*---------------parameters------------*/
            fscanf(fp, "%d", &(model->p));
            model->llist->flist = (int **)malloc(model->p*sizeof(int *));
            model->parmName = (char**)malloc(model->p * sizeof(char*));
            for(i=0; i< model->p; i++)
            {
                //printf("CALLED-------------  \n");
                model->llist->flist[i] = (int *)calloc(NFUN,sizeof(int));
                int j;
                model->llist->flist[i][0] = 1;
                for(j=1; j<NFUN; j++)
                {
                    //model->llist->flist[i][j] = 0;
                }
            }
        }
        if(strcmp(sstring,"pname") == 0){ /*---------------parameter name------------*/
          int pID;
          fscanf(fp, "%d", &pID);
          model->parmName[pID] = (char*)malloc(sizeof(char) * 1000);
          fscanf(fp, "%s", model->parmName[pID]);
        }
        if(strcmp(sstring,"L") == 0){ /*---------------Links----------------*/
            fscanf(fp, "%d", &(model->L));
            model->llist->nlinks = (int *)calloc(model->L, sizeof(int));
            int *sint = model->llist->nlinks;
            model->llist->links = (int **)malloc(model->L*sizeof(int *));
            for(i = 0; i < model->L; i++)
            {
                fscanf(fp,"%d", &(sint[i]));   
                if(sint[i] <= 0){printf("Error: Link Size\n"); exit(0);}
                model->llist->links[i] = (int *)malloc(sint[i]*sizeof(int));
            }
        }
        if(strcmp(sstring,"l") == 0){ /*------------------Populate links------------------*/
            if(model->llist->nlinks==NULL){printf("Error: No links have been created\n"); exit(0);}
            int selected= 0;
            fscanf(fp, "%d", &selected);
            if(selected > model->L){printf("Error: Link exceeds limit\n"); exit(0);}
            for(i=0; i<model->llist->nlinks[selected]; i++)
            {
                //printf("Step: %d \n", i);
                fscanf(fp, "%d", &(model->llist->links[selected][i]));
            }
        }
        if(strcmp(sstring,"F") == 0){
            //printf("CALLED F-----------\n");
            int functype = 0, par = 0, fval=0;
            fscanf(fp, "%d", &par);
            fscanf(fp, "%d", &functype);
            fscanf(fp, "%d", &fval);
            //fscanf(fp, "%d", &par);
            //printf("par=%d, functype=%d, fval=%d\n", par, functype, fval);
            model->llist->flist[par][functype]=fval;
            updateLinkedFunc(model, par, functype,fval);
        } 
       
        if(strcmp(sstring, "GAT") == 0){fscanf(fp,"%d", &(gamodel->runs));}
        if(strcmp(sstring, "GAS") == 0){fscanf(fp,"%d", &(gamodel->sets));}
        if(strcmp(sstring, "GARS") == 0){fscanf(fp,"%d", &(gamodel->seed));}
        if(strcmp(sstring, "GAFIT") == 0){fscanf(fp,"%d", &(gamodel->fitfunction));}
        if(strcmp(sstring, "GAMAT") == 0){fscanf(fp, "%d", &(gamodel->tmate));}
        if(strcmp(sstring, "GAMUT") == 0){fscanf(fp, "%d", &(gamodel->tmute));}
                
    }//End while EOF
}
/*Function used to update function of all links*/
void updateLinkedFunc(MODEL* model, int parm, int func, int fval)
{
    //Search for links
    int linkloc[32];
    int i,j, counter=0;
    for(i = 0; i< model->L; i++)
    {
        for(j=0; j< model->llist->nlinks[i]; j++)
        {
            if(model->llist->links[i][j] == parm){linkloc[counter]=i; counter++;}
        }
    }
    //Update 
    for(i=0; i<counter; i++)
    {
        for(j=0; j < model->llist->nlinks[linkloc[i]]; j++)
        {
            model->llist->flist[model->llist->links[linkloc[i]][j]][func] = fval;
        }
    }
}

