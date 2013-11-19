// Licence information included in file LICENCE
#include "../sight_layout.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "module_layout.h"

using namespace std;

namespace sight {
namespace layout {

// Escapes the whitespace in a node's name
std::string escapeNodeWhitespace(std::string s)
{
  string out;
  for(unsigned int i=0; i<s.length(); i++) {
    if(s[i]==' ' || s[i]=='\t' || s[i]=='\n' || s[i]=='\r') {
    } else
    	out += s[i];
  }
  return out;
}

// Record the layout handlers in this file
void* moduleEnterHandler(properties::iterator props) { return new module(props); }
void  moduleExitHandler(void* obj) { module* m = static_cast<module*>(obj); delete m; }
  
moduleLayoutHandlerInstantiator::moduleLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["module"]      = &moduleEnterHandler;
  (*layoutExitHandlers )["module"]      = &moduleExitHandler;
  (*layoutEnterHandlers)["moduleTrace"] = &module::registerTraceID;
  (*layoutExitHandlers )["moduleTrace"] = &defaultExitHandler;
  (*layoutEnterHandlers)["moduleNode"]  = &module::addNode;
  (*layoutExitHandlers )["moduleNode"]  = &defaultExitHandler;
  (*layoutEnterHandlers)["moduleEdge"]  = &module::addEdge;
  (*layoutExitHandlers )["moduleEdge"]  = &defaultExitHandler;
}
moduleLayoutHandlerInstantiator moduleLayoutHandlerInstance;

// The path the directory where output files of the graph widget are stored
// Relative to current path
std::string module::outDir="";
// Relative to root of HTML document
std::string module::htmlOutDir="";

// Stack of the modules that are currently in scope
std::list<module*> module::mStack;

module::module(properties::iterator props) : block(properties::next(props)) {
  dbg.enterBlock(this, false, true);
  initEnvironment();  
    
  moduleID = properties::getInt(props, "moduleID");
  
  dbg.ownerAccessing();
  dbg << "<div id=\"module_container_"<<moduleID<<"\"></div>\n";
  dbg.userAccessing();
  
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << moduleID << ".dot";
  dotFile.open(origDotFName.str().c_str());
  dotFile << "digraph G {"<<endl;
  
  // Add the current graph to the stack
  mStack.push_back(this);
}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void module::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  pair<string, string> paths = dbg.createWidgetDir("module");
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
}

module::~module() {
  dotFile << "}"<<endl;
  dotFile.close();
  
  dbg.exitBlock();

  // Lay out the dot graph   
  //  #ifdef DOT_PATH
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << moduleID << ".dot";
  ostringstream placedDotFName; placedDotFName << outDir << "/placed." << moduleID << ".dot";

  // Create the SVG file's picture of the dot file
  //ostringstream cmd; cmd << DOT_PATH << "dot -Tsvg -o"<<imgPath<<" "<<dotFName.str() << "-Tcmapx -o"<<mapFName.str()<<"&"; 
  // Create the explicit DOT file that details the graph's layout
  //ostringstream cmd; cmd << DOT_PATH << "dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  ostringstream cmd; cmd << ROOT_PATH << "/widgets/graphviz/bin/dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());
  
  dbg.widgetScriptCommand(txt() << 
     "  var canviz_"<<moduleID<<";\n" <<
     "  canviz_"<<moduleID<<" = new Canviz('module_container_"<<moduleID<<"');\n" <<
     //dbg << "  canviz_"<<graphID<<".setImagePath('graphs/images/');\n";
     "  canviz_"<<moduleID<<".setScale(1);\n" <<
     "  canviz_"<<moduleID<<".load('"<<htmlOutDir<<"/placed." << moduleID << ".dot');\n"); 

/*  #else
  dbg << "<b>graphviz not available</b>" << endl;
  #endif*/

  
  // Remove the current graph from the stack
  assert(mStack.size()>0);
  assert(mStack.back()==this);
  mStack.pop_back();
}

string portName(const common::module::context& node, common::module::ioT type, int index) 
{ return txt()<<"\"port_"<<escapeNodeWhitespace(node.UID())<<"_"<<(type==common::module::input?"In":"Out")<<"_"<<index<<"\""; }
  
string portNamePrefix(const common::module::context& node) 
{ return txt()<<"port_"<<escapeNodeWhitespace(node.UID()); }

string portNameSuffix(common::module::ioT type, int index) 
{ return txt()<<(type==common::module::input?"In":"Out")<<index; }



// Registers the ID of the trace that is associated with the current module
void module::registerTraceID(int traceID) {
  // [{ctxt:{key0:..., val0:..., key1:..., val1:..., ...}, div:divID}, ...]
  std::list<std::pair<std::list<std::pair<std::string, std::string> >, std::string> > modules;
  
  for(std::set<std::string>::iterator m=moduleNames.begin(); m!=moduleNames.end(); m++) {
    std::list<std::pair<std::string, std::string> moduleName;
    moduleName.push_back(make_pair("Name", *m))
    modules.push_back(make_pair(moduleName, "div_"+*m));
  }
  
  trace::getTrace(traceID)->setSplitCtxtHostDivs(modules);
}

