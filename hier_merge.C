#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "sight_structure.h"
#include "utils.h"
#include "process.h"
#include "process.C"
using namespace std;
using namespace sight;
using namespace sight::structure;

#define VERBOSE

// parsers - Vector of parsers from which information will be read
// nextTag - If merge() is called recursively after a given tag is entered on some but not all the parsers,
//    contains the information of this entered tag.
// variantStackDepth - depth of calls to merge(). Each level of recursion corresponds to some structural
//    difference between the different output streams where in one parser we enter a tag and in another
//    we do not. For each such difference we call merge() recursively on all the parsers that enter a tag
//    and exit this call when we exit this tag.
// out - stream to which data will be written
// Returns a bool vector that records whether each parser that was passed in is still active.
vector<bool> merge(vector<FILEStructureParser*>& parsers, 
                   vector<pair<structureParser::tagType, properties::iterator> >& nextTag, 
                   int variantStackDepth,
                   structure::dbgStream& out
                   #ifdef VERBOSE
                   , string indent
                   #endif
                   );

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

class MergeFunctorBase {
  public:
  virtual sight::structure::Merger* merge(const std::vector<std::pair<structureParser::tagType, properties::iterator> >& tags)=0;
};

template<class M>
class MergeFunctor : public MergeFunctorBase {
  public:
  sight::structure::Merger* merge(const std::vector<std::pair<structureParser::tagType, properties::iterator> >& tags) {
    return new M(tags);
  }
};

map<string, MergeFunctorBase*> mergers;



int main(int argc, char** argv) {
  if(argc<3) { cerr<<"Usage: slayout [fNames]"<<endl; exit(-1); }
  vector<FILEStructureParser*> fileParsers;
  for(int i=1; i<argc; i++) {
    fileParsers.push_back(new FILEStructureParser(argv[i], 10000));
  }
  #ifdef VERBOSE
  cout << "#fileParserRefs="<<fileParsers.size()<<endl;
  #endif
  
  mergers["sight"]    = new MergeFunctor<dbgStreamMerger>();
  mergers["indent"]   = new MergeFunctor<IndentMerger>();
  mergers["scope"]    = new MergeFunctor<ScopeMerger>();
  mergers["graph"]    = new MergeFunctor<GraphMerger>();
  mergers["trace"]    = new MergeFunctor<TraceMerger>();
  //mergers["traceObs"] = new MergeFunctor<TraceObsMerger>();
  
  vector<pair<structureParser::tagType, properties::iterator> > emptyNextTag;
  merge(fileParsers, emptyNextTag, 0, dbg
        #ifdef VERBOSE
        , "    "
        #endif
        );
  
  // Close all the parsers and their files
  for(vector<FILEStructureParser*>::iterator p=fileParsers.begin(); p!=fileParsers.end(); p++)
    delete *p;
  
  return 0;
}

