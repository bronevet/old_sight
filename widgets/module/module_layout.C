// Licence information included in file LICENCE
#define MODULE_LAYOUT_C
#include "../../sight_layout.h"
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
void* modularAppEnterHandler(properties::iterator props) { return new modularApp(props); }
void  modularAppExitHandler(void* obj) { modularApp* ma = static_cast<modularApp*>(obj); delete ma; }
  
moduleLayoutHandlerInstantiator::moduleLayoutHandlerInstantiator() { 
  (*layoutEnterHandlers)["modularApp"]   = &modularAppEnterHandler;
  (*layoutExitHandlers )["modularApp"]   = &modularAppExitHandler;
  (*layoutEnterHandlers)["moduleNodeTS"] = &modularApp::enterTraceStream;
  (*layoutExitHandlers )["moduleNodeTS"] = &defaultExitHandler;
  (*layoutEnterHandlers)["module"]       = &modularApp::enterModule;
  (*layoutExitHandlers )["module"]       = &modularApp::exitModule;
  (*layoutEnterHandlers)["moduleEdge"]   = &modularApp::addEdge;
  (*layoutExitHandlers )["moduleEdge"]   = &defaultExitHandler;
}
moduleLayoutHandlerInstantiator moduleLayoutHandlerInstance;

// Points to the currently active instance of modularApp. There can be only one.
modularApp* modularApp::activeMA=NULL;

// The path the directory where output files of the graph widget are stored
// Relative to current path
string modularApp::outDir="";
// Relative to root of HTML document
string modularApp::htmlOutDir="";

// Stack of the modules that are currently in scope
//list<module*> modularApp::mStack;

modularApp::modularApp(properties::iterator props) : block(properties::next(props)) {
  // Register this modularApp instance (there can be only one)
  assert(activeMA == NULL);
  activeMA = this;
  
  dbg.enterBlock(this, false, true);
  initEnvironment();  
    
  appName = properties::get(props, "appName");
  appID = properties::getInt(props, "appID");
  
  dbg.ownerAccessing();
  dbg << "<div id=\"module_container_"<<appID<<"\"></div>\n";
  dbg.userAccessing();
  
  ostringstream origDotFName;   origDotFName   << outDir << "/orig."   << appID << ".dot";
  dotFile.open(origDotFName.str().c_str());
  dotFile << "digraph G {"<<endl;
  
  // Add the current module to the stack
  //mStack.push_back(this);
}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void modularApp::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  // Create the directory that holds the module-specific scripts
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
  
  dbg.includeFile("module/module.js"); dbg.includeWidgetScript("module/module.js", "text/javascript"); 
}

modularApp::~modularApp() {
  // Unregister this modularApp instance (there can be only one)
  assert(activeMA);
  activeMA = NULL;
  
  dotFile << "}"<<endl;
  dotFile.close();
  
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
     "  canviz_"<<appID<<" = new Canviz('module_container_"<<appID<<"');\n" <<
     "  canviz_"<<appID<<".setScale(1);\n" <<
     "  canviz_"<<appID<<".load('"<<htmlOutDir<<"/placed." << appID << ".dot');\n"); 
  
  // Delete all the traceStreams associated with the given trace, which emits their output
  for(map<int, traceStream*>::iterator m=moduleTraces.begin(); m!=moduleTraces.end(); m++)
    delete m->second;
  
  // Remove the current module from the stack
  /*assert(mStack.size()>0);
  assert(mStack.back()==this);
  mStack.pop_back();*/
}

void *modularApp::enterTraceStream(properties::iterator props) {
  //cout << "modularApp::enterTraceStream"<<endl;
  assert(props.name() == "moduleNodeTS");
  //string moduleName = properties::get(nameProps, "ModuleName");
  
  // Allocate a new moduleNodeTraceStream. The constructor takes care of registering it with the currently active module
  new moduleNodeTraceStream(props);
  
  return NULL;
}