void* module::registerTraceID(properties::iterator props) {
  assert(mStack.size()>0);
  mStack.back()->registerTraceID(properties::getInt(props, "traceID")); 
  return NULL;
}

// Add a a module node
void module::addNode(/*context node, */string node, int numInputs, int numOutputs, int ID, int count/*, const set<string>& nodeContexts*/) {
  //knownCtxt[ID] = node;
  //knownCtxt[ID] = nodeContexts;
  moduleNames.insert(node);
  
  /*dotFile << "subgraph cluster"<<ID<<" {"<<endl;
  dotFile << "\tstyle=filled;"<<endl;
  dotFile << "\tcolor=lightgrey;"<<endl;*/
  //dotFile << "\tlabel=\""<<node.str()<<"\";"<<endl;
  
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
  //dotFile << "\t\""<<portNamePrefix(node)<<"\" [shape=none, fill=lightgrey, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  dotFile << "\tnode"<<ID<<" [shape=none, fill=lightgrey, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  
  // Input ports
  if(numInputs>0) {
    dotFile << "\t\t<TR><TD><TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\"><TR>";
    for(int i=0; i<numInputs; i++) dotFile << "<TD PORT=\""<<portNameSuffix(input, i)<<"\">In "<<i<<"</TD>";
    dotFile << "</TR></TABLE></TD></TR>"<<endl;
  }
  
  // Node Info
  dotFile << "\t\t<TR><TD";
  //if(numInputs + numOutputs > 0) dotFile << " COLSPAN=\""<<(numInputs>numOutputs? numInputs: numOutputs)<<"\"";
  dotFile << ">"<<node/*.str()*/<<"</TD></TR>"<<endl;
  
  dotFile << "\t\t<TR><TD><TABLE><TR><TD BGCOLOR=\"#FF00FF\" COLOR=\"#FF00FF\" WIDTH=\"200\" HEIGHT=\"100\">Box</TD></TR></TABLE></TD></TR>"<<endl;
  // <IMG SRC=\"img/attrAnd.gif\" SCALE=\"WIDTH\"/>
  
  // Output ports
  if(numOutputs>0) {
    dotFile << "\t\t<TR><TD><TABLE CELLBORDER=\"1\" CELLSPACING=\"0\"><TR>";
    for(int o=0; o<numOutputs; o++) dotFile << "<TD PORT=\""<<portNameSuffix(output, o)<<"\">Out "<<o<<"</TD>";
    dotFile << "</TR></TABLE></TD></TR>"<<endl;
  }  
  dotFile << "</TABLE>>];" <<endl;
  
  //dotFile << "}"<<endl;
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* module::addNode(properties::iterator props) {
  /*set<string> nodeContexts;
  int numContexts = properties::getInt(props, "numContexts");
  for(int i=0; i<numContexts; i++)
    nodeContexts.insert(properties::get(props, txt()<<"context_"<<i));*/
  
  assert(mStack.size()>0);
  //mStack.back()->addNode(context(props), properties::getInt(props, "ID"), properties::getInt(props, "count")); 
  mStack.back()->addNode(properties::get(props, "name"), 
                         properties::getInt(props, "numInputs"), properties::getInt(props, "numOutputs"), 
                         properties::getInt(props, "ID"), properties::getInt(props, "count")/*, 
                         nodeContexts*/); 
  return NULL;
}


// Add a directed edge from one port to another
void module::addEdge(int fromCID, common::module::ioT fromT, int fromP, 
                     int toCID,   common::module::ioT toT,   int toP) {
  /*dotFile << "\t\""<<portNamePrefix(knownCtxt[fromCID].UID())<<"\":"<<portNameSuffix(fromT, fromP)<<""<<
             " -> "<<
               "\""<<portNamePrefix(knownCtxt[toCID].UID())<<"\":"<<portNameSuffix(toT,   toP  )<<";\n";*/
  dotFile << "\tnode"<<fromCID<<":"<<portNameSuffix(fromT, fromP)<<""<<
             " -> "<<
               "node"<<toCID  <<":"<<portNameSuffix(toT,   toP  )<<";\n";
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* module::addEdge(properties::iterator props) {
  mStack.back()->addEdge(properties::getInt(props, "fromCID"), (ioT)properties::getInt(props, "fromT"), properties::getInt(props, "fromP"),
                         properties::getInt(props, "toCID"),   (ioT)properties::getInt(props, "toT"),   properties::getInt(props, "toP")); 
  return NULL;
}

}; // namespace layout
}; // namespace sight
