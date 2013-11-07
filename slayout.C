#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "process.C"
using namespace std;
using namespace sight;
using namespace sight::layout;

//#define VERBOSE

int main(int argc, char** argv) {
  if(argc!=1 && argc!=2) { cerr<<"Usage: slayout fName"<<endl; exit(-1); }
  char* fName=NULL;
  if(argc==2) fName = argv[1];
 
  FILE* f;
  if(argc==1)
    f = stdin;
  else {
    f = fopen(fName, "r");
    if(f==NULL) { cerr << "ERROR opening file \""<<fName<<"\" for reading! "<<strerror(errno)<<endl; exit(-1); }
  }
  
  FILEStructureParser parser(f, 10000);
  
  layoutStructure(parser);

  if(argc==2)
    fclose(f);
}
