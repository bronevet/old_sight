#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "sight_layout.h"
#include "process.h"
#include "process.C"
using namespace std;
using namespace sight;

// The stack of all the objects of each type that have been entered but not yet exited
map<string, list<void*> > stack;

/*scopeLayoutHandlerInstantiator scopeLayoutHandlerInstance;
graphLayoutHandlerInstantiator graphLayoutHandlerInstance;
traceLayoutHandlerInstantiator traceLayoutHandlerInstance;
valSelectorLayoutHandlerInstantiator valSelectorLayoutHandlerInstance;
attributesLayoutHandlerInstantiator attributesLayoutHandlerInstance;
#ifdef MFEM
#include "apps/mfem/mfem_layout.h"
mfemLayoutHandlerInstantiator mfemLayoutHandlerInstance;
#endif*/

//#define VERBOSE

int main(int argc, char** argv) {
  if(argc!=1 && argc!=2) { cerr<<"Usage: slayout fName"<<endl; exit(-1); }
  char* fName=NULL;
  if(argc==2) fName = argv[1];

  #ifdef VERBOSE
  cout << "layoutHandlers:\n";
  for(map<std::string, layoutEnterHandler>::iterator i=layoutHandlerInstantiator::layoutEnterHandlers->begin(); i!=layoutHandlerInstantiator::layoutEnterHandlers->end(); i++)
    cout << i->first << endl;
  #endif
 
  FILE* f;
  if(argc==1)
    f = stdin;
  else {
    f = fopen(fName, "r");
    if(f==NULL) { cerr << "ERROR opening file \""<<fName<<"\" for reading! "<<strerror(errno)<<endl; exit(-1); }
  }
  
  FILEStructureParser parser(f, 10000);
  
  pair<FILEStructureParser::tagType, const properties*> props = parser.next();
  while(props.second->size()>0) {
    if(props.first == FILEStructureParser::enterTag) {
      // If this is just text between tags, print it out 
      if(props.second->name() == "text")
        fprintf(f, "%s", properties::get(props.second->begin(), "text").c_str());
      // Else, if this is the entry into a new tag, process it
      else {
        // Call the entry handler of the most recently-entered object with this tag name
        // and push the object it returns onto the stack dedicated to objects of this type.
        if(layoutHandlerInstantiator::layoutEnterHandlers->find(props.second->name()) == layoutHandlerInstantiator::layoutEnterHandlers->end()) { cerr << "ERROR: no entry handler for \""<<props.second->name()<<"\" tags!" << endl; }
        assert(layoutHandlerInstantiator::layoutEnterHandlers->find(props.second->name()) != layoutHandlerInstantiator::layoutEnterHandlers->end());
        stack[props.second->name()].push_back((*layoutHandlerInstantiator::layoutEnterHandlers)[props.second->name()](props.second->begin()));
      }
    } else if(props.first == FILEStructureParser::exitTag) {
      // Call the exit handler of the most recently-entered object with this tag name
      // and pop the object off its stack
      assert(stack[props.second->name()].size()>0);
      if(layoutHandlerInstantiator::layoutEnterHandlers->find(props.second->name()) == layoutHandlerInstantiator::layoutEnterHandlers->end()) { cerr << "ERROR: no exit handler for \""<<props.second->name()<<"\" tags!" << endl; }
      assert(layoutHandlerInstantiator::layoutExitHandlers->find(props.second->name()) != layoutHandlerInstantiator::layoutExitHandlers->end());
      (*layoutHandlerInstantiator::layoutExitHandlers)[props.second->name()](stack[props.second->name()].back());
      stack[props.second->name()].pop_back();
    }
    props = parser.next();
  }
  
  if(argc==2)
    fclose(f);
}
