#include "sight.h"
#include <map>
#include <vector>
#include <assert.h>
using namespace std;
using namespace sight;

int fib(int a);
int fibLinear(int a);
int fibBase(int a);
int fibIndent(int a);
int fibScope(int a, scope::scopeLevel level);
int fibScopeLinks(int a, scope::scopeLevel level, list<int>& stack, 
                  map<list<int>, anchor>& InFW, 
                  map<list<int>, anchor>& InBW, 
                  map<list<int>, anchor>& OutFW,
                  map<list<int>, anchor>& OutBW,
                  bool doFWLinks);
int fibGraph(int a, graph& g, anchor* parent);
double histRecurrence(int a, const vector<double>& hist);
std::pair<int, std::vector<port> > fibModule(int a, int depth);

int main(int argc, char** argv)
{
  if(argc<2) { printf("Usage: 0.Demo maxDepth\n"); exit(-1); }
  int maxDepth = atoi(argv[1]);
  
  // The absolute path of this file. This is necessary to have Sight include the relevant regions of 
  // source code in the output.
  string thisFile = txt()<<ROOT_PATH<<"/examples/0.Demo.C";
  
  SightInit(argc, argv, "Demo", txt()<<"dbg.0.Demo.maxDepth_"<<maxDepth);
   
  dbg << "<h1>Demonstration of Sight</h1>" << endl;
  
  { 
    scope s("No formatting", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "NFStart",      "NFEnd"),
                                           source::reg(thisFile, "fibBaseStart", "fibBaseEnd"))); }
    
#pragma sightLoc NFStart
    for(int depth=1; depth<maxDepth; depth++) {
      dbg << "<<<<< Depth "<<depth<<" <<<<<"<<endl;
      fibBase(depth);
      dbg << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
    }
#pragma sightLoc NFEnd
  }
  
  { 
    scope s("Indentation", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "IndStart",       "IndEnd"),
                                           source::reg(thisFile, "fibIndentStart", "fibIndentEnd"))); }
    
#pragma sightLoc IndStart
    for(int depth=1; depth<maxDepth; depth++) {
      dbg << "<<<<< Depth "<<depth<<" <<<<<"<<endl;
      fibIndent(depth);
      dbg << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
    }
#pragma sightLoc IndEnd
  }
  
  { 
    scope s("Indentation Mixed with Scoping", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "IndScopeStart",  "IndScopeEnd"),
                                           source::reg(thisFile, "fibIndentStart", "fibIndentEnd"))); }
    
#pragma sightLoc IndScopeStart
    for(int depth=1; depth<maxDepth; depth++) {
      scope s2(txt()<<"Depth "<<depth);
      fibIndent(depth);
    }
#pragma sightLoc IndScopeEnd
  }
  
  { 
    scope s("Multi-level Scoping", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "MultScopeStart", "MultScopeEnd"),
                                           source::reg(thisFile, "fibScopeStart",  "fibScopeEnd"))); }
    
#pragma sightLoc MultScopeStart
    for(int depth=1; depth<maxDepth; depth++) {
      scope s2(txt()<<"Depth "<<depth, scope::high);
      fibScope(depth, scope::medium);
    }
#pragma sightLoc MultScopeEnd
  }
  
  { 
    scope s("Multi-level Scoping with Links", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "MultScopeLinksStart", "MultScopeLinksEnd"),
                                           source::reg(thisFile, "fibScopeLinksStart",  "fibScopeLinksEnd"))); }
    
#pragma sightLoc MultScopeLinksStart
    map<list<int>, anchor> InFW, InBW, OutFW, OutBW;
    for(int depth=1; depth<maxDepth; depth++) {
      scope s2(txt()<<"Depth "<<depth, scope::high);
      list<int> stack;
      fibScopeLinks(depth, scope::medium, stack, InFW, InBW, OutFW, OutBW, true);
      InFW = OutFW;
      InBW = OutBW;
      OutFW.clear();
      OutBW.clear();
    }
#pragma sightLoc MultScopeLinksEnd
  }
  
  { 
    scope s("Multi-level Scoping with Graphs", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "MultScopeGraphsStart", "MultScopeGraphsEnd"),
                                           source::reg(thisFile, "fibGraphStart",        "fibGraphEnd"))); }

#pragma sightLoc MultScopeGraphsStart
    graph g;
    for(int depth=1; depth<maxDepth; depth++) {
      scope s2(txt()<<"Depth "<<depth, scope::high);
      fibGraph(depth, g, NULL);
    }
#pragma sightLoc MultScopeGraphsEnd
  }

  {
    scope s("Performance Analysis", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "PerfAnalysisStart", "PerfAnalysisEnd"),
                                           source::reg(thisFile, "fibStart",          "fibEnd"),
                                           source::reg(thisFile, "fibLinearStart",    "fibLinearEnd"))); }
    
