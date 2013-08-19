#include "trace.h"
using namespace std;

namespace dbglog {

// Stack of currently active traces
std::list<trace*> trace::stack;
  
// Records whether the tracer infrastructure has been initialized
bool trace::initialized=false;

trace::trace(std::string label, const std::list<std::string>& contextAttrs) : block(label), contextAttrs(contextAttrs) {
  if(contextAttrs.size()==0) { cerr << "trace::trace() ERROR: contextAttrs must be non-empty!"; exit(-1); }
  
  init();
}

trace::trace(std::string label, std::string contextAttr) : block(label){
  contextAttrs.push_back(contextAttr);
  
  init();
}


void trace::init() {
  if(!initialized) {
    dbg.includeFile("trace.js");
    //dbg.includeScript("https://www.google.com/jsapi", "text/javascript");
    dbg.includeScript("http://yui.yahooapis.com/3.11.0/build/yui/yui-min.js", "text/javascript");
    dbg.includeWidgetScript("trace.js", "text/javascript");
    initialized = true;
  }
  
  stack.push_back(this);
  
  dbg << "<div class=\"example yui3-skin-sam\">\n";
  dbg.enterBlock(this, false, true);
  dbg.exitBlock();
  dbg << "</div>\n";
  //dbg.widgetScriptPrologCommand(txt()<<"loadGoogleAPI();");
  dbg.widgetScriptEpilogCommand(txt()<<"displayTrace('"<<getLabel()<<"', '"<<getBlockID()<<"');");
}

trace::~trace() {
  
  assert(stack.size()>0);
  stack.pop_back();
}

void traceAttr(std::string key, const attrValue& val) {
  assert(trace::stack.size()>0);
  trace* t = *(trace::stack.rbegin());
  
  ostringstream cmd;
  cmd << "traceRecord('"<<key<<"', '"<<val.getAsStr()<<"', {";
  for(std::list<std::string>::iterator a=t->contextAttrs.begin(); a!=t->contextAttrs.end(); a++) {
    if(a!=t->contextAttrs.begin()) cmd << ", ";
    const std::set<attrValue>& vals = attributes.get(*a);
    assert(vals.size()>0);
    if(vals.size()>1) { cerr << "trace::traceAttr() ERROR: context attribute "<<*a<<" has multiple values!"; }
    cmd << *a << ":" << vals.begin()->getAsStr();
  }
  cmd << "});";
  dbg.widgetScriptCommand(cmd.str());
}

}; // namespace dbglog
