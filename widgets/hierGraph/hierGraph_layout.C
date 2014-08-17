// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
#define HIERGRAPH_LAYOUT_C
#include "../../sight_layout.h"
#include "../trace/trace_layout.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <iomanip>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "hierGraph_layout.h"

using namespace std;
using namespace sight::common;

namespace sight {
namespace layout {

// Escapes the whitespace in a node's name
string HG_escapeNodeWhitespace(string s)
{
  string out;
  for(unsigned int i=0; i<s.length(); i++) {
    if(s[i]==' ' || s[i]=='\t' || s[i]=='\n' || s[i]=='\r') {
    } else
    	out += s[i];
  }
  return out;
}

std::string HG_data2str(const map<string, string>& data) {
  ostringstream s;
  for(map<string, string>::const_iterator d=data.begin(); d!=data.end(); d++) { cout << d->first << "=>"<<d->second<<" "; }
  return s.str();
}

// Record the layout handlers in this file
void* hierGraphAppEnterHandler(properties::iterator props) { return new hierGraphApp(props); }
void  hierGraphAppExitHandler(void* obj) { hierGraphApp* ma = static_cast<hierGraphApp*>(obj); delete ma; }
  
hierGraphLayoutHandlerInstantiator::hierGraphLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["hierGraphApp"]          = &hierGraphAppEnterHandler;
  (*layoutExitHandlers )["hierGraphApp"]          = &hierGraphAppExitHandler;
  (*layoutEnterHandlers)["hierGraphAppBody"]      = &defaultEntryHandler;
  (*layoutExitHandlers )["hierGraphAppBody"]      = &defaultExitHandler;
  (*layoutEnterHandlers)["hierGraphAppStructure"] = &defaultEntryHandler;
  (*layoutExitHandlers )["hierGraphAppStructure"] = &defaultExitHandler;
  (*layoutEnterHandlers)["hierGraphTS"]            = &hierGraphTraceStream::enterTraceStream;
  (*layoutExitHandlers )["hierGraphTS"]            = &defaultExitHandler;
  (*layoutEnterHandlers)["hierGraph"]              = &hierGraphApp::enterHierGraph;
  (*layoutExitHandlers )["hierGraph"]              = &hierGraphApp::exitHierGraph;
  (*layoutEnterHandlers)["hierGraphMarker"]        = &defaultEntryHandler;
  (*layoutExitHandlers )["hierGraphMarker"]        = &defaultExitHandler;
  (*layoutEnterHandlers)["hierGraphCtrl"]          = &defaultEntryHandler;
  (*layoutExitHandlers )["hierGraphCtrl"]          = &defaultExitHandler;
  (*layoutEnterHandlers)["hierGraphEdge"]          = &hierGraphApp::addEdge;
  (*layoutExitHandlers )["hierGraphEdge"]          = &defaultExitHandler;
  (*layoutEnterHandlers)["compHierGraphTS"]        = &compHierGraphTraceStream::enterTraceStream;
  (*layoutExitHandlers )["compHierGraphTS"]        = &defaultExitHandler;
  (*layoutEnterHandlers)["processedHierGraphTS"]   = &processedHierGraphTraceStream::enterTraceStream;
  (*layoutExitHandlers )["processedHierGraphTS"]   = &defaultExitHandler;
}
hierGraphLayoutHandlerInstantiator hierGraphLayoutHandlerInstance;

// Points to the currently active instance of hierGraphApp. There can be only one.
hierGraphApp* hierGraphApp::activeMA=NULL;

// The path the directory where output files of the graph widget are stored
// Relative to current path
string hierGraphApp::outDir="";
// Relative to root of HTML document
string hierGraphApp::htmlOutDir="";

// Stack of the hierGraphs that are currently in scope within this hierGraphApp
list<sight::layout::hierGraphInfo> hierGraphApp::hgStack;

hierGraphApp::hierGraphApp(properties::iterator props) : block(properties::next(props)) {
  // Register this hierGraphApp instance (there can be only one)
  assert(activeMA == NULL);
  activeMA = this;
  
  dbg.enterBlock(this, false, true);
  initEnvironment();  
    
  appName = properties::get(props, "appName");
  appID = properties::getInt(props, "appID");
  
  dbg.ownerAccessing();

  // hoa edit
  //dbg << "<div> <canvas id=\"flGra\" data-processing-sources=\"widgets/hierGraph/flGra.pde\" width=\"1200\" height=\"900\"> </canvas> </div>\n";
  //dbg << "<a href=\"widgets/hierGraph/index.html\">Flow Graph</a>\n";
  dbg << "<iframe id=\"flGrFrame\" src=\"widgets/hierGraph/index.html\" width=\"1300\" height=\"1200\"></iframe>\n";

  dbg << "<div id=\"hierGraph_container_"<<appID<<"\"></div>\n";

  dbg.userAccessing();
  
  ostringstream origDotFName;
  origDotFName   << outDir << "/orig."   << appID << ".dot";
  dotFile.open(origDotFName.str().c_str());
  dotFile << "digraph G {"<<endl;
  dotFile << "\tcompound=true;"<<endl;

  // hoa edit
  // node file
  ostringstream tFName;
  //tFName   << outDir << "/node."   << appID << ".txt";
  tFName   << outDir << "/node.txt";
  tFile.open(tFName.str().c_str());
  // input_output file
  ostringstream inouFName;
  //inouFName << outDir << "/inout." << appID << ".txt";
  inouFName << outDir << "/inout.txt";
  inouFile.open(inouFName.str().c_str());
  // data for statistic visualization
  ostringstream datFName;
  //datFName << outDir << "/inout." << appID << ".txt";
  datFName << outDir << "/dat.txt";
  datFile.open(datFName.str().c_str());
  // data for input and output variable information of hierGraphs
  ostringstream ioInfoFName;
  ioInfoFName << outDir << "/ioInfo.txt";
  ioInfoFile.open(ioInfoFName.str().c_str());

}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void hierGraphApp::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  // Create the directory that holds the hierGraph-specific scripts
  pair<string, string> paths = dbg.createWidgetDir("hierGraph");
  outDir = paths.first;
  htmlOutDir = paths.second;
  //cout << "outDir="<<outDir<<" htmlOutDir="<<htmlOutDir<<endl;
  
  dbg.includeFile("canviz-0.1");
  
  //<!--[if IE]><script type="text/javascript" src="excanvas/excanvas.js"></script><![endif]-->
  //dbg.includeWidgetScript("canviz-0.1/prototype/excanvas/excanvas.js", "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/prototype/prototype.js", "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/path/path.js",           "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/canviz.css",             "text/css");
  dbg.includeWidgetScript("canviz-0.1/canviz.js",              "text/javascript");
  dbg.includeWidgetScript("canviz-0.1/x11colors.js",           "text/javascript");
  //dbg.includeWidgetScript("canviz-0.1/graphs/graphlist.js",  "text/javascript");
  //dbg.includeWidgetScript("canviz-0.1/graphs/layoutlist.js", "text/javascript");
  
  dbg.includeFile("hierGraph/hierGraph.js"); dbg.includeWidgetScript("hierGraph/hierGraph.js", "text/javascript"); 

  // hoa edit
  dbg.includeFile("hierGraph/processing.js"); dbg.includeWidgetScript("hierGraph/processing.js", "text/javascript");
  dbg.includeFile("hierGraph/flgr.js"); dbg.includeWidgetScript("hierGraph/flgr.js", "text/javascript");
  dbg.includeFile("hierGraph/flGra.pde");
  dbg.includeFile("hierGraph/index.html");
}

hierGraphApp::~hierGraphApp() {
  // Unregister this hierGraphApp instance (there can be only one)
  assert(activeMA);
  activeMA = NULL;
  
  dotFile << "}"<<endl;
  dotFile.close();
  
  // hoa edit
  tFile.close();
  inouFile.close();
  datFile.close();
  ioInfoFile.close();

  dbg.exitBlock();

  // Lay out the dot graph   
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << appID << ".dot";
  ostringstream placedDotFName; placedDotFName << outDir << "/placed." << appID << ".dot";

  // Create the explicit DOT file that details the graph's layout
  ostringstream cmd; cmd << ROOT_PATH << "/widgets/graphviz/bin/dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  //cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());
  
  dbg.widgetScriptCommand(txt() << 
     "  var canviz_"<<appID<<";\n" <<
     "  canviz_"<<appID<<" = new Canviz('hierGraph_container_"<<appID<<"');\n" <<
     "  canviz_"<<appID<<".setScale(1);\n" <<
     "  canviz_"<<appID<<".load('"<<htmlOutDir<<"/placed." << appID << ".dot');\n"); 
  
  // Delete all the traceStreams associated with the given trace, which emits their output
  for(map<int, traceStream*>::iterator m=hierGraphTraces.begin(); m!=hierGraphTraces.end(); m++)
    delete m->second;
}

string portName(common::hierGraph::ioT type, int index) 
{ return txt()<<(type==common::hierGraph::input?"input":"output")<<"_"<<index; }

// Given the name of a trace attribute, the string representation of its polynomial fit and a line width 
// emits to dotFile HTML where line breaks are inserted at approximately every lineWidth characters.
void HG_printPolyFitStr(ostream& dotFile, std::string traceName, std::string polyFit, unsigned int lineWidth) {
//void HG_printPolyFitStr(ostream& tFile, std::string traceName, std::string polyFit, unsigned int lineWidth) {

  // hoa edit
  //tFile << "HG_printPolyFitStr"<< endl;
  //tFile << "traceName : " << traceName << endl;

  unsigned int i=0;
  dotFile << "\t\t<TR><TD><TABLE BORDER=\"0\"  CELLBORDER=\"0\">"<<endl;
  dotFile << "\t\t\t<TR><TD>:"<<traceName<<"</TD>";
  
  while(i<polyFit.length()) {
    // Look for the next line-break
    unsigned int nextLB = polyFit.find_first_of("\n", i);
    // If the next line is shorter than lineWidth, add it to labelMulLineStr and move on to the next line
    if(nextLB-i < lineWidth) {
      if(i!=0) dotFile << "\t\t<TR><TD></TD>";
      dotFile << "<TD>:"<<polyFit.substr(i, nextLB-i+1)<<"</TD></TR>"<<endl;
      i = nextLB+1;
    // If the next line is longer than lineWidth, add just lineWidth characters to labelMulLineStr
    } else {
      // If it is not much longer than lineWidth, don't break it up
      if(i>=polyFit.length() - lineWidth) break;
      
      if(i!=0) dotFile << "\t\t<TR><TD></TD>";
      dotFile << "<TD>:"<<polyFit.substr(i, lineWidth)<<"</TD></TR>"<<endl;
      
      i += lineWidth;
    }
  }
  
  if(i<polyFit.length()) {
    if(i!=0) dotFile << "\t\t<TR><TD></TD>";
    dotFile << "<TD>:"<<polyFit.substr(i, polyFit.length()-i)<<"</TD></TR>"<<endl;
  }
  dotFile << "</TABLE></TD></TR>"<<endl;
}

// Emits to the dot file the buttons used to select the combination of input property and trace attribute
// that should be shown in the data panel of a given hierGraph node.
// numInputs/numOutputs - the number of inputs/outputs of this hierGraph node
// ID - the unique ID of this hierGraph node
// prefix - We measure both the observations of measurements during the execution of hierGraphs and the 
//    properties of hierGraph outputs. Both are included in the trace of the hierGraph but the names of measurements
//    are prefixed with "measure:" and the names of outputs are prefixed with "output:#:", where the # is the
//    index of the output. The prefix argument identifies the types of attributs we'll be making buttons for and
//    it should be either "measure" or "output".
// bgColor - The background color of the buttons
int HG_maxButtonID=0; // The maximum ID that has ever been assigned to a button
//void hierGraphApp::showButtons(int numInputs, int numOutputs, int ID, std::string prefix, std::string bgColor) {
//  // Buttons for showing the observation trace plots
//  //cout << "showButtons("<<numInputs<<", "<<numOutputs<<", #hierGraphs["<<ID<<"]->traceAttrNames="<<hierGraphs[ID]->traceAttrNames.size()<<endl;
//  for(set<string>::iterator t=hierGraphs[ID]->traceAttrNames.begin(); t!=hierGraphs[ID]->traceAttrNames.end(); t++) {
//    // If the current trace attribute does not have the selected prefix, skip it
//    if(t->find(prefix) == string::npos) continue;
//    
//    dotFile << "\t\t\t<TR>";
//    //for(int i=0; i<numInputs; i++) {
//    for(map<string, list<string> >::iterator ctxtGrouping=hierGraphs[ID]->ctxtNames.begin(); 
//        ctxtGrouping!=hierGraphs[ID]->ctxtNames.end(); ctxtGrouping++) {
//      //if(ctxtNames[ID].find(i)!=ctxtNames[ID].end()) {
//        //for(list<string>::iterator c=ctxtNames[ID][i].begin(); c!=ctxtNames[ID][i].end(); c++) {
//        for(list<string>::iterator c=ctxtGrouping->second.begin(); c!=ctxtGrouping->second.end(); c++) {
//          int buttonID = HG_maxButtonID; HG_maxButtonID++;
//          dotFile << "<TD BGCOLOR=\"#"<<bgColor<<"\"><FONT POINT-SIZE=\"14\">"<<buttonID<<":"<<*t<<"</FONT></TD>";
//
//          // Register the command to be executed when this button is clicked
//          ostringstream cmd; 
//          cmd << "registerHierGraphButtonCmd("<<buttonID<<", \""<<
//                               hierGraphTraces[ID]->getDisplayJSCmd(traceStream::attrNames(/*txt()<<i<<":"<<*c*/ctxtGrouping->first+":"+*c), traceStream::attrNames(*t))<<
//                             "\");"<<endl;
////cout << "ts="<<hierGraphTraces[ID]<<" cmd="<<cmd.str()<<endl;
//          dbg.widgetScriptCommand(cmd.str());
//        } // ctxt attrs
//      /*} else
//        dotFile << "<TD></TD>";*/
//    } // inputs
//    dotFile << "\t\t\t</TR>"<<endl;
//  } // trace attrs
//}