#pragma sightLoc PerfAnalysisStart
    trace tTime("Fib Time", "depth", trace::showBegin, trace::lines);
    trace tValue("Fib Value", "depth", trace::showBegin, trace::table);
    // Recursive
    for(int depth=1; depth<30; depth++) {
      attr depthAttr("depth", depth);
      measure* m = startMeasure<timeMeasure>("Fib Time", "Recursive");
      int value = fib(depth);
      endMeasure(m);
      traceAttr("Fib Value", "val", attrValue(value));
    }

    // Linear
    for(int depth=1; depth<30; depth++) {
      attr depthAttr("depth", depth);
      measure* m = startMeasure<timeMeasure>("Fib Time", "Recursive");
      fibLinear(depth);
      endMeasure(m);
    }
#pragma sightLoc PerfAnalysisEnd
  }

  {
    scope s("Modular Analysis", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "ModularStart",    "ModularEnd"),
                                           source::reg(thisFile, "modularFibStart", "modularFibEnd"))); }
    
#pragma sightLoc ModularStart
    modularApp modularFibonacci("Fibonacci"); 
    fibModule(10, 0);
#pragma sightLoc ModularEnd
  }
  
/*  { 
    scope s("Modular Analysis", scope::high);
    { source src("source", source::regions(source::reg(thisFile, "ModularStart",        "ModularEnd"),
                                           source::reg(thisFile, "histRecurrenceStart", "histRecurrenceEnd"))); }
     
 # pragma sightLoc ModularStart
    module rootModule(group("Modular Analysis", 0, 0), namedMeasures("time", new timeMeasure()));
    
    // Initialize the linear recurrence coefficients to Fibonacci
    for(int i=0; i<30; i++) {
      //scope si(txt()<<"Recurrence Index "<<i, scope::high);
      
      vector<double> linear;
      vector<port> initOutputs;
      {
        module initModule(group("Initialization", 0, 2), inputs(), initOutputs, namedMeasures("time", new timeMeasure()));
        linear.push_back(1);
        linear.push_back(1);
        initModule.setOutCtxt(0, context(config("history", 2)));
        initModule.setOutCtxt(1, context(config("idx",     0)));
      }

      vector<port> analysisOutputs;
   
      double lastValue=1; 
      for(int j=0; j<10; j++) {
        //scope sj(txt()<<"History Extention Iteration "<<i);

        double value;
        std::vector<port> recurOutputs;
        {
          module recurModule(group("Recurrence", 2, 1),
                             inputs(j==0? initOutputs[0]: analysisOutputs[0],
                                    j==0? initOutputs[1]: analysisOutputs[1]),
                             recurOutputs,
                             namedMeasures("time", new timeMeasure()));
          value = histRecurrence(i, linear);
        }
     
        if(j==0 || j%5==0) 
        {
          module analysisModule(group("Analysis", 1, 2),
                                inputs(recurOutputs[0]),
                                analysisOutputs,
                                namedMeasures("time", new timeMeasure()));

          linear.push_back((int)(value/lastValue + 1));
          analysisModule.setOutCtxt(0, context(config("history", (int)linear.size())));
          analysisModule.setOutCtxt(1, context(config("idx",     i)));
        }
        lastValue = value;
      }
 # pragma sightLoc ModularEnd
    }
  }*/
  
  return 0;
}

#pragma sightLoc fibStart
int fib(int a) {
  if(a==0 || a==1) { 
    return 1;
  } else {
    int val = fib(a-1) + fib(a-2);
    return val;
  }
}
#pragma sightLoc fibEnd

#pragma sightLoc fibLinearStart
int fibLinear(int a) {
  int x=1;
  int y=1;
  for(int i=1; i<a; i++) {
    int tmp = y;
    y = x+y;
    x = y;
  }
  return y;
}
#pragma sightLoc fibLinearEnd

#pragma sightLoc fibBaseStart
int fibBase(int a) {
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibBase(a-1) + fibBase(a-2);
    dbg << "="<<val<<endl;
    return val;
  }
}
#pragma sightLoc fibBaseEnd

#pragma sightLoc fibIndentStart
int fibIndent(int a) {
  indent ind(":  ");
  
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibIndent(a-1) + fibIndent(a-2);
    dbg << "="<<val<<endl;
    return val;
  }
}
#pragma sightLoc fibIndentEnd

// Each recursive call to fibScope() generates a new scope at the desired level. 
// The scope level that is passed in controls the types of scopes that are recursively created
#pragma sightLoc fibScopeStart
int fibScope(int a, scope::scopeLevel level) {
  scope s(txt()<<"fib("<<a<<")", level);
  
  if(a==0 || a==1) { 
    dbg << "=1."<<endl;
    return 1;
  } else {
    int val = fibScope(a-1, level) + 
              fibScope(a-2, level);
    dbg << "="<<val<<endl;
    return val;
  }
}
#pragma sightLoc fibScopeEnd