moduleNodeTraceStream::moduleNodeTraceStream(properties::iterator props) : 
  traceStream(properties::next(props), txt()<<"CanvizBox_node"<<properties::getInt(props, "moduleID"), false)
{
  assert(props.name() == "moduleNodeTS");
    
  int moduleID = properties::getInt(props, "moduleID");
  
  // Get the currently active module that this traceStream belongs to
  /*assert(modularApp::mStack.size()>0);
  module* m = modularApp::activeMA->mStack.back();*/
  assert(modularApp::activeMA->moduleTraces.find(moduleID) == modularApp::activeMA->moduleTraces.end());
  
  // Create a new traceStream object to collect the observations for this module group
  modularApp::activeMA->moduleTraces[moduleID] = this;
    
  // Register this module group to listen in on observations recorded by this traceStream
  modularApp::activeMA->moduleTraces[moduleID]->registerObserver(modularApp::activeMA);
  
  // Record the mapping between traceStream IDs and the IDs of the module group they're associated with
  modularApp::activeMA->trace2moduleID[getID()] = moduleID;
  ///cout << "moduleID="<<moduleID<<", traceID="<<m->moduleTraces[moduleID]->getID()<<endl;
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

// Do a multi-variate polynomial fit of the data observed for the given moduleID and return for each trace attribute 
// a string that describes the function that best fits its values
std::vector<std::string> modularApp::polyFit(int moduleID)
{
  vector<string> polynomials;
  
  // Do nothing if we had no observations with numeric context attributes
  if(polyfitCtxt.find(moduleID) == polyfitCtxt.end()) return polynomials;
    
  assert(numObs.find(moduleID)      != numObs.end());
  assert(polyfitObs.find(moduleID)  != polyfitObs.end());
  //assert(polyfitObs[moduleID].size()>0);
  
  // Fit a polynomial to the observed data
  int maxDegree = 2;
  long numTerms = polyfitCtxt[moduleID]->size2;
  //cout << "polyFit() numObs["<<moduleID<<"]="<<numObs[moduleID]<<", numTerms="<<numTerms<<endl; 
  // Covariance matrix
  gsl_matrix *polyfitCov = gsl_matrix_alloc(numTerms, numTerms);

  //for(int i=0; i<polyfitObs[moduleID].size(); i++) {
  int i=0; 
  for(set<std::string>::iterator t=traceAttrNames[moduleID].begin(); t!=traceAttrNames[moduleID].end(); t++, i++) {
    //cout << i << ":  attr "<<*t<<endl;
    
    gsl_vector* polyfitCoeff = gsl_vector_alloc(numTerms);
    gsl_multifit_linear_workspace *polyfitWS = gsl_multifit_linear_alloc(numObs[moduleID], numTerms);
    double chisq;
    gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitCtxt[moduleID], 0, 0, numObs[moduleID], numTerms);
    //gsl_matrix_view ctxtView = gsl_matrix_submatrix (polyfitObs[moduleID][i], 0, 0, numObs[moduleID], numTerms);
    //gsl_vector_view obsView = gsl_vector_subvector(polyfitObs[moduleID][i], 0, numObs[moduleID]);
    //gsl_vector_view obsView = gsl_matrix_subcolumn(polyfitObs[moduleID][i], i, 0, numObs[moduleID]);
    gsl_vector_view obsView1 = gsl_matrix_column(polyfitObs[moduleID], i);
    gsl_vector_view obsView = gsl_vector_subvector(&(obsView1.vector), 0, numObs[moduleID]);
    
    /*for(int j=0; j<numObs[moduleID]; j++) {
      for(int k=0; k<numTerms; k++)
        cout << " "<<gsl_matrix_get(&(ctxtView.matrix),j,k);
      cout << " => "<<gsl_vector_get(&(obsView.vector), j)<<endl;
    }*/
    
    
    // If we have enough observations to fit a model
    if(numTerms <= numObs[moduleID]) {
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
        selTerms2Names(numericCtxtNames[moduleID], coeffIdxes, selCtxt, 0, degree, idx, subNames/*""*/);
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

    //gsl_vector_free(polyfitObs[moduleID][i]);
    gsl_vector_free(polyfitCoeff);
  }
  
  gsl_matrix_free(polyfitCov);
  gsl_matrix_free(polyfitCtxt[moduleID]);
  gsl_matrix_free(polyfitObs[moduleID]);
  
  polyfitCtxt.erase(moduleID);
  polyfitObs.erase(moduleID);
  numObs.erase(moduleID);
  
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

// Emits to the dot file the buttons used to select the combination of input property and trace attribute
// that should be shown in the data panel of a given module node.
// numInputs/numOutputs - the number of inputs/outputs of this module node
// ID - the unique ID of this module node
// prefix - We measure both the observations of measurements during the execution of modules and the 
//    properties of module outputs. Both are included in the trace of the module but the names of measurements
//    are prefixed with "measure:" and the names of outputs are prefixed with "output:#:", where the # is the
//    index of the output. The prefix argument identifies the types of attributs we'll be making buttons for and
//    it should be either "measure" or "output".
// bgColor - The background color of the buttons
void modularApp::showButtons(int numInputs, int numOutputs, int ID, std::string prefix, std::string bgColor) {
  static int maxButtonID=0; // The maximum ID that has ever been assigned to a button
  
  // Buttons for showing the observation trace plots
  for(set<string>::iterator t=traceAttrNames[ID].begin(); t!=traceAttrNames[ID].end(); t++) {
    // If the current trace attribute does not have the selected prefix, skip it
    if(t->find(prefix) == string::npos) continue;
    
    dotFile << "\t\t\t<TR>";
    for(int i=0; i<numInputs; i++) {
      if(ctxtNames[ID].find(i)!=ctxtNames[ID].end()) {
        for(list<string>::iterator c=ctxtNames[ID][i].begin(); c!=ctxtNames[ID][i].end(); c++) {
          int buttonID = maxButtonID; maxButtonID++;
          dotFile << dotFile << "<TD BGCOLOR=\"#"<<bgColor<<"\"><FONT POINT-SIZE=\"14\">"<<buttonID<<":"<<*t<<"</FONT></TD>";

          // Register the command to be executed when this button is clicked
          ostringstream cmd; 
          cmd << "registerModuleButtonCmd("<<buttonID<<", \""<<
                               moduleTraces[ID]->getDisplayJSCmd(traceStream::attrNames(txt()<<i<<":"<<*c), traceStream::attrNames(*t))<<
                             "\");"<<endl;
          dbg.widgetScriptCommand(cmd.str());
        } // ctxt attrs
      } else
        dotFile << dotFile << "<TD></TD>";
    } // inputs
    dotFile << "\t\t\t</TR>"<<endl;
  } // trace attrs
}

// Enter a new module within the current modularApp
// numInputs/numOutputs - the number of inputs/outputs of this module node
// ID - the unique ID of this module node
void modularApp::enterModule(string moduleName, int moduleID, int numInputs, int numOutputs, int count) {
  //cout << "modularApp::enterModule("<<moduleName<<")"<<endl;
  
  // Add a module object that records this module to the modularApp's stack
  //mStack.push_back(sight::layout::module(moduleName, moduleID, numInputs, numOutputs, count));
  
  // Start a subgraph for the current module
  dotFile << "subgraph cluster"<<moduleID<<" {"<<endl;
  //dotFile << "\tstyle=filled;"<<endl;
  dotFile << "\tcolor=black;"<<endl;
  dotFile << "\tlabel=\""<<moduleName<<"\";"<<endl;
  
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
  
  
  //dotFile << "\t\""<<portNamePrefix(moduleName)<<"\" [shape=none, fill=lightgrey, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"<<endl;
  dotFile << "\tnode"<<moduleID<<" [shape=none, fill=lightgrey, href=\"#\", onclick=\"return ClickOnModuleNode('node"<<moduleID<<"', this, ID);\", label=";
  
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
                       "COLSPAN=\""<<(ctxtNames[moduleID].find(i)!=ctxtNames[moduleID].end()? ctxtNames[moduleID][i].size(): 1)<<"\">"<<
                        "<FONT POINT-SIZE=\"20\">:In "<<i<<"</FONT>"<<
                   "</TD>";
      dotFile << "</TR>"<<endl;
      
      // The names of the context attributes for each output
      dotFile << "\t\t\t<TR>";
      for(int i=0; i<numInputs; i++) {
        if(ctxtNames[moduleID].find(i)!=ctxtNames[moduleID].end()) {
          for(list<string>::iterator c=ctxtNames[moduleID][i].begin(); c!=ctxtNames[moduleID][i].end(); c++) {
            dotFile << dotFile << "<TD BGCOLOR=\"#000066\"><FONT COLOR=\"#ffffff\" POINT-SIZE=\"18\">:"<<*c<<"</FONT></TD>";
          }
        } else
          dotFile << dotFile << "<TD></TD>";
      }
      dotFile << "\t\t\t</TR>"<<endl;
      
      // Buttons for showing the observation trace plots
      showButtons(numInputs, numOutputs, moduleID, "measure", "B0CDFF");
      showButtons(numInputs, numOutputs, moduleID, "output",  "F78181");

      dotFile << "</TABLE></TD></TR>"<<endl;
    }
  
    // Node Info
    dotFile << "\t\t<TR><TD";
    //if(numInputs + numOutputs > 0) dotFile << " COLSPAN=\""<<(numInputs>numOutputs? numInputs: numOutputs)<<"\"";
    dotFile << "><FONT POINT-SIZE=\"26\">:"<<moduleName<<"</FONT></TD></TR>"<<endl;
  
    // If we observed values during the execution of this module group  
    if(traceAttrNames[moduleID].size()>0) {
      // Polynomial fit of the observations
      vector<string> polynomials = polyFit(moduleID);
      assert(polynomials.size() == traceAttrNames[moduleID].size());
      int i=0; 
      for(set<std::string>::iterator t=traceAttrNames[moduleID].begin(); t!=traceAttrNames[moduleID].end(); t++, i++) {
        // If we were able to train a model for the current trace attribute, emit it
        if(polynomials[i] != "") printPolyFitStr(dotFile, *t, polynomials[i], 80);
      }
        //dotFile << "\t\t<TR><TD>:"<<traceAttrNames[moduleID][i]<< ": "<<wrapStr(polynomials[i], 50)<<"</TD></TR>"<<endl;
    }
    
    if(traceAttrNames[moduleID].size()>0) {
    //for(int i=0; i<traceAttrNames[moduleID].size(); i++) {
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
}

// Static version of enterModule() that pulls the from/to anchor IDs from the properties iterator and calls 
// enterModule() in the currently active modularApp
void* modularApp::enterModule(properties::iterator props) {
  assert(modularApp::activeMA);
  modularApp::activeMA->enterModule(properties::get   (props, "name"), 
                                    properties::getInt(props, "moduleID"), 
                                    properties::getInt(props, "numInputs"), 
                                    properties::getInt(props, "numOutputs"), 
                                    properties::getInt(props, "count")); 
  return NULL;
}

// Exit a module within the current modularApp
void modularApp::exitModule() {
  /* // Grab the information about the module we're exiting from this modularApp's mStack and pop it off
  assert(mStack.size()>0);
  sight::layout::module m = mStack.back();
  mStack.pop_back();*/
  
  //cout << "modularApp::exitModule("<<m.moduleName<<")"<<endl;
  // Close the current module's sub-graph
  dotFile << "}" << endl;
}

// Static version of enterModule() that calls exitModule() in the currently active modularApp
void modularApp::exitModule(void* obj) {
  assert(modularApp::activeMA);
  modularApp::activeMA->exitModule();
}

// Add a directed edge from one port to another
void modularApp::addEdge(int fromCID, common::module::ioT fromT, int fromP, 
                         int toCID,   common::module::ioT toT,   int toP,
                         double prob) {
  dotFile << "\tnode"<<fromCID<<":"<<portName(fromT, fromP)<<""<<
             " -> "<<
               "node"<<toCID  <<":"<<portName(toT,   toP  )<<" "<<
             "[penwidth="<<(1+prob*3)<<"];\n";
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* modularApp::addEdge(properties::iterator props) {
  assert(activeMA);
  activeMA->addEdge(properties::getInt(props, "fromCID"), (ioT)properties::getInt(props, "fromT"), properties::getInt(props, "fromP"),
                    properties::getInt(props, "toCID"),   (ioT)properties::getInt(props, "toT"),   properties::getInt(props, "toP"),
                    properties::getFloat(props, "prob")); 
  return NULL;
}

// Interface implemented by objects that listen for observations a traceStream reads. Such objects
// call traceStream::registerObserver() to inform a given traceStream that it should observations.
void modularApp::observe(int traceID,
                         const map<string, string>& ctxt, 
                         const map<string, string>& obs,
                         const map<string, anchor>& obsAnchor) {
  assert(trace2moduleID.find(traceID) != trace2moduleID.end());
  int moduleID = trace2moduleID[traceID];
  
  /*cout << "module::observe(this="<<this<<")(traceID="<<traceID<<", moduleID="<<moduleID<<"), #ctxtNames="<<ctxtNames.size()<<endl;
  cout << "    ctxt=";
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) { cout << c->first << ":"<<c->second<<" "; }
  cout << endl;
  cout << "    obs=";
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) { cout << o->first << ":"<<o->second<<" "; }
  cout << endl;*/
  
  // Extract the input ports with which all the context attributes are associated and record this in 
  std::map<int, std::list<std::string> > curCtxtNames;
  for(map<string, string>::const_iterator c=ctxt.begin(); c!=ctxt.end(); c++) {
    // Separate the port name and raw context name
    size_t colonLoc = c->first.find(":"); assert(colonLoc != string::npos);
    int portID = strtol(c->first.substr(0, colonLoc).c_str(), NULL, 10);
    string ctxtName = c->first.substr(colonLoc+1);
    curCtxtNames[portID].push_back(ctxtName);
  }
  if(ctxtNames.find(moduleID) == ctxtNames.end()) ctxtNames[moduleID] = curCtxtNames;
  else if(ctxtNames[moduleID] != curCtxtNames)
  { cerr << "ERROR: Inconsistent context attributes in different observations for the same module node "<<moduleID<<"! Before observed "<<ctxtNames[moduleID].size()<<" numeric context attributed but this observation has "<<curCtxtNames.size()<<"."<<endl; assert(false); }
  
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
  if(numericCtxtNames.find(moduleID) == numericCtxtNames.end()) numericCtxtNames[moduleID] = curNumericCtxtNames;
  else if(numericCtxtNames[moduleID] != curNumericCtxtNames)
  { cerr << "ERROR: Inconsistent numeric context attributes in different observations for the same module node "<<moduleID<<"! Before observed "<<numericCtxtNames[moduleID].size()<<" numeric context attributed but this observation has "<<curNumericCtxtNames.size()<<"."<<endl; assert(false); }
  
  // Do nothing if none of the context attributes are numeric
  if(numericCtxt.size()==0) return;
    
  // Read out the names of the observation's trace attributes
  /*if(traceAttrNames.find(moduleID) == traceAttrNames.end()) {
    for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++)
      traceAttrNames[moduleID].push_back(o->first);
  }*/
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    traceAttrNames[moduleID].insert(o->first);
    // If this is the first time we've encountered this trace attribute, map it to a fresh column number
    if(traceAttrName2Col[moduleID].find(o->first) == traceAttrName2Col[moduleID].end()) {
      assert(traceAttrName2Count[moduleID].size() == traceAttrName2Col[moduleID].size());
      int newCol = traceAttrName2Col[moduleID].size();
      traceAttrName2Col[moduleID][o->first] = newCol;
      
      // We have not yet recorded any observations for this trace attribute (we will later in this function)
      traceAttrName2Count[moduleID].push_back(0);
    }
  }
  
  int maxDegree = 2;
  // The total number of polynomial terms to maxDegree: 
  
  long numTerms = (numericCtxt.size()==1 ? maxDegree+1:
                       // #numericCtxt^0 + #numericCtxt^1 + #numericCtxt^2 + ... + #numericCtxt^maxDegree = #numericCtxt^(maxDegree+1) - 1
                       pow(numericCtxt.size(), maxDegree+1));
  
  //cout << "module::observe() moduleID="<<moduleID<<", #numericCtxt="<<numericCtxt.size()<<", #polyfitCtxt="<<polyfitCtxt.size()<<", found="<<(polyfitCtxt.find(moduleID) != polyfitCtxt.end())<<", numTerms="<<numTerms<<endl;
     
  // If this is the first observation we have from the given traceStream, allocate the
  // polynomial fit datastructures
  if(polyfitCtxt.find(moduleID) == polyfitCtxt.end()) {
    polyfitCtxt[moduleID] = gsl_matrix_alloc(1000, numTerms);
    
    //for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++) {
    //  polyfitObs[moduleID].push_back(gsl_vector_alloc(1000));
    //}
    polyfitObs[moduleID] = gsl_matrix_alloc(1000, traceAttrName2Col[moduleID].size());
    
    numObs[moduleID] = 0;
    numAllocObs[moduleID] = 1000;
    numAllocTraceAttrs[moduleID] = traceAttrName2Col[moduleID].size();
    
    //cout << "    Allocated "<<numAllocObs[moduleID]<<" rows, "<<numAllocTraceAttrs[moduleID]<<" columns"<<endl;
  }
  //cout << "module::observe() #polyfitCtxt="<<polyfitCtxt.size()<<", found="<<(polyfitCtxt.find(moduleID) != polyfitCtxt.end())<<endl;
  
  //cout << "traceAttrName2Col[moduleID].size()="<<traceAttrName2Col[moduleID].size()<<" numAllocTraceAttrs[moduleID]="<<numAllocTraceAttrs[moduleID]<<endl;
  // If we're out of space in polyfitCtxt[moduleID] and polyfitObs[moduleID] to store another observation or store more columns, grow them
  if(numObs[moduleID] == numAllocObs[moduleID] || 
     traceAttrName2Col[moduleID].size() > numAllocTraceAttrs[moduleID]) {
    // If we're out of space for rows, double it
    int newNumAllocObs = numAllocObs[moduleID];
    if(numObs[moduleID] == numAllocObs[moduleID]) newNumAllocObs *= 2;
    
    // If we need new columns, adjust accordingly
    int newNumAllocTraceAttrs = numAllocTraceAttrs[moduleID];
    if(traceAttrName2Col[moduleID].size() > numAllocTraceAttrs[moduleID])
      newNumAllocTraceAttrs = traceAttrName2Col[moduleID].size();
    
    // Expand polyfitCtxt[moduleID]
    gsl_matrix* newCtxt = gsl_matrix_alloc(newNumAllocObs, numTerms);
    gsl_matrix_view newCtxtView = gsl_matrix_submatrix (newCtxt, 0, 0, numAllocObs[moduleID], numTerms);
    gsl_matrix_memcpy (&(newCtxtView.matrix), polyfitCtxt[moduleID]);
    gsl_matrix_free(polyfitCtxt[moduleID]);
    polyfitCtxt[moduleID] = newCtxt;
    
    // Expand polyfitObs[moduleID]
    gsl_matrix* newObs = gsl_matrix_alloc(newNumAllocObs, newNumAllocTraceAttrs);
    gsl_matrix_view newObsView = gsl_matrix_submatrix (newObs, 0, 0, numAllocObs[moduleID], numAllocTraceAttrs[moduleID]);
    gsl_matrix_memcpy (&(newObsView.matrix), polyfitObs[moduleID]);
    gsl_matrix_free(polyfitObs[moduleID]);
    polyfitObs[moduleID] = newObs;
    
    // Update our allocated space records
    numAllocObs[moduleID] = newNumAllocObs;
    numAllocTraceAttrs[moduleID] = newNumAllocTraceAttrs;
    //cout << "    Reallocated "<<numAllocObs[moduleID]<<" rows, "<<numAllocTraceAttrs[moduleID]<<" columns"<<endl;
  }
  
  // Add this observation to polyfitObs
  int obsIdx=0;
  // Records whether this observation corresponds to a new context row
  bool newContext = false;
  
  //cout << "    Adding Observations:"<<endl;
  for(map<string, string>::const_iterator o=obs.begin(); o!=obs.end(); o++, obsIdx++) {
    // For now we assert that all observed values must be numeric
    const char *valStr = o->second.c_str();
    char *nextValStr;
    double v = strtod(valStr, &nextValStr);
    assert(nextValStr != valStr);
    
    int traceAttrCol = traceAttrName2Col[moduleID][o->first];
    /*cout << "        "<<o->first<<"  traceAttrCol="<<traceAttrCol<<
               ", numAllocTraceAttrs[moduleID]="<<numAllocTraceAttrs[moduleID]<<
               ", traceAttrName2Count[moduleID][traceAttrCol]="<<traceAttrName2Count[moduleID][traceAttrCol]<<
               ", numObs[moduleID]="<<numObs[moduleID]<<endl;*/
    
    assert(traceAttrCol < numAllocTraceAttrs[moduleID]);
    gsl_matrix_set(polyfitObs[moduleID], traceAttrName2Count[moduleID][traceAttrCol], traceAttrCol, v);
    
    // Increment the number of values written into the current column
    traceAttrName2Count[moduleID][traceAttrCol]++;
    
    // If this observation has started a new row in the observations matrix
    if(traceAttrName2Count[moduleID][traceAttrCol] > numObs[moduleID]) {
      assert(traceAttrName2Count[moduleID][traceAttrCol] == numObs[moduleID]+1);
      // Update numObs[moduleID] to correspond to the maximum number of values written to any column
      numObs[moduleID] = traceAttrName2Count[moduleID][traceAttrCol];
      
      // Record that this is a new context, which should be written into polyfitCtxt
      newContext = true;
    }
  }

  // Add the context of the observation to polyfitCtxt
      
  // The first entry corresponds to the degree-0 constant term
  gsl_matrix_set(polyfitCtxt[moduleID], numObs[moduleID], 0, 1);

  // Add all the polynomial terms of degrees upto and including maxDegree
  int col=1;
  for(int degree=1; degree<=maxDegree; degree++)
    addPolyTerms(numericCtxt, 0, degree, numObs[moduleID], col, 1, polyfitCtxt[moduleID]);
}

}; // namespace layout
}; // namespace sight