// kyushick edit
// Enter a new hierGraph within the current hierGraphApp
// numInputs/numOutputs - the number of inputs/outputs of this hierGraph node
// ID - the unique ID of this hierGraph node
//void hierGraphApp::enterHierGraph(string hierGraphName, int hierGraphID, int numInputs, int numOutputs, int count) {
void hierGraphApp::enterHierGraph(string hierGraphName, int hierGraphID, int horID, int verID, int numInputs, int numOutputs, int count) {

  // hoa edit
  //tFile << hierGraphID <<":"<< hierGraphName << ":" << numInputs << ":"<<numOutputs <<endl;
  //tFile << hierGraphName << ":" << numInputs << ":"<<numOutputs <<endl;
  //ioInfoFile << hierGraphID <<":"<< numInputs << ":"<<numOutputs<< endl;

  //cout << "hierGraphApp::enterHierGraph("<<hierGraphName<<") numInputs="<<numInputs<<", #hierGraphs["<<hierGraphID<<"]->ctxtNames="<<hierGraphs[hierGraphID]->ctxtNames.size()<<endl;
  
  // Inform the traceStream associated with this hierGraph that it is finished. We need this stream to wrap up
  // all of its processing and analysis now, rather than before it is deallocated.
  hierGraphTraces[hierGraphID]->obsFinished();

  // Get the ID of the hierGraph that contains this one, if any.
  int containerHierGraphID = -1;
  if(hgStack.size()>0) containerHierGraphID = hgStack.back().hierGraphID;
  
  // hoa edit
  tFile << hierGraphID <<":"<< hierGraphName <<":"<< numInputs <<":"<<numOutputs<<":"<<containerHierGraphID<<endl;
  // kyushick edit
  // you may want to get horID and verID here.
  // tFile << hierGraphID << ":"<< horID<< ":"<< verID <<":"<< hierGraphName <<":"<< numInputs <<":"<<numOutputs<<":"<<containerHierGraphID<<endl;


  // Start a subgraph for the current hierGraph
  dotFile << "subgraph cluster"<<hierGraphID<<" {"<<endl;
  //dotFile << "\tstyle=filled;"<<endl;
  dotFile << "\tcolor=black;"<<endl;
  //dotFile << "\tlabel=\""<<hierGraphName<<"\";"<<endl;
  
  /*for(int i=0; i<numInputs; i++) {
    dotFile << "\t\t"<<portName(node, input, i)<<" [shape=box, label=\"In "<<i<<"\"];\n";//, href=\"javascript:"<<b->first.getLinkJS()<<"\"];\n";
    if(i>0) dotFile << "\t\t"<<portName(node, input, i-1)<<" -> "<<portName(node, input, i)<<" [style=invis];"<<endl;
  }
  
  for(int o=0; o<numOutputs; o++)
    dotFile << "\t\t"<<portName(node, output, o)<<" [shape=box, label=\"Out "<<o<<"\"];\n";//, href=\"javascript:"<<b->first.getLinkJS()<<"\"];\n";
  
  if(numInputs>0) { 
    dotFile << "\t{rank=\"source\"";
    for(int i=0; i<numInputs; i++) dotFile << " "<<portName(node, input, i)<<"";
    dotFile << "}"<<endl;
  }
  
  if(numOutputs>0) { 
    dotFile << "\t{rank=\"sink\"";
    for(int o=0; o<numOutputs; o++) dotFile << " "<<portName(node, output, o)<<"";
    dotFile << "}"<<endl;
  }*/

  // In the construction below we wish to allow users to interact with the trace information displayed in the main ID
  // box by clicking on the names of context attributes. graphviz/Canviz allow us to create a single onclick method for
  // the entire node but not separate ones for each TD. As such, Canviz has been extended to treat this special case
  // (javascript handlers + HTML nodes) specially, where each piece of text in such a node must be formated as 
  // "ID:text" where ID is the ID that uniquely identifies the piece of text and text is the actual text that should 
  // be displayed. ID must not contain ":" but text may. Then, the javascript handlers of the node can assume the existence 
  // of a special ID variable that uniquely identifies the particular piece of text that got clicked. If an ID of a given
  // piece of text is empty, then no link is placed around it. If some piece of text is mis-formatted (if missing the ":", 
  // Canviz provides an alert). 
  
  int databoxWidth = 300;
  int databoxHeight = 200;
  
  //cout << "hierGraph "<<hierGraphName<<", numInputs="<<numInputs<<", #hierGraphs[hierGraphID]->ctxtNames="<<hierGraphs[hierGraphID]->ctxtNames.size()<<endl;
  //dotFile << "\t\""<<portNamePrefix(hierGraphName)<<"\" [shape=none, fill=lightgrey, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  // Input ports and body
  dotFile << "\tnode"<<hierGraphID<<" [shape=none, fill=lightgrey, href=\"#\", onclick=\"return ClickOnHierGraphNode('node"<<hierGraphID<<"', this, ID);\", label=";
  dotFile << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
 
  // Records whether the entry and ports have been emitted
  bool entryEmitted=false; 
  bool exitEmitted=false;
  //cout << "hierGraphName="<<hierGraphName<<", numInputs="<<numInputs<<", #hierGraphs["<<hierGraphID<<"]->ctxtNames="<<hierGraphs[hierGraphID]->ctxtNames.size()<<endl;
  if(numInputs>0) {
    if(hierGraphs[hierGraphID]->ctxtNames.size()>0) {
      dotFile << "\t\t<TR><TD PORT=\"ENTRY\"><TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
      entryEmitted = true;
      dotFile << "\t\t\t<TR>";
      
      int maxCtxt=0; // The maximum number of context names across all the inputs
      for(map<string, list<string> >::iterator c=hierGraphs[hierGraphID]->ctxtNames.begin(); 
          c!=hierGraphs[hierGraphID]->ctxtNames.end(); c++) {
        string hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName;
        decodeCtxtName(c->first, hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName);
        
        dotFile << "<TD PORT=\""<<ctxtGrouping<<"_"<<ctxtSubGrouping<<"\" "<<
        // Horizontal context               "COLSPAN=\""<<c->second.size()<<"\""<<
                    ">"<<
                        "<FONT POINT-SIZE=\"20\">:"<<ctxtGrouping<<" "<<ctxtSubGrouping<<" "<<attrName<<"</FONT>"<<
                   "</TD>";

        maxCtxt = (maxCtxt > c->second.size()? maxCtxt: c->second.size());
      }
      dotFile << "</TR>"<<endl;
      
      // The names of the context attributes for each output
      /* Horizontal context
      dotFile << "\t\t\t<TR>";
      for(map<string, list<string> >::iterator ctxtGrouping=hierGraphs[hierGraphID]->ctxtNames.begin(); 
          ctxtGrouping!=hierGraphs[hierGraphID]->ctxtNames.end(); ctxtGrouping++) {
        for(list<string>::iterator c=ctxtGrouping->second.begin(); c!=ctxtGrouping->second.end(); c++) {
          dotFile << "<TD BGCOLOR=\"#000066\"><FONT COLOR=\"#ffffff\" POINT-SIZE=\"18\">:"<<*c<<"</FONT></TD>";
        }
      }
      dotFile << "\t\t\t</TR>"<<endl;*/
      
      // Vertical context
      /* --- Not showing this anymore since it looks ugly ---
      // Place the first context name of each input, then the 2nd, etc.
      map<string, pair<list<string>::iterator, list<string>::iterator> > i; // Points to the i^th context name for each input
      // Initializes i to contain the starting iterators of each input's context names list
      for(map<string, list<string> >::iterator c=hierGraphs[hierGraphID]->ctxtNames.begin(); c!=hierGraphs[hierGraphID]->ctxtNames.end(); c++)
        i[c->first] = make_pair(c->second.begin(), c->second.end());
      
      for(int cIdx=0; cIdx<maxCtxt; cIdx++) {
        dotFile << "<TR>";
        map<string, pair<list<string>::iterator, list<string>::iterator> > newI;
        for(map<string, pair<list<string>::iterator, list<string>::iterator> >::iterator j=i.begin(); j!=i.end(); j++) {
          // Place boxes for the current context of each input
          dotFile << "<TD>";
          // If we have not yet run out of contexts for the current input
          if(j->second.first != j->second.second) dotFile << ":" << *(j->second.first);
          dotFile << "</TD>";
          
          // Advance the iterator for the current input
          //j->second.first++;
          list<string>::iterator next = j->second.first;
          if(j->second.first != j->second.second) next++;
          newI[j->first] = make_pair(next, j->second.second);
        }
        i = newI;
        
        dotFile << "</TR>"<<endl;
      }*/
      
      // Buttons for showing the observation trace plots
      //showButtons(numInputs, numOutputs, hierGraphID, "measure", "B0CDFF");
      //showButtons(numInputs, numOutputs, hierGraphID, "output",  "F78181");
      
      dotFile << "</TABLE></TD></TR>"<<endl;
    }
  }
  
  {
    // Node Info
    
    // Collect the names of all the context and trace attributes
    list<string> contextAttrs;
    for(map<string, list<string> >::iterator c=hierGraphs[hierGraphID]->ctxtNames.begin(); c!=hierGraphs[hierGraphID]->ctxtNames.end(); c++) {
      for(list<string>::iterator d=c->second.begin(); d!=c->second.end(); d++)
        contextAttrs.push_back(c->first+":"+*d);
    }
    
    list<string> traceAttrs;
    for(set<string>::iterator t=hierGraphs[hierGraphID]->traceAttrNames.begin(); t!=hierGraphs[hierGraphID]->traceAttrNames.end(); t++)
      traceAttrs.push_back(*t);
    
    // Register the command to be executed when this button is clicked
    ostringstream cmd; 

    //hoa edit
    list<string> vizList;
    vizList.push_back("scatter3d");

    // getDisplayJSCmd(contextAttrs, traceAttrs, hostDiv, vizList, inwin, showFresh, showLabels, refreshView)
    cmd << "registerHierGraphButtonCmd("<<HG_maxButtonID<<", \""<<
                             hierGraphTraces[hierGraphID]->getDisplayJSCmd(contextAttrs, traceAttrs, "", vizList,true, true, false, true)<<
                           "\");"<<endl;
    vizList.clear();
    vizList.push_back("ccp");
    cmd << "registerHierGraphButtonCmd("<<(HG_maxButtonID+100)<<", \""<<
                                 hierGraphTraces[hierGraphID]->getDisplayJSCmd(contextAttrs, traceAttrs, "", vizList,true, true, false, true)<<
                               "\");"<<endl;

    vizList.clear();
	vizList.push_back("pcp");
	cmd << "registerHierGraphButtonCmd("<<(HG_maxButtonID+200)<<", \""<<
								 hierGraphTraces[hierGraphID]->getDisplayJSCmd(contextAttrs, traceAttrs, "", vizList,true, true, false, true)<<
							   "\");"<<endl;

    string vizl = "scatter3d:ccp:pcp";
    //datFile << "nodeid="<< hierGraphID << ",buttonID = " << HG_maxButtonID << ",vizList =" << vizl << endl;
    datFile << hierGraphID << "," << HG_maxButtonID << "," << vizl << endl;

    /*
    cmd << "registerHierGraphButtonCmd("<<HG_maxButtonID<<", \""<<
                             hierGraphTraces[hierGraphID]->getDisplayJSCmd(contextAttrs, traceAttrs, "", trace::scatter3d, true, false, true)<<
                           "\");"<<endl;
    */
    dbg.widgetScriptCommand(cmd.str());
    
    dotFile << "\t\t<TR><TD";
    if(!entryEmitted) dotFile << " PORT=\"ENTRY\"";
    //if(numInputs + numOutputs > 0) dotFile << " COLSPAN=\""<<(numInputs>numOutputs? numInputs: numOutputs)<<"\"";
    dotFile << "><FONT POINT-SIZE=\"26\">"<<(HG_maxButtonID++)<<":"<<hierGraphName<<"</FONT></TD></TR>"<<endl;
  }
  
  if(numInputs>0) {
/*    // If we observed values during the execution of this hierGraph group  
    if(hierGraphs[hierGraphID]->traceAttrNames.size()>0) {
      // Polynomial fit of the observations
      vector<string> polynomials = hierGraphs[hierGraphID]->polyFit();
      assert(polynomials.size() == hierGraphs[hierGraphID]->traceAttrNames.size());
      int i=0; 
      for(set<std::string>::iterator t=hierGraphs[hierGraphID]->traceAttrNames.begin(); t!=hierGraphs[hierGraphID]->traceAttrNames.end(); t++, i++) {
        // If we were able to train a model for the current trace attribute, emit it
        if(polynomials[i] != "") HG_printPolyFitStr(dotFile, *t, polynomials[i], 80);
      }
        //dotFile << "\t\t<TR><TD>:"<<hierGraphs[hierGraphID]->traceAttrNames[i]<< ": "<<wrapStr(polynomials[i], 50)<<"</TD></TR>"<<endl;
    }*/
    //cout << "polyFits[hierGraphID]="<<polyFits[hierGraphID]<<", polyFits[hierGraphID]->numFits()="<<polyFits[hierGraphID]->numFits()<<endl;

	// hoa edit
	//ioInfoFile << hierGraphID << ":"<<polyFits[hierGraphID]->numFits()<< endl;

    if(polyFits[hierGraphID] && polyFits[hierGraphID]->numFits()>0)
    {
      dotFile << "\t\t"<<polyFits[hierGraphID]->getFitText()<<""<<endl;

      //ioInfoFile <<"numFits="<<polyFits[hierGraphID]->numFits()<<endl;
      ioInfoFile <<hierGraphID << ";"<<polyFits[hierGraphID]->numFits() << ";"<< polyFits[hierGraphID]->saveFitText()<<endl;
    }
    
    
    //cout << "#hierGraphs[hierGraphID]->traceAttrNames="<<hierGraphs[hierGraphID]->traceAttrNames.size()<<endl;
    
    if(hierGraphs[hierGraphID]->traceAttrNames.size()>0) {
    //for(int i=0; i<hierGraphs[hierGraphID]->traceAttrNames.size(); i++) {
      dotFile << "\t\t<TR><TD PORT=\"EXIT\"><TABLE><TR><TD BGCOLOR=\"#FF00FF\" COLOR=\"#FF00FF\" WIDTH=\""<<databoxWidth<<"\" HEIGHT=\""<<databoxHeight<<"\"></TD></TR></TABLE></TD></TR>"<<endl;
      exitEmitted = true;
    }
  }
  else
  {
	  // hoa edit
	  ioInfoFile << hierGraphID << ";0"<< endl;
  }

  if(!exitEmitted)
    dotFile << "\t\t<TR><TD PORT=\"EXIT\"></TD></TR>"<<endl;
  
  dotFile << "</TABLE>>";
  
  dotFile << "];" << endl;

  // Output ports
  dotFile << "\tnode"<<hierGraphID<<"_Out [shape=none, fill=lightgrey, href=\"#\", onclick=\"return ClickOnHierGraphNode('node"<<hierGraphID<<"', this, ID);\", label=";
  if(numOutputs==0)
    dotFile << "\"\"";
  else {
    dotFile << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
    //dotFile << "\t<TR><TD PORT=\"ENTRY\" HEIGHT=\"0\">:</TD></TR>"<<endl;
    dotFile << "\t<TR>"<<endl;
    for(int o=0; o<numOutputs; o++) 
      dotFile << "\t\t<TD WIDTH=\""<<databoxWidth<<"\" PORT=\""<<portName(output, o)<<"\"><FONT POINT-SIZE=\"14\">:Out "<<o<<"</FONT></TD>"<<endl;
    dotFile << "\t</TR>"<<endl;
    //dotFile << "\t<TR><TD PORT=\"EXIT\" HEIGHT=\"0\">:</TD></TR>"<<endl;
    dotFile << "</TABLE>>";
  }
  dotFile << "];" <<endl;
  
  
  // Add a high-weight invisible edge between the input and the output nodes to vertically align them
  dotFile << "\tnode"<<hierGraphID<<":EXIT:s "
             " -> "<<
               "node"<<hierGraphID<<"_Out "<<
             "[weight=100, style=invis];\n";
  
  // If this hierGraph is contained inside another, add invisible edges between the input portion of container hierGraph and 
  // this one and the output portion of this hierGraph and the container to vertically align them
  if(containerHierGraphID>=0) {
    dotFile << "\tnode"<<containerHierGraphID<<":EXIT:s -> node"<<hierGraphID<<":ENTRY:n              [weight=150, style=invis];"<<endl;
    //dotFile << "\tnode"<<hierGraphID<<"_Out:EXIT:s      -> node"<<containerHierGraphID<<"_Out:ENTRY:n [weight=300];"<<endl;
    dotFile << "\tnode"<<hierGraphID<<"_Out      -> node"<<containerHierGraphID<<"_Out [weight=150, style=invis];"<<endl;
    //dotFile << "\tnode"<<containerHierGraphID<<" -> node"<<hierGraphID<<"         [ltail=cluster"<<containerHierGraphID<<", lhead=cluster"<<hierGraphID<<", weight=5];"<<endl;
    //dotFile << "\tnode"<<hierGraphID<<"_Out -> node"<<containerHierGraphID<<"_Out [ltail=cluster"<<hierGraphID<<", lhead=cluster"<<containerHierGraphID<<", weight=5];"<<endl;

    // hoa edit
    //tFile<<"ContainerHierGraphID="<<containerHierGraphID<<endl;
  }
  
  dotFile << "{rank=source;node"<<hierGraphID<<";}"<<endl;
  dotFile << "{rank=sink;node"<<hierGraphID<<"_Out;}"<<endl;
  
  // kyushick edit
  // Add a hierGraphInfo object that records this hierGraph to the hierGraphApp's stack
//  hgStack.push_back(sight::layout::hierGraphInfo(hierGraphName, hierGraphID, numInputs, numOutputs, count));
  hgStack.push_back(sight::layout::hierGraphInfo(hierGraphName, hierGraphID, horID, verID, numInputs, numOutputs, count));

  //cout << "hierGraphApp::enterHierGraph() done\n";
}

