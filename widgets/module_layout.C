// Licence information included in file LICENCE
#define MODULE_LAYOUT_C
#include "../sight_layout.h"
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

#include "module_layout.h"

using namespace std;

namespace sight {
namespace layout {

// Escapes the whitespace in a node's name
string escapeNodeWhitespace(string s)
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
  (*layoutEnterHandlers)["module"]       = &moduleEnterHandler;
  (*layoutExitHandlers )["module"]       = &moduleExitHandler;
  (*layoutEnterHandlers)["moduleNodeTS"] = &module::enterTraceStream;
  (*layoutExitHandlers )["moduleNodeTS"] = &defaultExitHandler;
  (*layoutEnterHandlers)["moduleNode"]   = &module::addNode;
  (*layoutExitHandlers )["moduleNode"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["moduleEdge"]   = &module::addEdge;
  (*layoutExitHandlers )["moduleEdge"]   = &defaultExitHandler;
}
moduleLayoutHandlerInstantiator moduleLayoutHandlerInstance;

// The path the directory where output files of the graph widget are stored
// Relative to current path
string module::outDir="";
// Relative to root of HTML document
string module::htmlOutDir="";

// Stack of the modules that are currently in scope
list<module*> module::mStack;

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
  
  dbg.includeFile("module.js"); dbg.includeWidgetScript("module.js", "text/javascript"); 
}

module::~module() {
  dotFile << "}"<<endl;
  dotFile.close();
  
  dbg.exitBlock();

  // Lay out the dot graph   
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << moduleID << ".dot";
  ostringstream placedDotFName; placedDotFName << outDir << "/placed." << moduleID << ".dot";

  // Create the explicit DOT file that details the graph's layout
  ostringstream cmd; cmd << ROOT_PATH << "/widgets/graphviz/bin/dot "<<origDotFName.str()<<" -Txdot -o"<<placedDotFName.str()<<"&"; 
  cout << "Command \""<<cmd.str()<<"\"\n";
  system(cmd.str().c_str());
  
  dbg.widgetScriptCommand(txt() << 
     "  var canviz_"<<moduleID<<";\n" <<
     "  canviz_"<<moduleID<<" = new Canviz('module_container_"<<moduleID<<"');\n" <<
     "  canviz_"<<moduleID<<".setScale(1);\n" <<
     "  canviz_"<<moduleID<<".load('"<<htmlOutDir<<"/placed." << moduleID << ".dot');\n"); 
  
  // Delete all the traceStreams associated with the given trace, which emits their output
  for(map<int, traceStream*>::iterator m=moduleTraces.begin(); m!=moduleTraces.end(); m++)
    delete m->second;
  
  // Remove the current graph from the stack
  assert(mStack.size()>0);
  assert(mStack.back()==this);
  mStack.pop_back();
}

void *module::enterTraceStream(properties::iterator props) {
  assert(properties::name(props) == "moduleNodeTS");
  //string moduleName = properties::get(nameProps, "ModuleName");
  
  // Allocate a new moduleNodeTraceStream. The constructor takes care of registering it with the currently active module
  new moduleNodeTraceStream(props);
  
  return NULL;
}

moduleNodeTraceStream::moduleNodeTraceStream(properties::iterator props) : 
  traceStream(properties::next(props), txt()<<"CanvizBox_node"<<properties::getInt(props, "nodeID"), false)
{
  assert(properties::name(props) == "moduleNodeTS");
    
  int nodeID = properties::getInt(props, "nodeID");
  
  // Get the currently active module that this traceStream belongs to
  assert(module::mStack.size()>0);
  module* m = module::mStack.back();
  assert(m->moduleTraces.find(nodeID) == m->moduleTraces.end());
  
  // Create a new traceStream object to collect the observations for this module node
  m->moduleTraces[nodeID] = this;
    
  // Register this module to listen in on observations recorded by this traceStream
  m->moduleTraces[nodeID]->registerObserver(m);
  
  // Record the mapping between traceStream IDs and the IDs of the nodes they're associated with
  m->trace2nodeID[m->moduleTraces[nodeID]->getID()] = nodeID;
  ///cout << "nodeID="<<nodeID<<", traceID="<<m->moduleTraces[nodeID]->getID()<<endl;
}

string portName(common::module::ioT type, int index) 
{ return txt()<<(type==common::module::input?"In":"Out")<<index; }


// Iterates over all combinations of keys in numericCtxt upto maxDegree in size and computes the products of
// their values. Adds each such product to the given row of polyfitCtxt at different columns.
void addPolyTerms(const map<string, double>& numericCtxt, int termCnt, int maxDegree, 
                  int row, int& col, double product, gsl_matrix *polyfitCtxt) {
  // If product contains maxDegree terms, add it to the given column of polyfitObs and increment the column counter
  if(termCnt==maxDegree) {
    gsl_matrix_set(polyfitCtxt, row, col, product);
    col++;
  } else {
    // Iterate through all the possible choices of the next factor in the current polynomial term
    for(map<string, double>::const_iterator c=numericCtxt.begin(); c!=numericCtxt.end(); c++) {
      addPolyTerms(numericCtxt, termCnt+1, maxDegree, row, col, product * c->second, polyfitCtxt);
    }
  }
}


// Iterates over all combinations of keys in numericCtxt upto maxDegree in size. Given ctxtNames, which maintains the list
// of names for each attribute and selTerms, which identifies the attribute combinations that should be selected, matches
// the selections to the names and inserts the matched pairs to selCtxt. 
// termCnt - counts the number of terms that have been added to the current polynomial product
// maxDegree - the maximum such terms that can go into a single product
// idx - The current count of the terms from the overall space of terms. May not be 0 at the root recursive calls since
//       we may iterate over multiple choices of maxDegree
// subName - the name string that has been computed so far for the current prefix of terms in the polynomial product
void selTerms2Names(const list<string>& ctxtNames, const set<int>& selTerms, 
                    list<pair<int, string> >& selCtxt, 
                    int termCnt, int maxDegree, int& idx, map<string, int>& subNames) {
  // If product contains maxDegree terms, add it to the given column of polyfitObs and increment the column counter
  if(termCnt==maxDegree) {
    // If the current product index exists in selTerms
    if(selTerms.find(idx) != selTerms.end()) {
      // Match it to its name and add it to selCtx.
      //selCtxt.push_back(make_pair(idx, subName));
      ostringstream name;
      for(map<string, int>::iterator n=subNames.begin(); n!=subNames.end(); n++) {
        assert(n->second>0);
        if(n!=subNames.begin()) name << "*";
        name << n->first;
        if(n->second>1) name << "^"<<n->second;
      }
      selCtxt.push_back(make_pair(idx, name.str()));
    }
    idx++;
  } else {
    // Iterate through all the possible choices of the next factor in the current polynomial term
    for(list<string>::const_iterator c=ctxtNames.begin(); c!=ctxtNames.end(); c++) {
      if(subNames.find(*c) == subNames.end()) subNames[*c] = 1;
      else                                    subNames[*c]++;
      
      selTerms2Names(ctxtNames, selTerms, selCtxt, termCnt+1, maxDegree, idx, subNames/*subName+(termCnt>0?"*":"")+*c*/);
      
      subNames[*c]--;
      if(subNames[*c]==0) subNames.erase(*c);
    }
  }
}

// Do a multi-variate polynomial fit of the data observed for the given nodeID and return for each trace attribute 
// a string that describes the function that best fits its values
std::vector<std::string> module::polyFit(int nodeID)
{
  vector<string> polynomials;
  
  // Do nothing if we had no observations with numeric context attributes
  if(polyfitCtxt.find(nodeID) == polyfitCtxt.end()) return polynomials;
    
  assert(numObs.find(nodeID)      != numObs.end());
  assert(polyfitObs.find(nodeID)  != polyfitObs.end());
  //assert(polyfitObs[nodeID].size()>0);
  
  // Fit a polynomial to the observed data
  int maxDegree = 2;
  long numTerms = polyfitCtxt[nodeID]->size2;
  //cout << "polyFit() numObs["<<nodeID<<"]="<<numObs[nodeID]<<", numTerms="<<numTerms<<endl; 
  // Covariance matrix
  gsl_matrix *polyfitCov = gsl_matrix_alloc(numTerms, numTerms);

  //for(int i=0; i<polyfitObs[nodeID].size(); i++) {
  for(int i=0; i<traceAttrNames[nodeID].size(); i++) {
    gsl_vector* polyfitCoeff = gsl_vector_alloc(numTerms);
    gsl_multifit_linear_workspace *polyfitWS = gsl_multifit_linear_alloc(numObs[nodeID], numTerms);
    double chisq;
    gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitCtxt[nodeID], 0, 0, numObs[nodeID], numTerms);
    //gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitObs[nodeID][i], 0, 0, numObs[nodeID], numTerms);
    //gsl_vector_view obsView = gsl_vector_subvector(polyfitObs[nodeID][i], 0, numObs[nodeID]);
    //gsl_vector_view obsView = gsl_matrix_subcolumn(polyfitObs[nodeID][i], i, 0, numObs[nodeID]);
    gsl_vector_view obsView1 = gsl_matrix_column(polyfitObs[nodeID], i);
    gsl_vector_view obsView = gsl_vector_subvector(&(obsView1.vector), 0, numObs[nodeID]);
    
    /*for(int j=0; j<numObs[nodeID]; j++) {
      for(int k=0; k<numTerms; k++)
        cout << " "<<gsl_matrix_get(&(ctxtView.matrix),j,k);
      cout << " => "<<gsl_vector_get(&(obsView.vector), j)<<endl;
    }*/
    
    
    // If we have enough observations to fit a model
    if(numTerms <= numObs[nodeID]) {
      // Use a linear solver to fit the coefficients of the polynomial
      // ******
      gsl_multifit_linear(&(ctxtView.matrix), &(obsView.vector), polyfitCoeff, polyfitCov, &chisq, polyfitWS);
      // ******
      
      /*cout << "polyfitCoeff=";
      for(int t=0; t<numTerms; t++) cout << gsl_vector_get(polyfitCoeff, t)<<" ";
      cout << endl;*/
      
      // Identify the pair of coefficients in the sorted order with the largest relative difference between them
      map<double, int> sortedCoeff;
      map<int, double> sortedCoeffInv;
      for(int t=0; t<numTerms; t++) {
        sortedCoeff[fabs(gsl_vector_get(polyfitCoeff, t))] = t;
        sortedCoeffInv[t] = fabs(gsl_vector_get(polyfitCoeff, t));
      }
      map<double, int>::reverse_iterator splitIter = sortedCoeff.rend();
      double largestCoeff = sortedCoeff.rbegin()->first;
      
      // Iterate sortedCoeff from the largest to smallest coefficient, looking for the largest gap between adjacent coefficients
      double maxDiff = 0;
      map<double, int>::reverse_iterator c=sortedCoeff.rbegin();
      map<double, int>::reverse_iterator next = c; next++;
      while(next!=sortedCoeff.rend() && next->first!=0) {
        //cout << c->first<<" / "<<next->first<<endl;
        double diff = c->first / next->first;
        if(diff>maxDiff) {
          maxDiff = diff;
          splitIter = next;
        }
        c++;
        next = c; next++;
      }
      
      // Create a set of just the indexes of the large coefficients, which are the coefficients larger than the largest 
      // coefficient drop and are not irrelevantly small compared to the largest coefficient
      set<int> coeffIdxes;
      //cout << "sortedCoeff: ";
      for(map<double, int>::reverse_iterator c=sortedCoeff.rbegin(); c!=splitIter && c->first>=largestCoeff*1e-3; c++) {
        //cout << c->second<<"/"<<c->first<<" ";
        coeffIdxes.insert(c->second);
      }
      //cout << endl;
  
      list<pair<int, string> > selCtxt;
      
      // Add the constant term, if it is in coeffIdxes
      if(coeffIdxes.find(0) != coeffIdxes.end()) selCtxt.push_back(make_pair(0,""));
        
      int idx=1;
      for(int degree=1; degree<=maxDegree; degree++) {
        map<string, int> subNames;
        selTerms2Names(numericCtxtNames[nodeID], coeffIdxes, selCtxt, 0, degree, idx, subNames/*""*/);
      }
      
      // Sort the selected names according to the size of their coefficients
      map<double, pair<int, string> > selCtxtSorted;
      for(list<pair<int, string> >::iterator c=selCtxt.begin(); c!=selCtxt.end(); c++)
        selCtxtSorted[sortedCoeffInv[c->first]] = *c;
      
      //cout << "selCtxtSorted="<<selCtxtSorted.size()<<" #selCtxt="<<selCtxt.size()<<endl;
      
      // Serialize the polynomial fit terms in the order from largest to smallest coefficient
      ostringstream polyStr;
      for(map<double, pair<int, string> >::reverse_iterator c=selCtxtSorted.rbegin(); c!=selCtxtSorted.rend(); c++) {
        if(c!=selCtxtSorted.rbegin()) polyStr << " + ";
        polyStr << setiosflags(ios::scientific) << setprecision(2) << 
                   gsl_vector_get(polyfitCoeff, c->second.first) << 
                   (c->second.second==""?"":"*") << 
                   c->second.second;
      }
      polynomials.push_back(polyStr.str());
    // If we didn't get enough observations to train a model
    } else
      polynomials.push_back("");

    //gsl_vector_free(polyfitObs[nodeID][i]);
    gsl_vector_free(polyfitCoeff);
  }
  
