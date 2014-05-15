#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include "sight_common.h"
#include "main.h"
#include "model.h"
#include "ga.h"
#include <string.h>
#include <errno.h>

using namespace sight;
using namespace sight::common;
using namespace std;

extern void parseInput(char* fName, MODEL*, GAMODEL*);
extern void dataInput(char* fName, MODEL *);
extern void functionSetup(FILE* log, MODEL*, GAMODEL*);
extern void printParm(FILE* log, double **, int , int);
MODEL* initModel(void);
GAMODEL* initGAModel(void);
void printY(FILE* log, double *y, int n);
void PrintBestModel(FILE* log, GAMODEL* gamodel, char* label);

class dataReader : public traceFileReader {
  // Maps each observation attribute to its type
  map<string, attrValue::valueType> obsTypes;
  
  MODEL* model;
  public:
  dataReader(MODEL* model): model(model) {
    // Allocate memory for the model
    model->yfunc = (double *)calloc(model->N, sizeof(double));
    model->parm = (double **)malloc(model->N*sizeof(double *));
    for(int i=0; i<model->N; i++)
      model->parm[i] = (double *)calloc(model->p, sizeof(double));
  }
  
  void operator()(const std::map<std::string, attrValue>& ctxt,
                  const std::map<std::string, attrValue>& obs,
                  const std::map<std::string, int>& anchor, 
                  int lineNum) {
    // Ensure that all observation fields have the same type
    for(std::map<std::string, attrValue>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
      // If this is the first observation, record the type
      if(lineNum==1) obsTypes[o->first] = o->second.getType();
      // Otherwise, check for type consistency
      else if(obs.find(o->first) == obs.end()) { cerr << "ERROR: Inconsistent observation attributes! Attribute \""<<o->first<<"\" was provided on line "<<lineNum<<" for the first time!"<<endl; exit(-1); }
      else if(obsTypes[o->first] != o->second.getType()) { cerr << "ERROR: Inconsistent types of observation attribute \""<<o->first<<"\"! On line "<<lineNum<<" type is "<<attrValue::type2str(o->second.getType())<<" but on prior lines it is "<<attrValue::type2str(obsTypes[o->first])<<"!"<<endl; exit(-1); }
    }
    if(obs.size() != obsTypes.size()) { cerr << "ERROR: observation attributes on line "<<lineNum<<" inconsistent with those on earlier lines!"<<endl; exit(-1); }
  
    assert(obs.size()==1);
    map<string, attrValue>::const_iterator o=obs.begin();
    if(obsTypes[o->first] == attrValue::intT || obsTypes[o->first] == attrValue::floatT) {
      switch(obsTypes[o->first]) {
        case attrValue::intT:   model->yfunc[lineNum-1] = o->second.getInt();   break;
        case attrValue::floatT: model->yfunc[lineNum-1] = o->second.getFloat(); break;
        default: exit(-1);
      }
    }
    //printf("yfunc[%d]=%le\n", lineNum-1, model->yfunc[lineNum-1]);
    
    assert(model->p == ctxt.size());
    int cIdx=0;
    //printf("parm[%d]=", lineNum-1);
    for(std::map<std::string, attrValue>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++, cIdx++) {
      switch(c->second.getType()) {
        case attrValue::intT:   model->parm[lineNum-1][cIdx] = c->second.getInt();   break;
        case attrValue::floatT: model->parm[lineNum-1][cIdx] = c->second.getFloat(); break;
        default: exit(-1);
      }
      //printf("%le ", model->parm[lineNum-1][cIdx]);
    }
    //printf("\n");
  }
};

int main(int argc, char** argv) {
  if(argc!=4 && argc!=5) { cerr << "Usage: filter cfgFName dataFName label [logFName]"<<endl<<"Command Line:"; for(int i=0; i<argc; i++) cerr << "\""<<argv[i]<<"\" "; cerr<<endl; exit(-1); }
  char* cfgFName  = argv[1];
  char* dataFName = argv[2];
  char* label     = argv[3];
  char* logFName = NULL;
  if(argc==5) logFName = argv[4];
  
  FILE* log=NULL;
  if(logFName) {
    log = fopen(logFName, "w"); 
    if(log==NULL) { fprintf(stderr, "ERROR opening file \"%s\" for writing! %s", logFName, strerror(errno)); exit(-1); }
  }

  MODEL* model;
  model = initModel();
  GAMODEL* gamodel;
  gamodel = initGAModel();
  parseInput(cfgFName, model,gamodel);
  
  // If there is not enough data to train on, quit
  if(model->N <=1) return (EXIT_SUCCESS);
 
  if(log) fprintf(log, "Data\n");
  dataReader reader(model);
  readTraceFile(string(dataFName), reader);

  if(log) printParm(log, model->parm, model->N, model->p);

  functionSetup(log, model, gamodel);
  
  if(gamodel->model->cp<=1) { fprintf(stderr, "ERROR: Insufficient number of model terms provided: %d.\n", model->cp); exit(-1); }

  if(log) fprintf(log, "Number of Links: %d  %d\n", model->L, gamodel->model->L);
  if(log) fprintf(log, "Size of GALINKS: %d %d\n", gamodel->model->llist->nlinks[0], gamodel->model->llist->nlinks[1]);
  if(log) fprintf(log, "Link 1: %d  Link 2: %d \n", gamodel->model->llist->links[0][0], gamodel->model->llist->links[0][1] );
  if(log) fprintf(log, "Link 1: %d  Link 2: %d \n", gamodel->model->llist->links[1][0], gamodel->model->llist->links[1][1] );
  /*Done loading*/

  initBitVector(gamodel);
  if(log) fprintf(log, "bitvector: %d %d\n", gamodel->model->bitVector[0][0], gamodel->model->bitVector[0][1]);
  RunGA(log, gamodel);

  PrintBestModel(log, gamodel, label);
  
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
    if(log==NULL) return;
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
    if(log==NULL) return;
    fprintf(log, "-------------Y-VALUES ---------------\n");
    int i=0;
    for(i=0; i<n; i++)
    {
       fprintf(log, "%f \n", y[i]);
    }
    fprintf(log, "\n");
}

void PrintBestModel(FILE* log, GAMODEL* gamodel, char* label)
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

  // Find the smallest value in gamodel->chisq, which should contain the
  // AIC value of each model.
  double minAIC=DBL_MAX;
  int minAICIdx=-1;
  /*For Each Model*/
  for(i = 0 ; i < NUMM; i++ )
  {
      if(gamodel->AIC[i] < minAIC) {
        minAIC = gamodel->AIC[i];
        minAICIdx = i;
      }
  }
  
  // Train a linear model for this set of features to get each term's coefficients
  fillMtxWithData(N, n, data, ydata, bitVector[minAICIdx], input, y, c);
  trainLinModel(log, N, n, input, y, c, cov, work, chi);

  map<string, attrValue> ctxt;
  for(int in=0; in < n; in++)
      ctxt[gamodel->model->funcName[in]] = attrValue(gsl_vector_get(c, in));
  
  map<string, attrValue> obs;
  obs["chiSq"] = attrValue(chi);
  obs["AIC"]   = attrValue(minAIC);
  obs["label"] = label;
  
  map<string, int> anchor;
  
  cout << serializeTraceObservation(ctxt, obs, anchor) << endl;
  
  gsl_matrix_free(input); gsl_matrix_free(cov);
  gsl_vector_free(y); gsl_vector_free(c);
  gsl_multifit_linear_free(work);
} /*End Find Models*/