// Static version of enterHierGraph() that pulls the from/to anchor IDs from the properties iterator and calls 
// enterHierGraph() in the currently active hierGraphApp
void* hierGraphApp::enterHierGraph(properties::iterator props) {
  // kyushick edit
  int hierGraphID = properties::getInt(props, "hierGraphID");
  int horID       = properties::getInt(props, "horID");
  int verID       = properties::getInt(props, "verID");
  int count       = properties::getInt(props, "count");

  
  // Get this hierGraph's hierGraphTraceStream
  assert(hierGraphApp::getInstance()->hierGraphTraces.find(hierGraphID) != hierGraphApp::getInstance()->hierGraphTraces.end());
  hierGraphTraceStream* hgts = dynamic_cast<hierGraphTraceStream*>(hierGraphApp::getInstance()->hierGraphTraces[hierGraphID]);
  assert(hgts);
  
  assert(hierGraphApp::activeMA);

  // Kyushick edit
  hierGraphApp::activeMA->enterHierGraph(hgts->name, 
                                         hierGraphID, 
                                         horID,
                                         verID,
                                         hgts->numInputs, 
                                         hgts->numOutputs, 
                                         count); 
  return NULL;
}

// Exit a hierGraph within the current hierGraphApp
void hierGraphApp::exitHierGraph() {
  // Grab the information about the hierGraph we're exiting from this hierGraphApp's hgStack and pop it off
  assert(hgStack.size()>0);
  sight::layout::hierGraphInfo m = hgStack.back();
  hgStack.pop_back();
  
  //cout << "hierGraphApp::exitHierGraph("<<m.hierGraphName<<")"<<endl;
  // Close the current hierGraph's sub-graph
  dotFile << "}" << endl;
}

// Static version of exitHierGraph() that calls exitHierGraph() in the currently active hierGraphApp
void hierGraphApp::exitHierGraph(void* obj) {
  assert(hierGraphApp::activeMA);
  hierGraphApp::activeMA->exitHierGraph();
}

// Register the given hierGraph object (keeps data on the raw observations) and HG_polyFitObserver object 
void hierGraphApp::registerHierGraph(int hierGraphID, sight::layout::hierGraph* m, HG_polyFitObserver* pf) {
  assert(hierGraphApp::activeMA);
  hierGraphApp::activeMA->hierGraphs[hierGraphID] = m;
  hierGraphApp::activeMA->polyFits[hierGraphID] = pf;
}

