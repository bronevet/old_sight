#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "process.h"
#include "process.C"
#include <iostream>
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
  	// Make sure the file exists and is not a directory
  	struct stat s;
		int err = stat(fName, &s);
		if(-1 == err) {
		    if(ENOENT == errno) { cerr << "ERROR: path \""<<fName<<"\" does not exist!"<<endl; exit(-1); }
		    else                { perror("stat"); exit(-1); }
		} else {
		    if(S_ISDIR(s.st_mode)) { cerr << "ERROR: path \""<<fName<<"\" is a directory!"<<endl; exit(-1); }
		    else {
		       // exists but is no dir
		    }
		}
  	
    f = fopen(fName, "r");
    if(f==NULL) { cerr << "ERROR opening file \""<<fName<<"\" for reading! "<<strerror(errno)<<endl; exit(-1); }
  }

  
  FILEStructureParser parser(f, 10000);
  
  layoutStructure(parser);

  if(argc==2)
    fclose(f);
}