// parsers - Vector of parsers from which information will be read
// nextTag - If merge() is called recursively after a given tag is entered on some but not all the parsers,
//    contains the information of this entered tag.
// variantStackDepth - depth of calls to merge(). Each level of recursion corresponds to some structural
//    difference between the different output streams where in one parser we enter a tag and in another
//    we do not. For each such difference we call merge() recursively on all the parsers that enter a tag
//    and exit this call when we exit this tag.
// out - stream to which data will be written
// Returns a bool vector that records whether each parser that was passed in is still active.
vector<bool> merge(vector<FILEStructureParser*>& parsers, 
                   vector<pair<structureParser::tagType, properties::iterator> >& nextTag, 
                   int variantStackDepth,
                   structure::dbgStream& out
                   #ifdef VERBOSE
                   , string indent
                   #endif
                    ) {
  #ifdef VERBOSE
  cout << indent << "#parsers="<<parsers.size()<<", variantStackDepth="<<variantStackDepth<<", dir="<<out.workDir<<endl;
  #endif
                    
  // If this is not the root call to merge (variantStackDepth>1), nextTag is set to the tag entry items that were
  // just read on every parser in parsers. Otherwise, nextTag is empty
  assert((variantStackDepth==0 && nextTag.size()==0) ||
         (variantStackDepth>0  && nextTag.size()==parsers.size()));
  
  // The current depth of the nesting stack of tags that have been entered but not yet exited. If nextTag
  // is not empty, stackDepth is set to 1 to account for the entry tag read by the caller of merge(). 
  // Otherwise stackDepth is set to 0.
  int stackDepth=(variantStackDepth==0? 0: 1);

  // Records whether we're ready to read another tag from each parser
  vector<bool> readyForTag(parsers.size(), true);
  
  // Records whether each parser is still active or whether we've reached its end and the number of active parsers
  vector<bool> activeParser(parsers.size(), true);
  int numActive = parsers.size();
  
  // Counts the number of times we recursively consider variant tags. The output of processing
  // such variants needs to be written to a separate uniquely-named location. These unique names
  // are generated using this counter.
  int subDirCount=0;
  
  // Maps the next observed tag name/type to the input streams on which tags that match this signature were read
  map<pair<structureParser::tagType, string>, list<int> > tag2stream;
  
  while(numActive>0) {
    #ifdef VERBOSE
    cout << indent << "=================================\n";
    #endif
    
    // Read the next item from each parser
    int parserIdx=0;
    // Records whether we read text on any parser. We alternate between reading text and reading tags
    // and if text is read from some but not all parsers, the contributions from the other parsers are 
    // considered to be the empty string.
    bool anyText = false;
    for(vector<FILEStructureParser*>::iterator p=parsers.begin(); p!=parsers.end(); p++, parserIdx++) {
      #ifdef VERBOSE
      cout << indent << "readyForTag["<<parserIdx<<"]="<<readyForTag[parserIdx]<<", activeParser["<<parserIdx<<"]="<<activeParser[parserIdx]<<endl;
      #endif
      
      // If we're ready to read a tag on this parser
      if(readyForTag[parserIdx] && activeParser[parserIdx]) {
        pair<structureParser::tagType, const properties*> props = (*p)->next();
        #ifdef VERBOSE
        cout << indent << parserIdx << ": "<<const_cast<properties*>(props.second)->str()<<endl;
        #endif
        
        // If we've reached the end of this parser's data
        if(props.second->size()==0) {
          activeParser[parserIdx] = false;
          numActive--;
        } else {
          // Record the properties of the newly-read tag
          nextTag.push_back(make_pair(props.first, props.second->begin()));
          // Group this parser with all the other parsers that just read a tag with the same name and type (enter/exit)
          tag2stream[make_pair(props.first, props.second->name())].push_back(parserIdx);
                    
          // Record whether we read a text tag on any parser
          anyText = (props.second->name() == "text") || anyText;
        }
      }
    }
    
    // If we read text on any parser, emit the text immediately
    if(anyText) {
      assert(tag2stream.find(make_pair(properties::enterTag, "text")) != tag2stream.end());
      
      // Parsers on which we read text
      const list<int>& textParsers = tag2stream[make_pair(properties::enterTag, "text")]; 
      assert(textParsers.size()>0);
      for(list<int>::const_iterator i=textParsers.begin(); i!=textParsers.end(); i++) {
        assert(activeParser[*i]);
        
        // !!! Need to do proper merge
        out << "{"<<properties::get(nextTag[*i].second, "text")<<"}"<<endl;
        
        // We're ready to read a new tag on this parser
        readyForTag[*i] = true;
      }
      
      // Reset the tag2stream key associated with text reading
      tag2stream.erase(make_pair(properties::enterTag, "text"));
        
      // We'll now repeat the loop and read more tags on all the parsers from which we just read text
      
    // If we only entered or exited tags
    } else {
      // Group all the streams with the same next tag name/type. Since all parsers must have the same
      // stack of tags that have been entered, if we read the exit of a tag on any parser(s), they all 
      // must exit the same tag. However, different parsers may enter different tags.
      //
      // If there is just one group, then we merge the properties of the tags (all same type)
      // read from all the parsers, and perform the enter/exit action of this group
      if(tag2stream.size()==1) {
        assert(mergers.find(tag2stream.begin()->first.second) != mergers.end());
    
        // Merge the properties of all tags
        Merger* m = mergers[tag2stream.begin()->first.second]->merge(nextTag);
        #ifdef VERBOSE
        cout << indent << "merged="<<m->getProps().str()<<endl;
        if(tag2stream.begin()->first.second == "sight")
          cout << indent << "dir="<<dbg.workDir<<endl;
        #endif
        
        // Perform the common action
        if(tag2stream.begin()->first.first == properties::enterTag) {
          stackDepth++;
          out.enter(m->getProps());
        } else {
          stackDepth--;
          out.exit(m->getProps());
          
          // If we've exited out of the highest-level tag at this variant level, exit out to the parent
          // call to merge() unless this is the root call to merge()
          if(stackDepth==0 && variantStackDepth>0)
            return activeParser;
        }
        
        delete(m);

        // Record that we're ready for more tags on all the parsers
        for(vector<bool>::iterator i=readyForTag.begin(); i!=readyForTag.end(); i++)
          *i = true;
        
        // Reset tag2stream since we'll be reloading it based on the next tag read from each parser
        tag2stream.clear();
      }
      
      // If there are multiple groups consider the ones that entered a tag. These clearly diverge from
      // each other and from groups that exited a tag since all the parsers must have entered this
      // tag and now some are trying to exit while others are trying to enter another, more deeply-nested tag.
      // Thus, we recursively call merge() on each group that is trying to enter a tag, allowing it to process
      // this tag until it is exited. The groups that are trying to exit a tag are left alone to wait for the
      // enterers to complete. Eventually all the parsers that entered a tag will exit it and thus exit the 
      // merge() call. At this point we'll read the next tag from them and see if they can be merged.
      else {
        // Iterate over all the groups that entered a tag
        for(map<pair<structureParser::tagType, string>, list<int> >::iterator ts=tag2stream.begin();
            ts!=tag2stream.end(); ) {
          if(ts->first.first == properties::enterTag) {
            assert(ts->second.size()>0);
            
            // Contains the parsers of just this group
            vector<FILEStructureParser*> groupParsers;
            for(list<int>::const_iterator i=ts->second.begin(); i!=ts->second.end(); i++) {
              assert(activeParser[*i]);
              groupParsers.push_back(parsers[*i]);
            }
            
            // Contains the next read tag of just this group
            vector<pair<structureParser::tagType, properties::iterator> > groupNextTag;
            for(list<int>::const_iterator i=ts->second.begin(); i!=ts->second.end(); i++)
              groupNextTag.push_back(nextTag[*i]);
              
            // <<< Recursively Call merge()
              string subDir = txt()<<out.workDir<<"/"<<"var_"<<subDirCount;
              
              // Create the directory structure for the structural information
              // Main output directory
              createDir(subDir, "");
            
              // Directory where client-generated images will go
              string imgDir = createDir(subDir, "html/dbg_imgs");
              
              // Directory that widgets can use as temporary scratch space
              string tmpDir = createDir(subDir, "html/tmp");
              
              properties* props = new properties();
              map<string, string> pMap;
              props->add("dummy", pMap);
              structure::dbgStream groupStream(NULL, txt()<<"Variant "<<subDirCount, subDir, imgDir, tmpDir);
              #ifdef VERBOSE
              cout << indent << "<<<<<<<<<<<<<<<<<<<<<<"<<endl;
              #endif
              vector<bool> groupActiveParser = merge(groupParsers, groupNextTag, variantStackDepth+1, groupStream
                                                     #ifdef VERBOSE
                                                     , indent+"    "
                                                     #endif
                                                     );
              #ifdef VERBOSE
              cout << indent << ">>>>>>>>>>>>>>>>>>>>>>"<<endl;
              #endif
              out << "[Variant subDir=\""<<subDir<<"\"]\n";
              
              subDirCount++;
            // >>>
            
            // We're done with processing the tag that was just entered
            
            // Update activeParsers[] based on the reading activity that occured inside the merge() call.
            // If entry/exit tags were balanced we'd be guaranteed that all of the grou's parsers would
            // be active but since the log generating application may terminate prematurely, we may hit
            // the end of its log inside this merge call.
            // groupIdx - the index of a given parser within this group (groupParsers[])
            // globalIdx - the index of the same parser within the entire parsers[] vector
            int groupIdx=0;
            for(list<int>::iterator globalIdx=ts->second.begin(); globalIdx!=ts->second.end(); globalIdx++, groupIdx++) {
              assert(activeParser[*globalIdx]);
              
              // If the current parser was previously active and has now terminated, update state
              if(groupActiveParser[groupIdx]==false) {
                activeParser[*globalIdx]=false;
                numActive--;
              }
            }
                        
            // Reset readyForTag and tag2stream to forget this tag on the parsers within the current group
            // and get ready to read more from them.
            for(list<int>::const_iterator i=ts->second.begin(); i!=ts->second.end(); i++)
              readyForTag[*i] = true;
            tag2stream.erase(ts++);
          
          // Do nothing for exit tags since these parsers wait for the ones that entered tags to complete their
          // processing of these tags
          } else if(ts->first.first == properties::exitTag) {
            ts++;
          }
        } // Iterate over all the groups that entered a tag
      }
    } // If we only entered or exited tags
  } // while(numActive>0)
  
  // We reach this point if all parsers have terminated
  
  // Return the active state of all the parsers
  return activeParser;
}