// Add a directed edge from one port to another
void hierGraphApp::addEdge(int fromCID, common::hierGraph::ioT fromT, int fromP, 
                         int toCID,   common::hierGraph::ioT toT,   int toP,
                         double prob) {

  // hoa edit
  /*
  tFile << "adEdge" << endl;
  tFile << "fromCID="<<fromCID<<"_Out:"<<portName(fromT, fromP)<<":s"<<
          " -> "<<
            "toCID="<<toCID  <<":"<<portName(toT,   toP  )<<":n "<<
          "and penwidth="<<(1+prob*3)<<endl;
  */
  inouFile << fromCID <<"_"<< portName(fromT, fromP)<<":"<<toCID<<"_"<<portName(toT, toP)<< endl;

  dotFile << "\tnode"<<fromCID<<"_Out:"<<portName(fromT, fromP)<<":s"<<
             " -> "<<
               "node"<<toCID  <<":"<<portName(toT,   toP  )<<":n "<<
             "[penwidth="<<(1+prob*3)<<"];\n";
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* hierGraphApp::addEdge(properties::iterator props) {
  assert(activeMA);
  activeMA->addEdge(properties::getInt(props, "fromCID"), (ioT)properties::getInt(props, "fromT"), properties::getInt(props, "fromP"),
                    properties::getInt(props, "toCID"),   (ioT)properties::getInt(props, "toT"),   properties::getInt(props, "toP"),
                    properties::getFloat(props, "prob")); 
  return NULL;
}


/******************
 ***** hierGraph *****
 ******************/

hierGraph::hierGraph(int hierGraphID) : hierGraphID(hierGraphID)
{
  /*polyfitCtxt=NULL;
  polyfitObs=NULL;*/
  numObs=0;
  /*numAllocObs=0;
  numAllocTraceAttrs=0;
  numNumericCtxt=-1;*/
}

hierGraph::~hierGraph() {
  /*if(numObs>0) {
    gsl_matrix_free(polyfitCtxt);
    gsl_matrix_free(polyfitObs);
  }*/
}

//// Iterates over all combinations of keys in numericCtxt upto maxDegree in size and computes the products of
//// their values. Adds each such product to the given row of polyfitCtxt at different columns.
//void addPolyTerms(const map<string, double>& numericCtxt, int termCnt, int maxDegree, 
//                  int row, int& col, double product, gsl_matrix *polyfitCtxt) {
//  // If product contains maxDegree terms, add it to the given column of polyfitObs and increment the column counter
//  if(termCnt==maxDegree) {
//    gsl_matrix_set(polyfitCtxt, row, col, product);
//    col++;
//  } else {
//    // Iterate through all the possible choices of the next factor in the current polynomial term
//    for(map<string, double>::const_iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++) {
//      addPolyTerms(numericCtxt, termCnt+1, maxDegree, row, col, product * c->second, polyfitCtxt);
//    }
//  }
//}
//
//// Iterates over all combinations of keys in numericCtxt upto maxDegree in size. Given ctxtNames, which maintains the list
//// of names for each attribute and selTerms, which identifies the attribute combinations that should be selected, matches
//// the selections to the names and inserts the matched pairs to selCtxt. 
//// termCnt - counts the number of terms that have been added to the current polynomial product
//// maxDegree - the maximum such terms that can go into a single product
//// idx - The current count of the terms from the overall space of terms. May not be 0 at the root recursive calls since
////       we may iterate over multiple choices of maxDegree
//// subName - the name string that has been computed so far for the current prefix of terms in the polynomial product
//void selTerms2Names(const list<string>& ctxtNames, const set<int>& selTerms, 
//                    list<pair<int, string> >& selCtxt, 
//                    int termCnt, int maxDegree, int& idx, map<string, int>& subNames) {
//  // If product contains maxDegree terms, add it to the given column of polyfitObs and increment the column counter
//  if(termCnt==maxDegree) {
//    // If the current product index exists in selTerms
//    if(selTerms.find(idx) != selTerms.end()) {
//      // Match it to its name and add it to selCtx.
//      //selCtxt.push_back(make_pair(idx, subName));
//      ostringstream name;
//      for(map<string, int>::iterator n=subNames.begin(); n!=subNames.end(); n++) {
//        assert(n->second>0);
//        if(n!=subNames.begin()) name << "*";
//        name << n->first;
//        if(n->second>1) name << "^"<<n->second;
//      }
//      selCtxt.push_back(make_pair(idx, name.str()));
//    }
//    idx++;
//  } else {
//    // Iterate through all the possible choices of the next factor in the current polynomial term
//    for(list<string>::const_iterator c=ctxtNames.begin(); c!=ctxtNames.end(); c++) {
//      if(subNames.find(*c) == subNames.end()) subNames[*c] = 1;
//      else                                    subNames[*c]++;
//      
//      selTerms2Names(ctxtNames, selTerms, selCtxt, termCnt+1, maxDegree, idx, subNames/*subName+(termCnt>0?"*":"")+*c*/);
//      
//      subNames[*c]--;
//      if(subNames[*c]==0) subNames.erase(*c);
//    }
//  }
//}
//
//
//// Do a multi-variate polynomial fit of the data observed for the given hierGraphID and return for each trace attribute 
//// a string that describes the function that best fits its values
//std::vector<std::string> hierGraph::polyFit()
//{
//  vector<string> polynomials;
//  
//  // Do nothing if we had no observations with numeric context attributes
//  if(numObs==0) return polynomials;
//    
//  // Fit a polynomial to the observed data
//  int maxDegree = 2;
//  long numTerms = polyfitCtxt->size2;
//  // Covariance matrix
//  gsl_matrix *polyfitCov = gsl_matrix_alloc(numTerms, numTerms);
//
//  //for(int i=0; i<polyfitObs.size(); i++) {
//  int i=0; 
//  for(set<std::string>::iterator t=traceAttrNames.begin(); t!=traceAttrNames.end(); t++, i++) {
//    //cout << i << ":  attr "<<*t<<endl;
//    
//    gsl_vector* polyfitCoeff = gsl_vector_alloc(numTerms);
//    gsl_multifit_linear_workspace *polyfitWS = gsl_multifit_linear_alloc(numObs, numTerms);
//    double chisq;
//    gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitCtxt, 0, 0, numObs, numTerms);
//    //gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitObs[i], 0, 0, numObs, numTerms);
//    //gsl_vector_view obsView = gsl_vector_subvector(polyfitObs[i], 0, numObs);
//    //gsl_vector_view obsView = gsl_matrix_subcolumn(polyfitObs[i], i, 0, numObs);
//    gsl_vector_view obsView1 = gsl_matrix_column(polyfitObs, i);
//    gsl_vector_view obsView = gsl_vector_subvector(&(obsView1.vector), 0, numObs);
//    
//    /*for(int j=0; j<numObs; j++) {
//      for(int k=0; k<numTerms; k++)
//        cout << " "<<gsl_matrix_get(&(ctxtView.matrix),j,k);
//      cout << " => "<<gsl_vector_get(&(obsView.vector), j)<<endl;
//    }*/
//    
//    
//    // If we have enough observations to fit a model
//    if(numTerms <= numObs) {
//      // Use a linear solver to fit the coefficients of the polynomial
//      // ******
//      gsl_multifit_linear(&(ctxtView.matrix), &(obsView.vector), polyfitCoeff, polyfitCov, &chisq, polyfitWS);
//      // ******
//      
//      /*cout << "polyfitCoeff=";
//      for(int t=0; t<numTerms; t++) cout << gsl_vector_get(polyfitCoeff, t)<<" ";
//      cout << endl;*/
//      
//      // Identify the pair of coefficients in the sorted order with the largest relative difference between them
//      map<double, int> sortedCoeff;
//      map<int, double> sortedCoeffInv;
//      for(int t=0; t<numTerms; t++) {
//        sortedCoeff[fabs(gsl_vector_get(polyfitCoeff, t))] = t;
//        sortedCoeffInv[t] = fabs(gsl_vector_get(polyfitCoeff, t));
//      }
//      map<double, int>::reverse_iterator splitIter = sortedCoeff.rend();
//      double largestCoeff = sortedCoeff.rbegin()->first;
//      
//      // Iterate sortedCoeff from the largest to smallest coefficient, looking for the largest gap between adjacent coefficients
//      double maxDiff = 0;
//      map<double, int>::reverse_iterator c=sortedCoeff.rbegin();
//      map<double, int>::reverse_iterator next = c; next++;
//      while(next!=sortedCoeff.rend() && next->first!=0) {
//        //cout << c->first<<" / "<<next->first<<endl;
//        double diff = c->first / next->first;
//        if(diff>maxDiff) {
//          maxDiff = diff;
//          splitIter = next;
//        }
//        c++;
//        next = c; next++;
//      }
//      
//      // Create a set of just the indexes of the large coefficients, which are the coefficients larger than the largest 
//      // coefficient drop and are not irrelevantly small compared to the largest coefficient
//      set<int> coeffIdxes;
//      //cout << "sortedCoeff: ";
//      for(map<double, int>::reverse_iterator c=sortedCoeff.rbegin(); c!=splitIter && c->first>=largestCoeff*1e-3; c++) {
//        //cout << c->second<<"/"<<c->first<<" ";
//        coeffIdxes.insert(c->second);
//      }
//      //cout << endl;
//  
//      list<pair<int, string> > selCtxt;
//      
//      // Add the constant term, if it is in coeffIdxes
//      if(coeffIdxes.find(0) != coeffIdxes.end()) selCtxt.push_back(make_pair(0,""));
//        
//      int idx=1;
//      for(int degree=1; degree<=maxDegree; degree++) {
//        map<string, int> subNames;
//        selTerms2Names(numericCtxtNames, coeffIdxes, selCtxt, 0, degree, idx, subNames/*""*/);
//      }
//      
//      // Sort the selected names according to the size of their coefficients
//      map<double, pair<int, string> > selCtxtSorted;
//      for(list<pair<int, string> >::iterator c=selCtxt.begin(); c!=selCtxt.end(); c++)
//        selCtxtSorted[sortedCoeffInv[c->first]] = *c;
//      
//      //cout << "selCtxtSorted="<<selCtxtSorted.size()<<" #selCtxt="<<selCtxt.size()<<endl;
//      
//      // Serialize the polynomial fit terms in the order from largest to smallest coefficient
//      ostringstream polyStr;
//      for(map<double, pair<int, string> >::reverse_iterator c=selCtxtSorted.rbegin(); c!=selCtxtSorted.rend(); c++) {
//        if(c!=selCtxtSorted.rbegin()) polyStr << " + ";
//        polyStr << setiosflags(ios::scientific) << setprecision(2) << 
//                   gsl_vector_get(polyfitCoeff, c->second.first) << 
//                   (c->second.second==""?"":"*") << 
//                   c->second.second;
//      }
//      polynomials.push_back(polyStr.str());
//    // If we didn't get enough observations to train a model
//    } else
//      polynomials.push_back("");
//
//    //gsl_vector_free(polyfitObs[i]);
//    gsl_vector_free(polyfitCoeff);
//  }
//  
//  gsl_matrix_free(polyfitCov);
//  
//  return polynomials;
//}

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void hierGraph::observe(int traceID,
                     const map<string, string>& ctxt, 
                     const map<string, string>& obs,
                     const map<string, anchor>& obsAnchor) {
  /*cout << "hierGraph::observe("<<traceID<<") hierGraphID="<<hierGraphID<<" #ctxt="<<ctxt.size()<<" #obs="<<obs.size()<<endl;
  cout << "    ctxt=";
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) { cout << c->first << "=>"<<c->second<<" "; }
  cout << endl;
  cout << "    obs=";
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) { cout << o->first << "=>"<<o->second<<" "; }
  cout << endl;*/
  
  // Record the names of the trace attributes and verify that all observations have the same ones
  set<string> curTraceAttrNames;
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++)
    curTraceAttrNames.insert(o->first);
  if(numObs == 0) traceAttrNames = curTraceAttrNames;
  else if(traceAttrNames != curTraceAttrNames)
  { 
    /*cerr << "ERROR: Inconsistent trace attributes in different observations for the same hierGraph node "<<hierGraphID<<"!"<<endl;
    cerr << "Before observed "<<traceAttrNames.size()<<" trace attributes: ["; 
    for(set<string>::iterator t=traceAttrNames.begin(); t!=traceAttrNames.end(); t++) {
      if(t!=traceAttrNames.begin()) cerr << ", ";
      cerr << *t;
    }
    cerr << "]."<<endl;
    cerr <<"This observation has "<<curTraceAttrNames.size()<<": [";
    for(set<string>::iterator t=curTraceAttrNames.begin(); t!=curTraceAttrNames.end(); t++) {
      if(t!=curTraceAttrNames.begin()) cerr << ", ";
        cerr << *t;
    }
    cerr << "]."<<endl;
    assert(0);*/
    // Add any new names in curTraceAttrNames to traceAttrNames
    set<string> newTraceAttrNames;
    set_union(traceAttrNames.begin(), traceAttrNames.end(), curTraceAttrNames.begin(), curTraceAttrNames.end(), 
              inserter(newTraceAttrNames, newTraceAttrNames.begin()));
    traceAttrNames = newTraceAttrNames;
  }
  
  // Record the context attribute groupings and the names of the attributes within each one
  map<string, list<string> > curCtxtNames;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    string hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName;
    decodeCtxtName(c->first, hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName);
    curCtxtNames[hierGraphClass+":"+ctxtGrouping+":"+ctxtSubGrouping].push_back(attrName);
  }
  if(numObs==0) ctxtNames = curCtxtNames;
  else if(ctxtNames != curCtxtNames) { 
    cerr << "ERROR: Inconsistent context attributes in different observations for the same hierGraph node "<<hierGraphID<<"!"<<endl;
    cerr << "Before observed "<<ctxtNames.size()<<" context attributes: "<<endl; 
    for(map<string, list<string> >::iterator c=ctxtNames.begin(); c!=ctxtNames.end(); c++) {
      cerr << "    "<<c->first<<" : ";
      for(list<string>::iterator i=c->second.begin(); i!=c->second.end(); i++) {
        if(i!=c->second.begin()) cerr << ", ";
        cerr << *i;
      }
      cerr << endl;
    }
    cerr <<"This observation has "<<curCtxtNames.size()<<":"<<endl;
    for(map<string, list<string> >::iterator c=curCtxtNames.begin(); c!=curCtxtNames.end(); c++) {
      cerr << "    "<<c->first<<" : ";
      for(list<string>::iterator i=c->second.begin(); i!=c->second.end(); i++) {
        if(i!=c->second.begin()) cerr << ", ";
        cerr << *i;
      }
      cerr << endl;
    }
    assert(false);
  }
  
  numObs++;
  
  //cout << "#curTraceAttrNames="<<curTraceAttrNames.size()<<", curCtxtNames="<<curCtxtNames.size()<<", numObs="<<numObs<<endl;
  