  gsl_matrix_free(polyfitCov);
  gsl_matrix_free(polyfitCtxt[nodeID]);
  gsl_matrix_free(polyfitObs[nodeID]);
  
  polyfitCtxt.erase(nodeID);
  polyfitObs.erase(nodeID);
  numObs.erase(nodeID);
  
  return polynomials;
}

// Given the name of a trace attribute, the string representation of its polynomial fit and a line width 
// emits to dotFile HTML where line breaks are inserted at approximately every lineWidth characters.
void printPolyFitStr(ostream& dotFile, std::string traceName, std::string polyFit, unsigned int lineWidth) {
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

// Add a a module node
void module::addNode(string node, int numInputs, int numOutputs, int ID, int count/*, const set<string>& nodeContexts*/) {
  moduleNames.insert(node);
  //cout << "addNode() nodeID="<<ID<<", name="<<node<<endl;
  
  static int maxButtonID=0; // The maximum ID that has ever been assigned to a button
  
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

  // In the construction below we wish to allow users to interact with the trace information displayed in the main ID
  // box by clicking on the names of context attributes. graphviz/Canviz allow us to create a single onclick method for
  // the entire node but not separate ones for each TD. As such, Canviz has been extended to treat this special case
  // (javascript handlers + HTML nodes) specially, where each piece of text in such a node must be formated as 
  // "ID:text" where ID is the ID that uniquely identifies the piece of text and text is the actual text that should 
  // be displayed. ID must not contain ":" but text may. Then, the javascript handlers of the node can assume the existence 
  // of a special ID variable that uniquely identifies the particular piece of text that got clicked. If an ID of a given
  // piece of text is empty, then no link is placed around it. If some piece of text is mis-formatted (if missing the ":", 
  // Canviz provides an alert). 
  
  //dotFile << "\t\""<<portNamePrefix(node)<<"\" [shape=none, fill=lightgrey, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  dotFile << "\tnode"<<ID<<" [shape=none, fill=lightgrey, href=\"#\", onclick=\"return ClickOnModuleNode('node"<<ID<<"', this, ID);\", label=";
  
  if(numInputs==0 && numOutputs==0) {
    dotFile << "\"\"";
  } else {
    dotFile << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  
    // Input ports
    if(numInputs>0) {
      dotFile << "\t\t<TR><TD><TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
      dotFile << "\t\t\t<TR>";
      for(int i=0; i<numInputs; i++) 
        dotFile << "<TD PORT=\""<<portName(input, i)<<"\" "<<
                       "COLSPAN=\""<<(ctxtNames[ID].find(i)!=ctxtNames[ID].end()? ctxtNames[ID][i].size(): 1)<<"\">"<<
                        "<FONT POINT-SIZE=\"14\">:In "<<i<<"</FONT>"<<
                   "</TD>";
      dotFile << "</TR>"<<endl;
      
      // The names of the context attributes for each output
      dotFile << "\t\t\t<TR>";
      for(int i=0; i<numInputs; i++) {
        if(ctxtNames[ID].find(i)!=ctxtNames[ID].end()) {
          for(list<string>::iterator c=ctxtNames[ID][i].begin(); c!=ctxtNames[ID][i].end(); c++) {
            int buttonID = maxButtonID; maxButtonID++;
            dotFile << dotFile << "<TD BGCOLOR=\"#B0CDFF\"><FONT POINT-SIZE=\"14\">"<<buttonID<<":"<<*c<<"</FONT></TD>";
          
            // Register the command to be executed when this button is clicked
            ostringstream cmd; 
            cmd << "registerModuleButtonCmd("<<buttonID<<", \""<<
                                 moduleTraces[ID]->getDisplayJSCmd(traceStream::attrNames(txt()<<i<<":"<<*c), traceStream::attrNames())<<
                               "\");"<<endl;
            dbg.widgetScriptCommand(cmd.str());
          }
        } else
          dotFile << dotFile << "<TD></TD>";
      }
      dotFile << "\t\t\t</TR>"<<endl;
      dotFile << "</TABLE></TD></TR>"<<endl;
    }
  
    // Node Info
    dotFile << "\t\t<TR><TD";
    //if(numInputs + numOutputs > 0) dotFile << " COLSPAN=\""<<(numInputs>numOutputs? numInputs: numOutputs)<<"\"";
    dotFile << "><FONT POINT-SIZE=\"26\">:"<<node/*.str()*/<<"</FONT></TD></TR>"<<endl;
  
    // If we observed values during the execution of this module group  
    if(traceAttrNames[ID].size()>0) {
      // Polynomial fit of the observations
      vector<string> polynomials = polyFit(ID);
      assert(polynomials.size() == traceAttrNames[ID].size());
      for(int i=0; i<polynomials.size(); i++) {
        // If we were able to train a model for the current trace attribute, emit it
        if(polynomials[i] != "") printPolyFitStr(dotFile, traceAttrNames[ID][i], polynomials[i], 80);
      }
        //dotFile << "\t\t<TR><TD>:"<<traceAttrNames[ID][i]<< ": "<<wrapStr(polynomials[i], 50)<<"</TD></TR>"<<endl;
    }
    
    if(traceAttrNames[ID].size()>0) {
      dotFile << "\t\t<TR><TD><TABLE><TR><TD BGCOLOR=\"#FF00FF\" COLOR=\"#FF00FF\" WIDTH=\"300\" HEIGHT=\"200\"></TD></TR></TABLE></TD></TR>"<<endl;
    }
    
    // Output ports
    if(numOutputs>0) {
      dotFile << "\t\t<TR><TD><TABLE CELLBORDER=\"1\" CELLSPACING=\"0\"><TR>";
      for(int o=0; o<numOutputs; o++) 
        dotFile << "<TD PORT=\""<<portName(output, o)<<"\"><FONT POINT-SIZE=\"14\">:Out "<<o<<"</FONT></TD>";
      dotFile << "</TR></TABLE></TD></TR>"<<endl;
    }  
    dotFile << "</TABLE>>";
  }
  dotFile << "];" <<endl;
  
  //dotFile << "}"<<endl;
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* module::addNode(properties::iterator props) {
  assert(mStack.size()>0);
  mStack.back()->addNode(properties::get(props, "name"), 
                         properties::getInt(props, "numInputs"), properties::getInt(props, "numOutputs"), 
                         properties::getInt(props, "nodeID"), properties::getInt(props, "count")); 
  return NULL;
}


// Add a directed edge from one port to another
void module::addEdge(int fromCID, common::module::ioT fromT, int fromP, 
                     int toCID,   common::module::ioT toT,   int toP) {
  dotFile << "\tnode"<<fromCID<<":"<<portName(fromT, fromP)<<""<<
             " -> "<<
               "node"<<toCID  <<":"<<portName(toT,   toP  )<<";\n";
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* module::addEdge(properties::iterator props) {
  mStack.back()->addEdge(properties::getInt(props, "fromCID"), (ioT)properties::getInt(props, "fromT"), properties::getInt(props, "fromP"),
                         properties::getInt(props, "toCID"),   (ioT)properties::getInt(props, "toT"),   properties::getInt(props, "toP")); 
  return NULL;
}

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void module::observe(int traceID,
                     const map<string, string>& ctxt, 
                     const map<string, string>& obs,
                     const map<string, anchor>&      obsAnchor) {
  assert(trace2nodeID.find(traceID) != trace2nodeID.end());
  int nodeID = trace2nodeID[traceID];
  
  // Extract the input ports with which all the context attributes are associated and record this in 
  std::map<int, std::list<std::string> > curCtxtNames;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    // Separate the port name and raw context name
    size_t colonLoc = c->first.find(":"); assert(colonLoc != string::npos);
    int portID = strtol(c->first.substr(0, colonLoc).c_str(), NULL, 10);
    string ctxtName = c->first.substr(colonLoc+1);
    curCtxtNames[portID].push_back(ctxtName);
  }
  if(ctxtNames.find(nodeID) == ctxtNames.end()) ctxtNames[nodeID] = curCtxtNames;
  else if(ctxtNames[nodeID] != curCtxtNames)
  { cerr << "ERROR: Inconsistent context attributes in different observations for the same module node "<<nodeID<<"! Before observed "<<ctxtNames[nodeID].size()<<" numeric context attributed but this observation has "<<curCtxtNames.size()<<"."<<endl; assert(false); }
  
  // Maps the names of numeric contexts to their floating point values
  map<string, double> numericCtxt;
  list<string> curNumericCtxtNames;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    const char *valStr = c->second.c_str();
    char *nextValStr;
    double v = strtod(valStr, &nextValStr);
    // If we read a valid double value from valStr
    if(nextValStr != valStr) {
      numericCtxt[c->first] = v;
      curNumericCtxtNames.push_back(c->first);
    }
  }
  if(numericCtxtNames.find(nodeID) == numericCtxtNames.end()) numericCtxtNames[nodeID] = curNumericCtxtNames;
  else if(numericCtxtNames[nodeID] != curNumericCtxtNames)
  { cerr << "ERROR: Inconsistent numeric context attributes in different observations for the same module node "<<nodeID<<"! Before observed "<<numericCtxtNames[nodeID].size()<<" numeric context attributed but this observation has "<<curNumericCtxtNames.size()<<"."<<endl; assert(false); }
  
  // Do nothing if none of the context attributes are numeric
  if(numericCtxt.size()==0) return;
    
  // Read out the names of the observation's trace attributes
  if(traceAttrNames.find(nodeID) == traceAttrNames.end()) {
    for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++)
      traceAttrNames[nodeID].push_back(o->first);
  }
  
  int maxDegree = 2;
  // The total number of polynomial terms to maxDegree: 
  
  long numTerms = (numericCtxt.size()==1 ? maxDegree+1:
                       // #numericCtxt^0 + #numericCtxt^1 + #numericCtxt^2 + ... + #numericCtxt^maxDegree = #numericCtxt^(maxDegree+1) - 1
                       pow(numericCtxt.size(), maxDegree+1));
  
  //cout << "module::observe() nodeID="<<nodeID<<", #numericCtxt="<<numericCtxt.size()<<", numTerms="<<numTerms<<endl;
     
  // If this is the first observation we have from the given traceStream, allocate the
  // polynomial fit datastructures
  if(polyfitCtxt.find(nodeID) == polyfitCtxt.end()) {
    polyfitCtxt[nodeID] = gsl_matrix_alloc(1000, numTerms);
    
    //for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    //  polyfitObs[nodeID].push_back(gsl_vector_alloc(1000));
    //}
    polyfitObs[nodeID] = gsl_matrix_alloc(1000, obs.size());
    
    numObs[nodeID] = 0;
    numAllocObs[nodeID] = 1000;
  }
  
  // If we're out of space in polyfitCtxt[nodeID] and polyfitObs[nodeID] to store another observation, grow them
  if(numObs[nodeID] == numAllocObs[nodeID]) {
    gsl_matrix* newCtxt = gsl_matrix_alloc(numAllocObs[nodeID]*2, numTerms);
    gsl_matrix_view newCtxtView = gsl_matrix_submatrix (newCtxt, 0, 0, numObs[nodeID], numTerms);
    gsl_matrix_memcpy (&(newCtxtView.matrix), polyfitCtxt[nodeID]);
    gsl_matrix_free(polyfitCtxt[nodeID]);
    polyfitCtxt[nodeID] = newCtxt;
    
    gsl_matrix* newObs = gsl_matrix_alloc(numAllocObs[nodeID]*2, numTerms);
    gsl_matrix_view newObsView = gsl_matrix_submatrix (newObs, 0, 0, numObs[nodeID], obs.size());
    gsl_matrix_memcpy (&(newObsView.matrix), polyfitObs[nodeID]);
    gsl_matrix_free(polyfitObs[nodeID]);
    polyfitObs[nodeID] = newObs;
  }
  
  // Add this observation to polyfitObs
  int obsIdx=0;
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++, obsIdx++) {
    // For now we assert that all observed values must be numeric
    const char *valStr = o->second.c_str();
    char *nextValStr;
    double v = strtod(valStr, &nextValStr);
    assert(nextValStr != valStr);
    
    //gsl_vector_set(polyfitObs[nodeID][obsIdx], numObs[nodeID], v);
    gsl_matrix_set(polyfitObs[nodeID], numObs[nodeID], obsIdx, v);
  }

  // Add the context of the observation to polyfitCtxt
      
  // The first entry corresponds to the degree-0 constant term
  gsl_matrix_set(polyfitCtxt[nodeID], numObs[nodeID], 0, 1);

  // Add all the polynomial terms of degrees upto and including maxDegree
  int col=1;
  for(int degree=1; degree<=maxDegree; degree++)
    addPolyTerms(numericCtxt, 0, degree, numObs[nodeID], col, 1, polyfitCtxt[nodeID]);

  // Advance the observation counter to the next row in polyfitObs and polyfitCtxt
  numObs[nodeID]++;  
}

}; // namespace layout
}; // namespace sight