// InFW links: links from prior fib call to this one. These need to be anchored to the source::regions in this next.
// InBW links: links from this fib call nest to the prior one. These have already been anchored to the prior nest.
// OutBW links: links from the next nest to this one. These are anchored to established source::regions.
// OutFW links: links from this nest to the next one. These are un-anchored and will be anchored to the next nest
#pragma sightLoc fibScopeLinksStart
int fibScopeLinks(int a, scope::scopeLevel level, list<int>& stack, 
                  map<list<int>, anchor>& InFW, 
                  map<list<int>, anchor>& InBW, 
                  map<list<int>, anchor>& OutFW,
                  map<list<int>, anchor>& OutBW,
                  bool doFWLinks) {
  stack.push_back(a); // Add this call to stack
  list<int> stackNoTop = stack; stackNoTop.pop_front();
  //dbg << "stack="; for(list<int>::iterator i=stack.begin(); i!=stack.end(); i++) dbg << *i << " "; dbg << endl;
  //dbg << "stackNoTop="; for(list<int>::iterator i=stackNoTop.begin(); i!=stackNoTop.end(); i++) dbg << *i << " "; dbg << endl;
  
  // Each recursive call to fibScopeLinks generates a new scope at the desired level. To reduce the amount of text printed, we only 
  // generate scopes if the value of a is >= verbosityLevel
  scope s(txt()<<"fib("<<a<<")", 
            (InFW.find(stackNoTop)!=InFW.end()? InFW[stackNoTop]: anchor::noAnchor),
            level);
  OutBW[stack] = s.getAnchor();
  
  if(a==0 || a==1) { 
    dbg << "=1."<<endl;
    if(doFWLinks) {
      anchor fwLink;
      fwLink.linkImg("Forward link"); dbg << endl;
      OutFW[stack] = fwLink;
    }

    if(InBW.find(stackNoTop)!=InBW.end()) { 
      InBW[stackNoTop].linkImg("Backward link"); dbg<<endl;
    }
    
    //cout << "link="<<dbg.linkTo(linkScopes[stack], "go")<<endl;
    stack.pop_back(); // Remove this call from stack
    return 1;
  } else {
    int val = fibScopeLinks(a-1, level, stack, InFW, InBW, OutFW, OutBW, doFWLinks) + 
              fibScopeLinks(a-2, level, stack, InFW, InBW, OutFW, OutBW, doFWLinks);
    dbg << "="<<val<<endl;
    
    if(doFWLinks) {
      anchor fwLink;
      fwLink.linkImg("Forward link"); dbg << endl;
      OutFW[stack] = fwLink;
    }
    if(InBW.find(stackNoTop)!=InBW.end())
    { InBW[stack].linkImg("Backward link"); dbg<<endl; }
    
    //cout << "link="<<dbg.linkTo(linkScopes[stack], "go")<<endl;
    stack.pop_back(); // Remove this call from stack
    return val;
  }
}
#pragma sightLoc fibScopeLinksEnd

// Fibonacci, where we create graph edges from each call to its children.
#pragma sightLoc fibGraphStart
int fibGraph(int a, graph& g, anchor* parent) {
  scope s(txt()<<"fib("<<a<<")");
  anchor sAnchor = s.getAnchor();
  
  // Create a link from the calling scope to this one
  if(parent) g.addDirEdge(*parent, sAnchor);
  
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibGraph(a-1, g, &sAnchor) + fibGraph(a-2, g, &sAnchor);
    dbg << "="<<val<<endl;
    return val;
  }
}
#pragma sightLoc fibGraphEnd

// General recurrence where the last hist numbers are added to produce the next number.
#pragma sightLoc histRecurrenceStart
double histRecurrence(int a, const vector<double>& hist) {
//  cout << "histRecurrence(a="<<a<<", #hist="<<hist.size()<<")"<<endl;
  if(a<hist.size())
    return 1;
  else {
    double ret=0;
    for(int i=0; i<hist.size(); i++)
      ret += hist[i] * histRecurrence(a-hist.size()+i, hist);
    return ret;
  }
}
#pragma sightLoc histRecurrenceEnd

#pragma sightLoc modularFibStart
// Each recursive call to fibModule() generates a new module at the desired level. 
std::pair<int, std::vector<port> > fibModule(int a, int depth) {
  std::vector<port> fibOutputs;
  module m(instance(txt()<<"fib() depth="<<depth, 1, 1), 
           inputs(port(context("a", a))), fibOutputs, namedMeasures("time", new timeMeasure()));
  
  if(a==0 || a==1) { 
    dbg << "=1."<<endl;
    
    m.setOutCtxt(0, context("val", 1));
    
    return make_pair(1, fibOutputs);
  } else {
    std::pair<int, std::vector<port> > ret1 = fibModule(a-1, depth+1);
    std::pair<int, std::vector<port> > ret2 = fibModule(a-2, depth+1);
    dbg << "="<<(ret1.first + ret2.first)<<endl;
    
    m.setOutCtxt(0, context("val", ret1.first + ret2.first));
    
    return make_pair(ret1.first + ret2.first, fibOutputs);
  }
}
#pragma sightLoc modularFibEnd