/*  // Maps the names of numeric contexts to their floating point values
  map<string, double> numericCtxt;
  list<string> curNumericCtxtNames;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    attrValue val(c->second, attrValue::unknownT);
    // If this value is numeric
    if(val.getType()==attrValue::intT || val.getType()==attrValue::floatT) {
      numericCtxt[c->first] = val.getAsFloat();
      curNumericCtxtNames.push_back(c->first);
    }
  }
  if(numObs==0) numericCtxtNames = curNumericCtxtNames;
  else if(numericCtxtNames != curNumericCtxtNames)
  { cerr << "ERROR: Inconsistent numeric context attributes in different observations for the same hierGraph node "<<hierGraphID<<"! Before observed "<<numericCtxtNames.size()<<" numeric context attributed but this observation has "<<curNumericCtxtNames.size()<<"."<<endl; assert(false); }
  
  
  //cout << "    #numericCtxt="<<numericCtxt.size()<<endl;
  // Only bother computing the polynomial fit if any of the context attributes are numeric
  if(numericCtxt.size()>0) {
    // Read out the names of the observation's trace attributes
    for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
      traceAttrNames.insert(o->first);
      // If this is the first time we've encountered this trace attribute, map it to a fresh column number
      if(traceAttrName2Col.find(o->first) == traceAttrName2Col.end()) {
        assert(traceAttrName2Count.size() == traceAttrName2Col.size());
        int newCol = traceAttrName2Col.size();
        traceAttrName2Col[o->first] = newCol;

        // We have not yet recorded any observations for this trace attribute (we will later in this function)
        traceAttrName2Count.push_back(0);
      }
    }

    int maxDegree = 2;
    // The total number of polynomial terms to maxDegree: 

    long numTerms = (numericCtxt.size()==1 ? maxDegree+1:
                         // #numericCtxt^0 + #numericCtxt^1 + #numericCtxt^2 + ... + #numericCtxt^maxDegree = #numericCtxt^(maxDegree+1) - 1
                         pow(numericCtxt.size(), maxDegree+1));

    //cout << "hierGraph::observe() hierGraphID="<<hierGraphID<<", #numericCtxt="<<numericCtxt.size()<<", #polyfitCtxt="<<polyfitCtxt.size()<<", found="<<(polyfitCtxt.find(hierGraphID) != polyfitCtxt.end())<<", numTerms="<<numTerms<<endl;

    // If this is the first observation we have from the given traceStream, allocate the
    // polynomial fit datastructures
    if(numObs==0) {
      polyfitCtxt = gsl_matrix_alloc(1000, numTerms);

      //for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
      //  polyfitObs.push_back(gsl_vector_alloc(1000));
      //}
      polyfitObs = gsl_matrix_alloc(1000, traceAttrName2Col.size());

      numObs = 0;
      numAllocObs = 1000;
      numAllocTraceAttrs = traceAttrName2Col.size();

      //cout << "    Allocated "<<numAllocObs<<" rows, "<<numAllocTraceAttrs<<" columns"<<endl;
    }
    //cout << "    #polyfitCtxt="<<polyfitCtxt.size()<<", found="<<(polyfitCtxt.find(hierGraphID) != polyfitCtxt.end())<<", numObs="<<numObs<<", numAllocObs="<<numAllocObs<<endl;

    //cout << "traceAttrName2Col.size()="<<traceAttrName2Col.size()<<" numAllocTraceAttrs="<<numAllocTraceAttrs<<endl;
    // If we're out of space in polyfitCtxt and polyfitObs to store another observation or store more columns, grow them
    if(numObs == numAllocObs-1 || 
       traceAttrName2Col.size() > numAllocTraceAttrs) {
      // If we're out of space for rows, double it
      int newNumAllocObs = numAllocObs;
      if(numObs == numAllocObs-1) newNumAllocObs *= 2;

      // If we need new columns, adjust accordingly
      int newNumAllocTraceAttrs = numAllocTraceAttrs;
      if(traceAttrName2Col.size() > numAllocTraceAttrs)
        newNumAllocTraceAttrs = traceAttrName2Col.size();

      // Expand polyfitCtxt
      gsl_matrix* newCtxt = gsl_matrix_alloc(newNumAllocObs, numTerms);
      gsl_matrix_view newCtxtView = gsl_matrix_submatrix (newCtxt, 0, 0, numAllocObs, numTerms);
      gsl_matrix_memcpy (&(newCtxtView.matrix), polyfitCtxt);
      gsl_matrix_free(polyfitCtxt);
      polyfitCtxt = newCtxt;

      // Expand polyfitObs
      gsl_matrix* newObs = gsl_matrix_alloc(newNumAllocObs, newNumAllocTraceAttrs);
      gsl_matrix_view newObsView = gsl_matrix_submatrix (newObs, 0, 0, numAllocObs, numAllocTraceAttrs);
      gsl_matrix_memcpy (&(newObsView.matrix), polyfitObs);
      gsl_matrix_free(polyfitObs);
      polyfitObs = newObs;

      // Update our allocated space records
      numAllocObs = newNumAllocObs;
      numAllocTraceAttrs = newNumAllocTraceAttrs;
      //cout << "    Reallocated "<<numAllocObs<<" rows, "<<numAllocTraceAttrs<<" columns, numObs="<<numObs<<endl;
    }

    // Add this observation to polyfitObs
    int obsIdx=0;
    // Records whether this observation corresponds to a new context row
    bool newContext = false;

    //cout << "    Adding Observations:"<<endl;
    for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++, obsIdx++) {
      // Skip any non-numeric observed values
      attrValue val(o->second, attrValue::unknownT);
      if(val.getType() != attrValue::intT && val.getType() != attrValue::floatT) continue;
      double v = val.getAsFloat();
      
      int traceAttrCol = traceAttrName2Col[o->first];
      / *cout << "        "<<o->first<<"  traceAttrCol="<<traceAttrCol<<
                 ", numAllocTraceAttrs="<<numAllocTraceAttrs<<
                 ", traceAttrName2Count[traceAttrCol]="<<traceAttrName2Count[traceAttrCol]<<
                 ", numObs="<<numObs<<endl;* /

      assert(traceAttrCol < numAllocTraceAttrs);
      gsl_matrix_set(polyfitObs, traceAttrName2Count[traceAttrCol], traceAttrCol, v);

      // Increment the number of values written into the current column
      traceAttrName2Count[traceAttrCol]++;

      // If this observation has started a new row in the observations matrix
      if(traceAttrName2Count[traceAttrCol] > numObs) {
        assert(traceAttrName2Count[traceAttrCol] == numObs+1);
        // Update numObs to correspond to the maximum number of values written to any column
        numObs = traceAttrName2Count[traceAttrCol];

        // Record that this is a new context, which should be written into polyfitCtxt
        newContext = true;
      }
    }

    // Add the context of the observation to polyfitCtxt

    // The first entry corresponds to the degree-0 constant term
    gsl_matrix_set(polyfitCtxt, numObs, 0, 1);

    // Add all the polynomial terms of degrees upto and including maxDegree
    int col=1;
    for(int degree=1; degree<=maxDegree; degree++)
      addPolyTerms(numericCtxt, 0, degree, numObs, col, 1, polyfitCtxt);
  }
  */
  // Forward the observation to observers of this object
  emitObservation(traceID, ctxt, obs, obsAnchor/*, observers*/);
}

/*************************
 ***** HG_polyFitFilter *****
 *************************/

// The total number of HG_polyFitFilter instances that have been created so far. 
// Used to set unique names to files output by polyFitFilters.
int HG_polyFitFilter::maxFileNum=0;

// Records whether the working directory for this class has been initialized
bool HG_polyFitFilter::workDirInitialized=false;

// The directory that is used for storing intermediate files
std::string HG_polyFitFilter::workDir="";

HG_polyFitFilter::HG_polyFitFilter() {
  numObservations = 0;
  fileNum = maxFileNum++;
  
  // Initialize the directories this HG_polyFitFilter will use for its temporary storage
  if(!workDirInitialized) {
    // Create the directory that holds the workDirInitialized-specific data
    std::pair<std::string, std::string> dirs = dbg.createWidgetDir("HG_polyFitFilter");
    workDir = dirs.first;
  }
  workDirInitialized = true;
}

// Iterates over all combinations of keys in numericCtxt upto maxDegree in size and computes the products of
// their values. Adds each such product to the given vector termVals.
void HG_polyFitFilter::addPolyTerms(const map<string, double>& numericCtxt, int termCnt, int maxDegree, 
                                 std::vector<double>& termVals, double product) {
  // If product contains maxDegree terms, add it to the given column of polyfitObs and increment the column counter
  if(termCnt==maxDegree) {
    termVals.push_back(product);
  } else {
    // Iterate through all the possible choices of the next factor in the current polynomial term
    for(map<string, double>::const_iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++) {
      addPolyTerms(numericCtxt, termCnt+1, maxDegree, termVals, product * c->second);
    }
  }
}

// Determines which context or trace attributes are numeric and return a mapping of their names to their numeric values
//    (if numObservations>0, only the attributes that are currently in numericAttrNames)
// Updates numericAttrNames to be the the intersection of itself and the numeric 
//    attributes of this observation (keys of data)
// data - maps context/trace attribute name to their observed values
// numNumeric - refers to the count of numeric context/trace attributes
// numericAttrNames - refers to the list of names of numeric context/trace attributes
// label - identifies this as context or trace (for error messages)
map<string, /*double*/string> HG_polyFitFilter::getNumericAttrs(
                                    const map<string, string>& ctxttraceData, 
                                    std::set<std::string>& numericAttrNames, 
                                    string label) {
  /* // If this is not the first observation, we'll iterate through the names of the 
  // numeric attributes in prior observations to ensure they're the same
  list<string>::iterator priorObsNumAttrNames;
  if(numObservations>0) 
    priorObsNumAttrNames = numericAttrNames.begin();*/
    
  // Iterate over all the context/trace attributes, store them into numericData 
  // and check that the numeric attribute names are the same in all observations
  map<string, /*double*/string> numericData; // Maps the names of numeric attributes to their floating point values
  
  // The names of the numeric attributes in this observation
  //set<string> curNumericAttrNames;
  
  // If this is the first observation, initialize numericAttrNames to be the keys of ctxttraceData
  if(numObservations==0) {
    for(map<string, string>::const_iterator d=ctxttraceData.begin(); d!=ctxttraceData.end(); d++)
      numericAttrNames.insert(d->first);
  }
  
  for(std::set<std::string>::iterator i=numericAttrNames.begin(); i!=numericAttrNames.end(); ) {
    string name = *i; i++;
    map<string, string>::const_iterator valIt = ctxttraceData.find(name);
    
    // If the current attribute does not exist in this observation
    if(valIt == ctxttraceData.end())
      // Remove it from numericAttrNames
      numericAttrNames.erase(name);
    // If the attribute does exist
    else {
      attrValue val(valIt->second, attrValue::unknownT);

      // If this value is numeric
      if(val.getType()==attrValue::intT || val.getType()==attrValue::floatT) {
        // Record the value in numericCtxt and the name in curNumericCtxtNames.push_back
        //numericData[d->first] = val.getAsFloat();
        numericData[name] = valIt->second;

        // If this is the first observation, initialize the numeric attribute names from it  
        //if(numObservations==0) numericAttrNames.insert(d->first);
        /* // Otherwise, verify that the this observation has the same set of numeric attributes as prior ones did
        else {
          if(*priorObsNumAttrNames != d->first)
          { cerr << "ERROR: Inconsistent numeric "<<label<<" attributes in different observations for the same HG_polyFitFilter "<<fileNum<<"! Expecting attribute "<<*priorObsNumAttrNames<<" but observed attribute "<<d->first<<" instead!"<<endl; assert(false); }
          priorObsNumAttrNames++;
        }*/
      // If this attribute is not numeric, remove it from numericAttrNames
      } else
        numericAttrNames.erase(name);
    }
  }
  // Make sure that all observations have the same number of numeric attributes
  //if(numericData.size() != numericAttrNames.size())
  //{ cerr << "ERROR: Inconsistent number of numeric "<<label<<" attributes in different observations for the same HG_polyFitFilter "<<fileNum<<"! Before observed "<<numericAttrNames.size()<<" numeric context attributed but this observation has "<<numericData.size()<<"."<<endl; assert(false); }
  
  return numericData;
}

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void HG_polyFitFilter::observe(int traceID,
                            const std::map<std::string, std::string>& ctxt, 
                            const std::map<std::string, std::string>& obs,
                            const std::map<std::string, anchor>&      obsAnchor) {
  /*cout << "HG_polyFitFilter::observe() this="<<this<<endl;
  cout << "ctxt="<<endl<<HG_data2str(ctxt)<<endl;
  cout << "obs="<<endl<<HG_data2str(obs)<<endl;*/
  
  // Determine which context and trace attributes are numeric and 
  // check that all observations have the same numeric context and trace dimensions
  map<string, /*double*/string> numericCtxt  = getNumericAttrs(ctxt, numericCtxtNames,  "context");
  map<string, /*double*/string> numericTrace = getNumericAttrs(obs,  numericTraceNames, "trace");
  
  /*cout << "numericCtxt="<<endl<<HG_data2str(numericCtxt)<<endl;
  cout << "numericTrace="<<endl<<HG_data2str(numericTrace)<<endl;
  
  cout << "#numericCtxtNames="<<numericCtxtNames.size()<<", #numericTraceNames="<<numericTraceNames.size()<<endl;*/
  
  /*  int maxDegree = 2;
  // The total number of polynomial terms to maxDegree:
  long numTerms = (numericCtxt.size()==1 ? maxDegree+1:
                         // #numericCtxt^0 + #numericCtxt^1 + #numericCtxt^2 + ... + #numericCtxt^maxDegree = #numericCtxt^(maxDegree+1) - 1
                         pow(numericCtxt.size(), maxDegree+1));*/

  // Compute all the polynomial terms of degrees upto and including maxDegree
  /*vector<double> termVals;
  for(int degree=1; degree<=maxDegree; degree++)
    addPolyTerms(numericCtxt, 0, degree, termVals);*/
  
  // Track the context attributes with constant values
  for(map<string, /*double*/string>::iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++) {
    // If this is the first observation to be observed by this object
    if(numObservations==0) {
      // Initialize ctxtConstVals to the first observed value for each context attribute
      ctxtConstVals[c->first] = c->second;
    } else {
      // Remove from ctxtConstVals any context attributes that have multiple values
      if(ctxtConstVals.find(c->first) != ctxtConstVals.end() &&
         ctxtConstVals[c->first] != c->second) {
        ctxtConstVals.erase(c->first);
      }
    }
  }

  // Record the observed data
  dataCtxt.push_back(numericCtxt);
  dataTrace.push_back(numericTrace);
  
//  // For each numeric trace attribute, write to its file the observation's entire numeric 
//  // context and the value of this attribute.
//  for(map<string, /*double*/string>::iterator t=numericTrace.begin(); t!=numericTrace.end(); t++) {
//    // If this is the first observation to be observed by this object
//    if(numObservations==0) {
//      // Create files for all the numeric trace attributes 
//      //string outFName = txt()<<"HG_polyFitFilter.data."<<fileNum<<"."<<t->first;
//      //outFiles[t->first] = new ofstream(outFName.c_str());
//      outProcessors[t->first] = 
//              new externalTraceProcessor_File(string(ROOT_PATH)+"/widgets/funcFit/funcFit", 
//                                              easylist<string>(txt()<<workDir<<"/in"<<fileNum<<".cfg"),
//                                              txt()<<workDir<<"/in"<<fileNum<<"."<<t->first<<".data",
//                                              easylist<string>(txt()<<workDir<<"/in"<<fileNum<<"."<<t->first<<".log",
//                                                               "\""+t->first+"\""));
//     
//      /*// Write the column headers
//      for(map<string, double>::iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++)
//        *(outFiles[t->first]) << c->first << "\t";
//      *(outFiles[t->first]) << t->first << "\n";*/
//    }
//    /*for(map<string, double>::iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++)
//      *(outFiles[t->first]) << c->second << "\t";
//    *(outFiles[t->first]) << t->second << "\n";*/
//    
//    // Add a constant term
//    numericCtxt["hierGraph:HG_polyFitFilter:constant:constant"] = attrValue("1", attrValue::intT).serialize();
//    
//    std::map<std::string, std::string> numTraceVal;
//    numTraceVal[t->first] = t->second;
//    std::map<std::string, anchor> numTraceAnchor;
//    outProcessors[t->first]->observe(traceID, numericCtxt, numTraceVal, numTraceAnchor);
//  }
//
//  // If we've just created a bunch of output processors, insert them between this object and its observers
//  if(numObservations==0) {
//    set<traceObserver*> outProcObservers;
//    for(map<string, externalTraceProcessor_File*>::iterator p=outProcessors.begin(); p!=outProcessors.end(); p++)
//      outProcObservers.insert(p->second);
//    prependObservers(outProcObservers);
//  }
  
  numObservations++;
}

