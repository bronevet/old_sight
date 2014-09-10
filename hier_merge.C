#include <vector>
#include <map>
using namespace std;

#include "sight_structure.h"
using namespace sight;
using namespace sight::structure;

#include "sight_merge.h"
using namespace sight::merge;

int main(int argc, char** argv) {
  if(argc<3) { cerr<<"Usage: hier_merge outDir [fNames]"<<endl; exit(-1); }
  vector<baseStructureParser<FILE>*> fileParsers;
  const char* outDir = argv[1];
  //mergeType mt = str2MergeType(string(argv[2]));
  for(int i=2; i<argc; i++) {
    fileParsers.push_back(new FILEStructureParser(argv[i], 10000));
  }
  
  #ifdef VERBOSE
  SightInit(argc, argv, "hier_merge", txt()<<outDir<<".hier_merge");
  dbg << "#fileParserRefs="<<fileParsers.size()<<endl;
  #else
  SightInit_LowLevel();
  #endif
  
  // Set the working directory in the dbgStreamMerger class (it is a static variable). This must be done before an instance of this class is created
  // since the first and only instance of this class will read this working directory and write all output there.
  dbgStreamMerger::workDir = string(outDir);
  
  //dbgStreamStreamRecord::enterBlock(inStreamRecords);
#ifdef VERBOSE
  graph g;
  anchor outgoingA; 
#endif

  MergeState state(fileParsers
                   #ifdef VERBOSE
                   , g, anchor::noAnchor, outgoingA
                   #endif
                  );
  state.merge();
  
  // Close all the parsers and their files
  for(vector<baseStructureParser<FILE>*>::iterator p=fileParsers.begin(); p!=fileParsers.end(); p++)
    delete *p;
  
  return 0;
}
