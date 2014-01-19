#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include "sight_common.h"

using namespace sight;
using namespace sight::common;
using namespace std;

int main(int argc, char** argv) {
  if(argc!=3) { cerr << "Usage: 11.ExternTraceProcess.windowing.pl windowSize inFName"<<endl; exit(-1); }
  int windowSize = atoi(argv[1]);
  char* inFName    = argv[2];

  FILE* in = fopen(inFName, "r");
  if(in==NULL) { cerr << "ERROR opening file \""<<inFName<<"\" for reading!"<<endl; exit(-1); }
  
  // The contexts, observations and anchors in the current window and the iterators to their midpoints
  list<map<string, string> >    windowCtxt;   list<map<string, string> >::iterator    midCtxt;
  list<map<string, attrValue> > windowObs;    list<map<string, attrValue> >::iterator midObs;
  list<map<string, string> >    windowAnchor; list<map<string, string> >::iterator    midAnchor;
  
  // Flag that indicates whether the midpoint iterators have been initialized
  bool midInitialized = false;
  
  // Maps each observation attribute to its type
  map<string, attrValue::valueType> obsTypes;
  
  char line[100000];
  int lineNum=1;
  while(fgets(line, 100000, in)) {
    escapedStr eLine(line, " ", escapedStr::escaped);
    vector<string> data = eLine.unescapeSplit(" ");
    //cout << "eLine=\""<<eLine.unescape()<<"\""<<endl;
    
    map<string, string>    ctxt;   // The context name=>value mappings
    map<string, attrValue> obs;    // The observation name=>value mappings
    map<string, string>    anchor; // The anchor name=>anchorID mappings
    
    for(vector<string>::iterator d=data.begin(); d!=data.end(); d++) {
      escapedStr eD(*d, ":", escapedStr::escaped);
      vector<string> data = eD.unescapeSplit(":");
      
      //cout << "eD = "<<eD.unescape()<<" data (#"<<data.size()<<")=";
      //for(vector<string>::iterator d=data.begin(); d!=data.end(); d++) cout << *d << " ";
      //cout << endl;
      
      // If this is a context field
      if(data[0] == "ctxt") 
        ctxt[data[1]] = data[2];
      // If this is an observation field
      else if(data[0] == "obs") {
        obs[data[1]] = attrValue(data[2], attrValue::unknownT);
        if(lineNum==1) obsTypes[data[1]] = obs[data[1]].getType();
        else if(obs.find(data[1]) == obs.end()) { cerr << "ERROR: Inconsistent observation attributes! Attribute \""<<data[1]<<"\" was provided on line "<<lineNum<<" for the first time!"<<endl; exit(-1); }
        else if(obsTypes[data[1]] != obs[data[1]].getType()) { cerr << "ERROR: Inconsistent types of observation attribute \""<<data[1]<<"\"! On line "<<lineNum<<" type is "<<attrValue::type2str(obs[data[1]].getType())<<" but on prior lines it is "<<attrValue::type2str(obsTypes[data[1]])<<"!"<<endl; exit(-1); }
      // If this is an anchor field
      } else if(data[0] == "anchor")
        anchor[data[1]] = data[2];
    }
    
    if(obs.size() != obsTypes.size()) { cerr << "ERROR: observation attributes on line "<<lineNum<<" inconsistent with those on earlier lines!"<<endl; exit(-1); }
    
    // Add this line to the window
    windowCtxt.push_back(ctxt);
    windowObs.push_back(obs);
    windowAnchor.push_back(anchor);
    
    lineNum++;

    //print "line=\"$line\", ctxt=",obj2Str(\%ctxt),", obs=",obj2Str(\%obs),", anchor=",obj2Str(\%anchor),"\n";
    // If this window is full
    if(windowObs.size() == windowSize) {
      //print "window=",obj2Str(\@window),"\n";
      
      // If the midpoint iterators have not yet been initialized, set them to the window's midpoint
      if(!midInitialized) {
        midCtxt   = windowCtxt.begin();   for(int i=0; i<windowSize/2; i++) { midCtxt++; }
        midObs    = windowObs.begin();    for(int i=0; i<windowSize/2; i++) { midObs++; }
        midAnchor = windowAnchor.begin(); for(int i=0; i<windowSize/2; i++) { midAnchor++; }
        midInitialized = true;
      }

      // Iterate over the window, collecting the sums of all the observation attributes
      map<string, attrValue> sums;

      for(list<map<string, attrValue> >::iterator i=windowObs.begin(); i!=windowObs.end(); i++) {
        if(i==windowObs.begin()) {
          for(map<string, attrValue>::iterator o=i->begin(); o!=i->end(); o++)
            if(obsTypes[o->first] == attrValue::intT || obsTypes[o->first] == attrValue::floatT)
              sums[o->first] = o->second;
        } else {
          for(map<string, attrValue>::iterator o=i->begin(); o!=i->end(); o++) {
            if(obsTypes[o->first] == attrValue::intT || obsTypes[o->first] == attrValue::floatT) {
              switch(obsTypes[o->first]) {
                  case attrValue::intT:   sums[o->first] = sums[o->first].getInt()   + o->second.getInt();   break;
                  case attrValue::floatT: sums[o->first] = sums[o->first].getFloat() + o->second.getFloat(); break;
                default: exit(-1);
              }
            }
          }
        }
        //print "$i: sums=",obj2Str(\%sums),"\n";
      }
      
      
      bool first = true;
      // Emit the average of the window to the output file
      for(map<string, string>::iterator c=midCtxt->begin(); c!=midCtxt->end(); c++) {
        if(!first) cout << " "; else first = false;
        escapedStr cName(c->first,  ":", escapedStr::unescaped);
        escapedStr cVal (c->second, ":", escapedStr::unescaped);
        cout << "ctxt:"<<cName.escape()<<":"<<cVal.escape();
      }

      for(map<string, attrValue>::iterator o=midObs->begin(); o!=midObs->end(); o++) {
        if(!first) cout << " "; else first = false;
        escapedStr oName(o->first,  ":", escapedStr::unescaped);

        // If the current attribute was numeric, emit the windowed value
        if(sums.find(o->first) != sums.end()) {
          cout << "obs:"<<oName.escape()<<":";
          // Divide by window size to produce the average
          string ser;
          switch(obsTypes[o->first]) {
            case attrValue::intT:   ser = attrValue(sums[o->first].getInt() / windowSize).serialize();   break;
            case attrValue::floatT: ser = attrValue(sums[o->first].getFloat() / windowSize).serialize(); break;
            default: exit(-1);
          }
          escapedStr oVal(ser,  ":", escapedStr::unescaped);
          cout << oVal.escape();
        // Otherwise, emit the value in this observation
        } else
          cout << "obs:"<<oName.escape()<<":"<<o->second.serialize();
      }

      for(map<string, string>::iterator a=midAnchor->begin(); a!=midAnchor->end(); a++) {
        if(!first) cout << " "; else first = false;
        escapedStr aName(a->first,  ":", escapedStr::unescaped);
        escapedStr aVal (a->second, ":", escapedStr::unescaped);
        cout << "anchor:"<<aName.escape()<<":"<<aVal.escape();
      }
      cout << endl;

      // Remove the oldest entry from the window
      windowObs.pop_front();
      
      // Advance the midpoint iterators
      midCtxt++;
      midObs++;
      midAnchor++;
    }
  }
  
  fclose(in);
}