// Called when the stream of observations has finished to allow the implementor to perform clean-up tasks.
// This method is optional.
void HG_polyFitFilter::obsFinished() {
  //cout << "HG_polyFitFilter::obsFinished() this="<<this<<endl;
  // Remove from numericCtxtNames all the constant contexts
  for(map<string, string>::iterator c=ctxtConstVals.begin(); c!=ctxtConstVals.end(); c++)
    numericCtxtNames.erase(c->first);

  // If we've not collected at least one observation or all the context dimensions are constant, 
  // don't do any function fitting and just stop
  if(numObservations==0 || (numericCtxtNames.size() - ctxtConstVals.size())==0) return;
  
  // Set of all the externalTraceProcessor that will listen to this hierGraph's observations
  // and run them through funcFit
  set<traceObserver*> outProcessors;
  
  // For each numeric trace attribute, write to its file the observation's entire numeric 
  // context and the value of this attribute.
  for(set<string>::iterator t=numericTraceNames.begin(); t!=numericTraceNames.end(); t++) {
    //cout << "Trace attribute "<<*t<<endl;
    traceObserver* out = 
            new externalTraceProcessor_File(string(ROOT_PATH)+"/widgets/funcFit/funcFit", 
                                            easylist<string>(txt()<<workDir<<"/in"<<fileNum<<".cfg"),
                                            txt()<<workDir<<"/in"<<fileNum<<"."<<*t<<".data",
                                            easylist<string>("\""+*t+"\""/*,
                                                             txt()<<workDir<<"/in"<<fileNum<<"."<<*t<<".log"*/));
    outProcessors.insert(out);
    
    // Pass all the observations to this processor
    assert(dataCtxt.size() == dataTrace.size());
    std::list<std::map<std::string, std::string> >::iterator dc = dataCtxt.begin();
    std::list<std::map<std::string, std::string> >::iterator dt = dataTrace.begin();
    
    for(; dc!=dataCtxt.end(); dt++, dc++) {
      /*cout << "dc="<<endl<<HG_data2str(*dc)<<endl;
      cout << "dt="<<endl<<HG_data2str(*dt)<<endl;*/
    
      // If this is the first pass through data
      if(t==numericTraceNames.begin()) {
        // Trim the context attributes of all the observations to remove ones with values that were 
        // constant or were subsequently discovered to be not consistently numeric
        for(map<string, string>::iterator c=dc->begin(); c!=dc->end();) {
          // If the current context attribute is not a member of numericCtxtNames, remove it
          if(numericCtxtNames.find(c->first) == numericCtxtNames.end())
            dc->erase(c++);
          else
            c++;
        }
        
        // Add a constant term to the current observation's context
        (*dc)["hierGraph:HG_polyFitFilter:constant:constant"] = attrValue("1", attrValue::intT).serialize();
      }

      // Put together a map that holds just the value of the current trace attribute in this observation
      std::map<std::string, std::string> numTraceVal;
      assert(dt->find(*t) != dt->end());
      numTraceVal[*t] = (*dt)[*t];
      
      // An empty anchors map
      std::map<std::string, anchor> numTraceAnchor;
      
      // Pass the observation
      out->observe(-1, *dc, numTraceVal, numTraceAnchor);
    }
  }

  // Insert the newly-created output processors between this object and its observers
  prependObservers(outProcessors);
  
  // Now that all the data has been emitted to all the processors, set up the input files
  // for the funcFit runs that will do the processing
  
  // Write out the configuration file before the command runs
  {
    ofstream cfgFile((txt()<<workDir<<"/in"<<fileNum<<".cfg").c_str(), ofstream::out);

    // Number of data lines
    cfgFile << "N "<<numObservations<<endl;

    // Number of parameters
    cfgFile << "p "<<(numericCtxtNames.size()+1)<<endl;

    // Parameter names
    int l=0;
    // Polynomial terms
    for(set<string>::const_iterator c=numericCtxtNames.begin(); c!=numericCtxtNames.end(); c++, l++)
      cfgFile << "pname "<<l<<" "<<*c<<endl;
    // Constant term
    cfgFile << "pname "<<l<<" hierGraph:HG_polyFitFilter:constant:constant"<<endl;

    // Parameter links (one param per link)
    cfgFile << "L "<<(numericCtxtNames.size()+1);
    // Polynomial terms
    for(set<string>::const_iterator c=numericCtxtNames.begin(); c!=numericCtxtNames.end(); c++)
      cfgFile << " 1";
    // Constant term
    cfgFile << " 1";
    cfgFile << endl;

    // Link identification (one param per link)
    l=0;
    // Polynomial terms
    for(set<string>::const_iterator c=numericCtxtNames.begin(); c!=numericCtxtNames.end(); c++, l++)
      cfgFile << "l "<<l<<" "<<l<<endl;
    // Constant Term
    cfgFile << "l "<<l<<" "<<l<<endl;

    // Function assignment
//      l=0;
//      /*cfgFile << "#ctxtConstVals="<<ctxtConstVals.size()<<endl;
//      for(map<std::string, std::string>::iterator c=ctxtConstVals.begin(); c!=ctxtConstVals.end(); c++)
//        cfgFile << "    "<<c->first<<"|"<<c->second<<endl;*/
//      for(list<string>::const_iterator c=numericCtxtNames.begin(); c!=numericCtxtNames.end(); c++, l++) {
//        //cfgFile << "num: "<<*c<<endl;
//        // Specify empty functions for constant terms
//        if(ctxtConstVals.find(*c) != ctxtConstVals.end())
//          cfgFile << "F "<<l<<" 0 0"<<endl;
///*        else
//          for(int degree=2; degree<=2; degree++)
//            cfgFile << "F "<<l<<" 1 "<<degree<<endl;*/
//      }

    // GA Parameters
    cfgFile << "GAT 10"<<endl;
    cfgFile << "GAS 12"<<endl;
  }

  // Finish the external processors to run funcFit and forward the results to downstream observers
  for(set<traceObserver*>::iterator f=outProcessors.begin(); f!=outProcessors.end(); f++) {
    (*f)->obsFinished();
    delete *f;
  }
}

/***************************
 ***** HG_polyFitObserver *****
 ***************************/

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void HG_polyFitObserver::observe(int traceID,
                     const map<string, string>& ctxt, 
                     const map<string, string>& obs,
                     const map<string, anchor>& obsAnchor) {
  /*cout << "HG_polyFitObserver::observe("<<traceID<<") #ctxt="<<ctxt.size()<<" #obs="<<obs.size()<<endl;
  cout << "    ctxt=";
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) { cout << c->first << "=>"<<c->second<<" "; }
  cout << endl;
  cout << "    obs=";
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) { cout << o->first << "=>"<<o->second<<" "; }
  cout << endl;*/
  
  // Do not add polynomial fit text when we've received no fit data (likely because there were no observations)
  if(ctxt.size()==0 && obs.size()==0) return;

  // Add the name of the trace attribute (output or measurement) for which the fit was computed
  map<string, string>::const_iterator traceFullAttrNameIt = obs.find("label");
  assert(traceFullAttrNameIt != obs.end());
  attrValue traceFullAttrName(traceFullAttrNameIt->second, attrValue::unknownT);

  string traceHierGraphClass, traceGrouping, traceSubGrouping, traceAttrName;
  hierGraph::decodeCtxtName(traceFullAttrName.getAsStr(), traceHierGraphClass, traceGrouping, traceSubGrouping, traceAttrName);
  
  // The number of non-zero term coefficients encountered so far
  int numNonZeroCoeff=0;
  
  list<string> terms;
  
  // Add the string representation of the functional form, one term at a time
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    string hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName;
    hierGraph::decodeCtxtName(c->first, hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName);
    attrValue val(c->second, attrValue::unknownT);

    // For non-zero constants
    if(val.getAsFloat() != 0) {
      // Multiply coefficient by the context name. Omit the coefficient if it is 1 and 
      // omit the context name if it is the constant term
      string t="";
      if(val.getAsFloat()!=1)
        t = val.getAsStr();
      if(ctxtGrouping!="HG_polyFitFilter" || ctxtSubGrouping!="constant" || attrName != "constant") {
        if(t != "") t += "*";
        t += ctxtGrouping + ":" + ctxtSubGrouping + ":" + attrName;
      }
      
      // Add the term if it is not just 1*constant
      if(t!="") {
        terms.push_back(t);
        numNonZeroCoeff++;
      }
    }
  }
  
  // If all the coefficients were 0, set "traceAttrName = 0" as the formula
  if(numNonZeroCoeff==0)
    terms.push_back("0");
  
  // Record this fit
  fits[traceGrouping + ":" + traceSubGrouping + ":" + traceAttrName] = terms;
}

// Returns the formatted text representation of the fits, to be included in the HTML table 
// that encodes each hierGraph's graph node
std::string HG_polyFitObserver::getFitText() const
{
  string ret;
  
  ret += "\t<TR><TD><TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"1\">\n";
  for(map<string, list<string> >::const_iterator f=fits.begin(); f!=fits.end(); f++) {
    ret += "\t\t<TR><TD>:" + f->first + "</TD><TD>:=</TD><TD>:";
    for(list<string>::const_iterator t=f->second.begin(); t!=f->second.end(); t++) {
      // Connect the terms with +'s
      if(t!=f->second.begin()) ret += " + ";
      ret += *t;
    }
    ret += "</TD></TR>\n";
  }
  ret += "\t</TABLE></TD></TR>\n";
  
  return ret;
}

// hoa edit
// Returns the formatted text representation of the fits, not include the HTML table
// that encodes each hierGraph's graph node
std::string HG_polyFitObserver::saveFitText() const
{
  string ret;

  for(map<string, list<string> >::const_iterator f=fits.begin(); f!=fits.end(); f++)
  {
    ret += f->first + "=";
    for(list<string>::const_iterator t=f->second.begin(); t!=f->second.end(); t++)
    {
      // Connect the terms with +'s
      if(t!=f->second.begin()) ret += " + ";
      ret += *t;
    }
    ret += ",";
  }
  return ret;
}
  
/*****************************
 ***** hierGraphTraceStream *****
 *****************************/

hierGraphTraceStream::hierGraphTraceStream(properties::iterator props, traceObserver* observer) : 
  traceStream(properties::next(props), txt()<<"CanvizBox_node"<<properties::getInt(props, "hierGraphID"), false)
{
  assert(props.name() == "hierGraphTS");
    
  hierGraphID   = props.getInt("hierGraphID");
  name       = props.get("name");
  numInputs  = props.getInt("numInputs");
  numOutputs = props.getInt("numOutputs");
  
  // Get the currently active hierGraph that this traceStream belongs to
  /*assert(hierGraphApp::hgStack.size()>0);
  hierGraph* m = hierGraphApp::activeMA->hgStack.back();*/

  /*cout << "hierGraphApp::activeMA->hierGraphTraces=";
  for(map<int, traceStream*>::iterator i=hierGraphApp::activeMA->hierGraphTraces.begin(); i!=hierGraphApp::activeMA->hierGraphTraces.end(); i++)
    cout << "    "<<i->first<<" : "<<i->second<<endl;
  cout << "props="<<props.str()<<endl;*/

  assert(hierGraphApp::activeMA->hierGraphTraces.find(hierGraphID) == hierGraphApp::activeMA->hierGraphTraces.end());
  
  // Create a new traceStream object to collect the observations for this hierGraph group
  hierGraphApp::activeMA->hierGraphTraces[hierGraphID] = this;
  
  //cout << "hierGraphTraceStream::hierGraphTraceStream() this="<<this<<", traceID="<<getID()<<", hierGraphID="<<hierGraphID<<endl;
  
  // If no observer is specified, register the current instance of hierGraphApp to listen in on observations recorded by this traceStream
  if(observer==NULL) {
    // Create a fresh instance of hierGraph to analyze data of this stream
    mFilter = new hierGraph(hierGraphID);

    polyFitter = new HG_polyFitFilter();
    polyFitCollector = new HG_polyFitObserver();
    polyFitter->registerObserver(polyFitCollector);

    registerObserver(mFilter);
    registerObserver(polyFitter);
    registerObserver(this);
    
    hierGraphApp::registerHierGraph(hierGraphID, mFilter, polyFitCollector);
    
    // The queue of observation filters
/*    queue = new traceObserverQueue(traceObservers(
                    // - Observations pass through a new instance of hierGraph to enable it to build polynomial 
                    // fits of this data
                    new HG_polyFitFilter(),
                    mFilter,
                    // - They then end up at the original traceStream to be included in the generated visualization
                    this));
    registerObserver(queue);*/
  }
  // If an observer is specified, then that observer must be doing the processing. We assme that this derived class
  // sets mFilter and queue.
  
  
  // Record the mapping between traceStream IDs and the IDs of the hierGraph group they're associated with
  hierGraphApp::activeMA->trace2hierGraphID[getID()] = hierGraphID;
  ///cout << "hierGraphID="<<hierGraphID<<", traceID="<<m->hierGraphTraces[hierGraphID]->getID()<<endl;
}

hierGraphTraceStream::~hierGraphTraceStream() {
  if(mFilter)          { delete mFilter; }
  if(polyFitter)       { delete polyFitter; }
  if(polyFitCollector) { delete polyFitCollector; }
  //if(queue)   delete queue;
}

// Called when we observe the entry tag of a hierGraphTraceStream
void *hierGraphTraceStream::enterTraceStream(properties::iterator props) {
  //cout << "hierGraphApp::enterTraceStream props="<<props.str()<<endl;
  assert(props.name() == "hierGraphTS");
  //string hierGraphName = properties::get(nameProps, "HierGraphName");
  
  // Allocate a new hierGraphTraceStream. The constructor takes care of registering it with the currently active hierGraph
  new hierGraphTraceStream(props);
  
  return NULL;
}

/*********************************
 ***** compHierGraphTraceStream *****
 *********************************/

compHierGraphTraceStream::compHierGraphTraceStream(properties::iterator props, traceObserver* observer) :
  hierGraphTraceStream(props.next(), this)
{
  /*cout << "compHierGraphTraceStream::compHierGraphTraceStream() traceID="<<getID()<<", observer="<<observer<<endl;
  cout << "props="<<props.str()<<endl;*/
  // If no observer is specified, register a filtering queue containing a compHierGraph, followed by the the current instance of 
  // hierGraphApp to listen in on observations recorded by this traceStream.
  if(observer==NULL) {
    cmFilter = new compHierGraph(/*properties::getInt(props, "isReference"), common::hierGraph::context(props, "op")*/);

    // Load the comparators to be used with each input
    for(int i=0; i<numInputs; i++) {
      cmFilter->inComparators.push_back(map<string, comparator*>());
      
      int numCtxtAttrs = props.getInt(txt()<<"in"<<i<<":numAttrs");
      for(int c=0; c<numCtxtAttrs; c++) {
        escapedStr comparator(props.get(txt()<<"in"<<i<<":compare"<<c), ":", escapedStr::escaped);
        vector<string> fields = comparator.unescapeSplit(":");
        assert(fields.size()==3);
        // The name of the input attribute
        string inName = fields[0];
        // The name of the comparator to be used for this attribute
        string compName = fields[1];
        // The description of the comparator
        string compDesc = fields[2];
        
        // Generate a comparator object based on the encoded comparator name and description and store it in the compHierGraph
        cmFilter->inComparators[i][inName] = attrValueComparatorInstantiator::genComparator(compName, compDesc);
//cout << "inComparators["<<i<<"]["<<inName<<"] => "<<compName<<" , "<<compDesc<<endl;
      }
    }
    
    // Load the comparators to be used with each output
    for(int i=0; i<numOutputs; i++) {
      cmFilter->outComparators.push_back(map<string, comparator*>());
      
      int numCtxtAttrs = props.getInt(txt()<<"out"<<i<<":numAttrs");
      for(int c=0; c<numCtxtAttrs; c++) {
        escapedStr comparator(props.get(txt()<<"out"<<i<<":compare"<<c), ":", escapedStr::escaped);
        vector<string> fields = comparator.unescapeSplit(":");
        assert(fields.size()==3);
        // The name of the output attribute
        string outName = fields[0];
        // The name of the comparator to be used for this attribute
        string compName = fields[1];
        // The description of the comparator
        string compDesc = fields[2];
        
        // Generate a comparator object based on the encoded comparator name and description and store it in the compHierGraph
        cmFilter->outComparators[i][outName] = attrValueComparatorInstantiator::genComparator(compName, compDesc);
//cout << "outComparators["<<i<<"]["<<outName<<"] => "<<compName<<" , "<<compDesc<<endl;
      }
    }
    
    // Load the comparators to be used with each measurement
    int numMeasurements = props.getInt("numMeasurements");
    for(int m=0; m<numMeasurements; m++) {
      escapedStr comparator(props.get(txt()<<"measure"<<m), ":", escapedStr::escaped);
      vector<string> fields = comparator.unescapeSplit(":");
      assert(fields.size()==3);
      // The name of the measurement
      string measName = fields[0];
      // The name of the comparator to be used for this measurement
      string compName = fields[1];
      // The description of the comparator
      string compDesc = fields[2];
      
      // Generate a comparator object based on the encoded comparator name and description and store it in the compHierGraph
      cmFilter->measComparators[measName] = attrValueComparatorInstantiator::genComparator(compName, compDesc);
    }
    
    mFilter = new hierGraph(hierGraphID);
    
    polyFitter = new HG_polyFitFilter();
    polyFitCollector = new HG_polyFitObserver();
    polyFitter->registerObserver(polyFitCollector);

    // cmFilter observes this traceStream and performs comparisons
    registerObserver(cmFilter);
    
    // All the others observe the comparison observations that it emits
    cmFilter->registerObserver(mFilter);
    cmFilter->registerObserver(polyFitter);
    cmFilter->registerObserver(this);
    
    hierGraphApp::registerHierGraph(hierGraphID, mFilter, polyFitCollector);
    
    /*
    // The queue of observation filters
    queue = new traceObserverQueue(traceObservers(
                    // - filters the decoded data to replace the raw observations with comparisons between 
                    //   reference configurations and non-reference configurations
                    cmFilter, 
                    // - these observations pass through the instance of hierGraph that buids a polynomial to fit of this data
                    //hierGraphApp::getInstance(), 
                    mFilter,
                    // - finally it ends up at the original traceStream to be included in the generated visualization
                    this));
    registerObserver(queue);*/
  }
  //cout << "cmFilter="<<cmFilter<<", hierGraphApp::getInstance()="<<hierGraphApp::getInstance()<<", queue="<<queue<<endl;
}

compHierGraphTraceStream::~compHierGraphTraceStream() {
  // Deallocate the observation filter objects
  if(cmFilter) { delete cmFilter; cmFilter=NULL; }
  //if(mFilter)  { delete mFilter;  mFilter=NULL; }
  //if(queue)    { delete queue;    queue=NULL; }
}

// Called when we observe the entry tag of a compHierGraphTraceStream
void *compHierGraphTraceStream::enterTraceStream(properties::iterator props) {
  //cout << "hierGraphApp::enterTraceStream"<<endl;
  assert(props.name() == "compHierGraphTS");
  //string hierGraphName = properties::get(nameProps, "HierGraphName");
  
  //cout << "compHierGraphTraceStream::enterTraceStream() props="<<props.str()<<endl;
  // Allocate a new compHierGraphTraceStream. The constructor takes care of registering it with the currently active hierGraph
  new compHierGraphTraceStream(props);
  
  return NULL;
}

/**********************
 ***** compHierGraph *****
 **********************/

// Compare the value of each trace attribute value ctxtobs (a given context or observation) to the corresponding value 
// in ref (the corresponding reference observation) and return a mapping of each trace attribute to the serialized 
// representation of their relationship. Where a comparator is not provided add the raw value from ctxtobs to the 
// returned map.
/* // The set of trace attributes in ref must contain that in ctxtobs. */
std::map<std::string, std::string> compHierGraph::compareObservations(
                                         const std::map<std::string, attrValue>& ctxtobs,
                                         const std::map<std::string, attrValue>& ref) {
  map<string, string> relation;
  
  //assert(ctxtobs.size() <= ref.size());
  //cout << "compareObservations()\n";
  for(map<string, attrValue>::const_iterator o=ctxtobs.begin(); o!=ctxtobs.end(); o++) {
    map<string, attrValue>::const_iterator r=ref.find(o->first);

    string traceAttrType, traceGrouping, traceSubGrouping, traceAttrName;
    decodeCtxtName(o->first, traceAttrType, traceGrouping, traceSubGrouping, traceAttrName);
    //cout << "o->first="<<o->first<<", traceAttrType="<<traceAttrType<<", traceGrouping="<<traceGrouping<<", traceSubGrouping="<<traceSubGrouping<<", traceAttrName="<<traceAttrName<<endl;
    
    // Get the comparator to be used to compare the two the trace attribute values from ctxtobs and ref
    comparator* comp=NULL;
    
    // If the current trace attribute is an output
    if(traceGrouping == "input") {
      int inIdx = attrValue::parseInt(traceSubGrouping);
      //cout << "inIdx="<<inIdx<<", #outComparators="<<outComparators.size()<<", #outComparators[inIdx]="<<outComparators[inIdx].size()<<endl;
      
      // If there is a comparator for this input
      assert(inComparators.size()>=inIdx);
      if(inComparators[inIdx].find(traceAttrName) != inComparators[inIdx].end())
        // Get the comparator for this property of this output and compare the two attrValues
        comp = inComparators[inIdx][traceAttrName];
    
    // If the current trace attribute is a measurement
    } else if(traceGrouping == "output") {
      // ctxtobs and ref must have the same output attributes (map keys)
      assert(r != ref.end());
      
      int outIdx = attrValue::parseInt(traceSubGrouping);
      /*cout << "outIdx="<<outIdx<<", #outComparators="<<outComparators.size()<<", #outComparators[outIdx]="<<outComparators[outIdx].size()<<endl<<"    ";
      for(map<std::string, comparator*>::iterator c=outComparators[outIdx].begin(); c!=outComparators[outIdx].end(); c++)
        cout << " "<<c->first;
      cout << endl;*/
      
      // Get the comparator for this property of this output and compare the two attrValues
      assert(outComparators.size()>=outIdx);
      comp = outComparators[outIdx][traceAttrName];
    
    // If the current trace attribute is a measurement
    } else if(traceGrouping == "measure") {
      // ctxtobs and ref must have the same measure attributes (map keys)
      assert(r != ref.end());
      
      /*cout << "traceGrouping="<<traceGrouping<<", measComparators: (#"<<measComparators.size()<<") ";
      for(std::map<std::string, comparator*>::iterator mc=measComparators.begin(); mc!=measComparators.end(); mc++)
        cout << " "<<mc->first;
      cout << endl;*/
      
      // Get the comparator for this property of this output and compare the two attrValues
      assert(measComparators.find(traceSubGrouping) != measComparators.end());
      comp = measComparators[traceSubGrouping];
    
    // If the current attribute is an option, do not perform a comparison on it
    } else if(traceGrouping == "option")
      comp = NULL;
    
    // If a comparator was identified
    if(comp) {
      // Compare the two values and add the relation to the returned map
      attrValue rel = o->second.compare(r->second, *comp);
      //cout << "i->second="<<i->second.serialize()<<", i2->second="<<i2->second.serialize()<<" rel="<<rel.serialize()<<endl;
      //cout << "    rel="<<rel.serialize()<<endl;

      // Serialize the relationship between the two attrValues and record it in relation[]]
      relation[o->first] = rel.serialize();
    // If there is no comparator
    } else {
      // Add the raw value in ctxtobs to the returned map
      relation[o->first] = o->second.serialize();
    }
  }
  
  return relation;
}

// Given a mapping of trace attribute names to the serialized representations of their attrValues, returns
// the same mapping but with the attrValues deserialized as attrValues
std::map<std::string, attrValue> compHierGraph::deserializeObs(const std::map<std::string, std::string>& obs) {
  // Store the observation in comparisonObs, converting the observed values from strings to attrValues
  map<string, attrValue> valObs;
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    valObs[o->first] = attrValue(o->second, attrValue::unknownT);
    //cout << "    deserializeObs() "<<o->first<<" => "<<valObs[o->first].serialize()<<endl;
  }
  return valObs;
}

// Given a map, returns the same map with the given key removed
map<string, string> HG_remKey(const map<string, string>& m, std::string key) {
  map<string, string> m2 = m;
  m2.erase(key);
  return m2;
}


// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void compHierGraph::observe(int traceID,
                         const map<string, string>& ctxt, 
                         const map<string, string>& obs,
                         const map<string, anchor>& obsAnchor) {
  /*cout << "compHierGraph::observe("<<traceID<<")"<<endl;
  cout << "    ctxt=";
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) { cout << c->first << "=>"<<c->second<<" "; }
  cout << endl;
  cout << "    obs=";
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) { cout << o->first << "=>"<<o->second<<" "; }
  cout << endl;*/
  
  // Compute the portion of the context that identifies the inputs to the hierGraphs for which no compContext was 
  // specified. For identical values of these inputs we will consider different values of options and different 
  // relations between the reference and non-reference outputs and inputs for which a comparator was provided.
  map<string, string> inputCtxt;
  
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    string hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName;
    decodeCtxtName(c->first, hierGraphClass, ctxtGrouping, ctxtSubGrouping, attrName);
    
    /*cout << "        c->first="<<c->first<<", hierGraphClass="<<hierGraphClass<<", ctxtGrouping="<<ctxtGrouping<<", ctxtSubGrouping="<<ctxtSubGrouping<<", attrName="<<attrName<<endl;
    cout << "        inComparators="<<endl;
    int inIdx=0;
    for(vector<map<string, comparator*> >::iterator c=inComparators.begin(); c!=inComparators.end(); c++, inIdx++) {
      cout << "            input "<<inIdx<<": ";
      for(map<string, comparator*>::iterator i=c->begin(); i!=c->end(); i++)
        cout << i->first<<" ";
      cout << endl;
    }*/
    
    // If the current key is not a control attribute
    if(!(hierGraphClass=="compHierGraph" && ctxtGrouping=="isReference")) {
      // If the current context key is an input and no comparator was provided for it, include it in inputCtxt
      int inIdx = attrValue::parseInt(ctxtSubGrouping);
      //cout << "inIdx="<<inIdx<<", ctxtGrouping="<<ctxtGrouping<<", attrName="<<attrName<<", find="<<(inComparators[inIdx].find(attrName) == inComparators[inIdx].end())<<endl;
      if(ctxtGrouping=="input" && 
          (inComparators[inIdx].find(attrName) == inComparators[inIdx].end() ||
           inComparators[inIdx][attrName]->isNoComparator()))
        inputCtxt[c->first] = c->second;
      /*else if(inComparators[inIdx].find(attrName) != inComparators[inIdx].end())
        cout << "    inComparators[inIdx]="<<inComparators[inIdx][attrName]->str()<<" isNoComparator()="<<inComparators[inIdx][attrName]->isNoComparator()<<endl;*/
    }
  }
  
/*  cout << "    inputCtxt=";
  for(map<string, string>::const_iterator o=inputCtxt.begin(); o!=inputCtxt.end(); o++) { cout << o->first << (o->second.length()>300? "=> LARGE": string("=>")+o->second)<<" "; }
  cout << endl;
  cout << "    referenceObs[inputCtxt] (#"<<referenceObs.size()<<") : "<<(referenceObs.find(inputCtxt) != referenceObs.end())<<endl;
  cout << "    comparisonObs[inputCtxt] (#"<<comparisonObs.size()<<") : "<<(comparisonObs.find(inputCtxt) != comparisonObs.end())<<endl;*/
  
  // If this is the reference observation for the given input context
  map<string, string>::const_iterator isReferenceIter = ctxt.find("compHierGraph:isReference");
  assert(isReferenceIter != ctxt.end());
  attrValue isReference(isReferenceIter->second, attrValue::unknownT);
  //cout << "isReference.getInt()="<<isReference.getInt()<<", str="<<isReferenceIter->second<<endl;
  if(isReference.getInt()) {
    // There can only be one such observation for a given input context
    if(referenceObs.find(inputCtxt) != referenceObs.end())
    { cerr << "Multiple instances of hierGraph are declared to be reference!\nctxt=\n"<<HG_data2str(ctxt)<<"\nobs="<<HG_data2str(obs)<<endl; assert(0); }
    
    // Store the observation in referenceCtxt/referenceObs, converting the observed values from strings to attrValues
    referenceCtxt[inputCtxt] = deserializeObs(HG_remKey(ctxt, "compHierGraph:isReference"));
    referenceObs[inputCtxt]  = deserializeObs(obs);
    
    //cout << "comparisonObs.find(inputCtxt) = "<<(comparisonObs.find(inputCtxt) != comparisonObs.end())<<endl;
    // If we've observed any observations that we need to compare to the reference
    if(comparisonObs.find(inputCtxt) != comparisonObs.end()) {
      // Iterate through all the observations in comparisonObs for this input context and relate them to the 
      // observation in referenceObs for the same context
      list<map<string, attrValue> >& allObs = comparisonObs[inputCtxt];
      list<map<string, attrValue> >& allCtxt = comparisonCtxt[inputCtxt];
      assert(allObs.size() == allCtxt.size());
      map<string, attrValue>& ref = referenceObs[inputCtxt];
      
      // Iterate over the contexts and observed trace values of all the prior observations for this input context
      list<map<string, attrValue> >::iterator o=allObs.begin();
      list<map<string, attrValue> >::iterator c=allCtxt.begin();
      //cout << "    #alObs="<<allObs.size()<<endl;
      for(; o!=allObs.end(); o++, c++) {
        //cout << "#c="<<c->size()<<" #referenceCtxt[inputCtxt]="<<referenceCtxt[inputCtxt].size()<<endl;
        //cout << "#o="<<o->size()<<" #referenceObs[inputCtxt]="<<referenceObs[inputCtxt].size()<<endl;
        //assert(c->size() <= referenceCtxt[inputCtxt].size());
        assert(o->size() <= referenceObs[inputCtxt].size());
        
        // Compare each value in the current observation in comparisonObs and comparisonCtxt to the corresponding
        // value in referenceObs and add the result to relation
        map<string, string> ctxtRelation = compareObservations(*c, referenceCtxt[inputCtxt]);
        map<string, string> obsRelation  = compareObservations(*o, referenceObs[inputCtxt]);
                
        // Call the observe method of the parent class 
        emitObservation(traceID, ctxtRelation, obsRelation, map<string, anchor>());
      }
    }
  // If this is a non-reference observation
  } else {
    // If we've already observed the reference observation for the current input context
    if(referenceObs.find(inputCtxt) != referenceObs.end()) {
      map<string, string> ctxtRelation = compareObservations(deserializeObs(HG_remKey(ctxt, "compHierGraph:isReference")), referenceCtxt[inputCtxt]);
      map<string, string> obsRelation  = compareObservations(deserializeObs(obs), referenceObs[inputCtxt]);
      
      // Compare this observation to the reference and emit the result to the observe method of the parent class
      emitObservation(traceID, ctxtRelation, obsRelation, obsAnchor);
    // If we have not yet observed the reference, record this non-reference observation in comparisonObs
    } else {
      // Store the observation in comparisonObs, converting the observed values from strings to attrValues
      comparisonObs[inputCtxt].push_back(deserializeObs(obs));
      comparisonCtxt[inputCtxt].push_back(deserializeObs(HG_remKey(ctxt, "compHierGraph:isReference")));
    }
  }
}

/**************************************
 ***** processedHierGraphTraceStream *****
 **************************************/

// The directory that is used for storing intermediate files
std::string processedHierGraphTraceStream::workDir;
// The maximum unique ID assigned to any file that was used as input to a processor
int processedHierGraphTraceStream::maxFileID;

processedHierGraphTraceStream::processedHierGraphTraceStream(properties::iterator props, traceObserver* observer) :
  hierGraphTraceStream(props.next(), this)
{
  // Initialize the directories this processedTraceStream will use for its temporary storage
  static bool initialized=false;
  if(!initialized) {
    // Create the directory that holds the trace-specific scripts
    std::pair<std::string, std::string> dirs = dbg.createWidgetDir("hierGraphProcTS");
    workDir = dirs.first;
    maxFileID = 0;
  }
  
  // If no observer is specified, register a filtering queue containing a processHierGraph, followed by the the current instance of 
  // hierGraphApp to listen in on observations recorded by this traceStream.
  if(observer==NULL) {
    filterQueue = new traceObserverQueue();
  
    //cout << "<<< processedHierGraphTraceStream::processedHierGraphTraceStream"<<endl;
    // Add this trace object as a change listener to all the context variables
    long numCmds = properties::getInt(props, "numCmds");
    for(long i=0; i<numCmds; i++) {
      commandProcessors.push_back(new externalTraceProcessor_File(props.get(txt()<<"cmd"<<i), txt()<<workDir<<"/in"<<(maxFileID++)));
      filterQueue->push_back(commandProcessors.back());
    }

    // Create an instance of hierGraph to build the polynomial to fit of the data that comes out of the 
    // final command
    mFilter = new hierGraph(hierGraphID);
    hierGraphApp::registerHierGraph(hierGraphID, mFilter, NULL);
    
    filterQueue->push_back(mFilter);

    // The final observer in the queue is the original traceStream, which accepts the observations and sends
    // them to be visualized
    filterQueue->push_back(this);

    // Route all of this traceStream's observations through queue
    registerObserver(filterQueue);
    //cout << ">>> processedHierGraphTraceStream::processedHierGraphTraceStream"<<endl;
  }/* else {
    m = NULL;
    queue = NULL;
  }*/
  //cout << "cmFilter="<<cmFilter<<", hierGraphApp::getInstance()="<<hierGraphApp::getInstance()<<", queue="<<queue<<endl;
}

processedHierGraphTraceStream::~processedHierGraphTraceStream() {
  if(filterQueue)   { delete filterQueue;   filterQueue=NULL; }
  
  //if(mFilter) { delete mFilter; mFilter=NULL; }
  // Deallocate all the command processor objects
  for(list<externalTraceProcessor_File*>::iterator cp=commandProcessors.begin(); cp!=commandProcessors.end(); cp++)
    delete *cp;
}

// Called when we observe the entry tag of a processedHierGraphTraceStream
void *processedHierGraphTraceStream::enterTraceStream(properties::iterator props) {
  //cout << "hierGraphApp::enterTraceStream"<<endl;
  assert(props.name() == "processedHierGraphTS");
  //string hierGraphName = properties::get(nameProps, "HierGraphName");
  
  //cout << "processedHierGraphTraceStream::enterTraceStream() props="<<props.str()<<endl;
  // Allocate a new processedHierGraphTraceStream. The constructor takes care of registering it with the currently active hierGraph
  new processedHierGraphTraceStream(props);
  
  return NULL;
}


}; // namespace layout
}; // namespace sight
