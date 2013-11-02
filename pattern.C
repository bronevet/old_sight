#include "pattern.h"
#include <assert.h>
#include <vector>
#include <set>
#include <list>
#include <map>
using namespace std;

#include "sight.h"
using namespace sight;

// Returns a string representation of the given collection
template<class T>
string col2Str(const T& collection) {
  ostringstream oss;
  for(typename T::const_iterator i=collection.begin(); i!=collection.end(); i++) {
    if(i!=collection.begin()) oss<<", ";
    oss << *i;
  }
  return oss.str();
}

/*********************
 ***** aggregate *****
 *********************/

// Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
// all of which are dynamically allocated.
std::set<aggregate*> aggregate::add(const aggregate& newAggr) {
  set<aggregate*> resAggr;
  
  // If these aggregates have the same dimensionality and the same sub-type of aggregate, 
  // merge them to create a single higher-dimensional aggregate of their descriptors
  scope s("aggregate::add");
  dbg << "this="<<str()<<", desc="<<col2Str(getDescriptor())<<endl;
  dbg << "newAggr="<<newAggr.str()<<", desc="<<col2Str(newAggr.getDescriptor())<<endl;
  if(dim == newAggr.getDim() && getAggrType()==newAggr.getAggrType()) {
    // If these two aggregates can be grouped into a valid point, create it
    if(sameDescriptors(*this, newAggr))
      resAggr.insert(new point(*this, newAggr));
    
    // All other aggregate types are guaranteed to form a valid aggregate from two points
    resAggr.insert(new line(*this, newAggr));
    // ... more entries as we implement more aggregates
  
  // If the new aggregate has one lower dimension than this aggregate, this aggregate should be extended with newAggr
  } else if(dim == newAggr.getDim()+1) {
    resAggr = extend(newAggr);
  }
  
  dbg << "#resAggr="<<resAggr.size()<<endl;
  return resAggr;
}

// Returns whether these two aggregates have the same descriptors
bool aggregate::sameDescriptors(const aggregate& aggr1, const aggregate& aggr2) {
  vector<int>::const_iterator i1=aggr1.getDescriptor().begin();
  vector<int>::const_iterator i2=aggr2.getDescriptor().begin();
  for(; i1!=aggr1.getDescriptor().end(); i1++, i2++) {
    // If there is any difference between the two aggregates' descriptors, return false
    if(*i1 != *i2) return false;
  }
  
  // The aggregates have the same descriptors
  return true;
}

std::string aggregate::aggrType2Str(aggrType t) {
       if(t==noneT)  return "noneT";
  else if(t==pointT) return "pointT";
  else if(t==lineT)  return "lineT";
  assert(0);  
}

std::string aggregate::intStack2Str() const {
  ostringstream oss;
  for(vector<aggrType>::const_iterator i=intStack.begin(); i!=intStack.end(); i++) {
    if(i!=intStack.begin()) oss<<", ";
    oss << aggrType2Str(*i);
  }
  return oss.str();
}


// Add an edge from this aggregate to the given aggregate.
void aggregate::addEdgeTo(aggregate* to) {
//dbg << "#next="<<next.size()<<", #to->pred="<<to->pred.size()<<endl;
  next.insert(to);
  //if(to!=this)
    to->pred.insert(this);
}

// Remove an edge from this aggregate to the given aggregate, if one exists
void aggregate::rmEdgeTo(aggregate* to) {
//dbg << "#next="<<next.size()<<", #to->pred="<<to->pred.size()<<endl;
  assert(next.find(to) != next.end());
  next.erase(to);
  //if(to!=this) {
    assert(to->pred.find(this) != to->pred.end());
    to->pred.erase(this);
//  }
}

/*****************
 ***** point *****
 *****************/

point::point(const aggregate& newAggr) :  pt(newAggr.getDescriptor()), aggregate(newAggr)
{ }

point::point(const aggregate& aggr1, const aggregate& aggr2) :  pt(aggr1.getDescriptor()), aggregate(aggr1.getDim(), aggr1.getNumPts()+aggr2.getNumPts())
{
  assert(aggr1.getDim() == aggr2.getDim());
}

point::point(const std::vector<int>& pt, int numPts) :  pt(pt), aggregate(1, numPts)
{ }

// Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
// all of which are dynamically allocated.
std::set<aggregate*> point::extend(const aggregate& newAggr) {
  assert(0);
  return set<aggregate*>();
/*  set<aggregate*> ext;
  if(aggregate::sameDescriptor(pt, newAggr)) {
    ext.insert(new point(pt, numPts+1));
  } else {
    ext.insert(new line(pt, newAggr.getDescriptor(), numPts+newAggr.getNumPts()));
  }
  return ext;*/
}

// Returns a point vector that describes this aggregate
const std::vector<int>& point::getDescriptor() const {
  return pt;
}

// Returns the value of using this aggregate to summarize its points
double point::getValue() const {
  return low * getNumPts();
}

// Returns a dynamically-allocated copy of this aggregate
aggregate* point::copy() const {
  return new point(pt, getNumPts());
}

// Returns a human-readable string representation of this aggregate
std::string point::str() const {
  ostringstream oss;
  
  oss << "[point: <"<<col2Str(pt)<<">, dim="<<getDim()<<", numPts="<<getNumPts()<<"]";
  
  return oss.str();
}

// Returns a human-readable string representation of this aggregate's loop structure. 
// Used for generating output to users.
// Takes as input a vector of strings that denote the variables/constants defined by higher 
// levels of the loop structure that will control the parameters of this aggregate.
std::string point::loopStr(std::vector<std::string> ctrl) const {
  assert(ctrl.size()==1);
  return ctrl[0];
}

/****************
 ***** line *****
 ****************/
line::line(const aggregate& newAggr1, const aggregate& newAggr2) : aggregate(newAggr1.getDim()+1, newAggr1.getNumPts() + newAggr2.getNumPts()) {
  dbg << "newAggr1="<<newAggr1.str()<<endl;
  dbg << "newAggr2="<<newAggr2.str()<<endl;
  assert(newAggr1.getDim() == newAggr2.getDim());
  assert(newAggr1.getDescriptor().size() == newAggr2.getDescriptor().size());
  assert(newAggr1.getAggrType() == newAggr2.getAggrType());
  
  vector<int>::const_iterator i1=newAggr1.getDescriptor().begin();
  vector<int>::const_iterator i2=newAggr2.getDescriptor().begin();
  for(; i1!=newAggr1.getDescriptor().end(); i1++, i2++) {
    slope.push_back(*i2 - *i1);
    first.push_back(*i1);
  }
  
  // Copy the intStack from newAggr2 to this new aggregate, adding newAggr2's type
  intStack = newAggr2.getIntStack();
  intStack.push_back(newAggr2.getAggrType());
}

line::line(const line& that): aggregate(that.getDim(), that.getNumPts(), that.getIntStack()), first(that.first), slope(that.slope) {
  assert(first.size() == slope.size());
}

line::line(const std::vector<int>& first, const std::vector<int>& slope, int dim, int numPts, const std::vector<aggrType>& intStack) : 
  aggregate(dim, numPts, intStack), first(first), slope(slope) {
  assert(first.size() == slope.size());
}

// Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
// all of which are dynamically allocated.
std::set<aggregate*> line::extend(const aggregate& newAggr) {
  assert(newAggr.getDescriptor().size() == first.size());
  assert(newAggr.getDescriptor().size() == slope.size());
  
  // Check if newAggr represents a continuation of the sequence
  vector<int>::const_iterator iNew=newAggr.getDescriptor().begin();
  vector<int>::const_iterator iFirst=first.begin();
  vector<int>::const_iterator iSlope=slope.begin();
  bool match=true;
  for(; iNew!=newAggr.getDescriptor().end(); iNew++, iFirst++, iSlope++) {
    if(*iNew != *iFirst + *iSlope * getNumPts()) {
      match = false;
      break;
    }
  }
  
  set<aggregate*> ext;
  // If the new point matches the pattern, return the extended line
  if(match) {
    // Copy the intStack from newAggr2 to this new aggregate, adding newAggr2's type
    std::vector<aggrType> intStack = newAggr.getIntStack();
    intStack.push_back(newAggr.getAggrType());
    ext.insert(new line(first, slope, getDim(), getNumPts()+1, intStack));
  }
  
  return ext;
}

// Returns a point vector that describes this aggregate
const std::vector<int>& line::getDescriptor() const {
  // TODO: perform this calculation only when the line has changed
  ((line*)this)->descriptor.clear();
   
  // Append the first and slope vectors to create the descriptor
  for(vector<int>::const_iterator i=first.begin(); i!=first.end(); i++)
    ((line*)this)->descriptor.push_back(*i);
  for(vector<int>::const_iterator i=slope.begin(); i!=slope.end(); i++)
    ((line*)this)->descriptor.push_back(*i);
  return descriptor;
}

// Returns the value of using this aggregate to summarize its points
double line::getValue() const {
  if(getNumPts()==2) { return low*2; }
  else               { return high * getNumPts(); }
}

// Returns a dynamically-allocated copy of this aggregate
aggregate* line::copy() const {
  //return new line(first, slope, getDim(), getNumPts(), intStack);
  return new line(*this);
}

// Returns a human-readable string representation of this aggregate
std::string line::str() const {
  ostringstream oss;
  oss << "[line: first=<"<<col2Str(first)<<">, slope=<"<<col2Str(slope)<<">, dim="<<getDim()<<", numPts="<<getNumPts()<<", desc=<"<<col2Str(getDescriptor())<<">";
  oss << ", intStack=<"<<intStack2Str()<<">]";
  return oss.str();
}

// Returns a human-readable string representation of this aggregate's loop structure. 
// Used for generating output to users.
// Takes as input a vector of strings that denote the variables/constants defined by higher 
// levels of the loop structure that will control the parameters of this aggregate.
std::string line::loopStr(std::vector<std::string> ctrl) const {
  assert(ctrl.size()==2);
  assert(first.size()>0);
  ostringstream oss;
  oss << "for i "<<first[0]<<
  vector<string> subCtrl;
  for(int j=0; j<first.size(); j++)
    if(slope[j]==0) subCtrl.push_back(first[j]);
    else            subCtrl.push_back(txt()<<first[j]<<"+i*"<<slope[j]);
  }
  switch
  return ctrl[0];
}

/**********************
 ***** transGraph *****
 **********************/

transGraph::transGraph() {
  value = 0;
  entry = new entryNode();
  curFrom = entry;
  curTo = NULL;
}

// Functor that deletes all the nodes over which it is run
class destructFunctor : public transGraph::GraphNodeFunctor
{
  public:
  destructFunctor(transGraph* graph): transGraph::GraphNodeFunctor(graph) {}
  
  void operator()(aggregate* a) {
    delete a;
  }
};

transGraph::~transGraph() {
  destructFunctor df(this);
  mapNode(df);
}

// Functor for merging one existing graph node into another
class mergeFunctor : public transGraph::GraphNodeFunctor
{
  aggregate* mergeAggr;
  public:
  set<transGraph*>& alternateGraphs;
  mergeFunctor(transGraph* graph, aggregate* mergeAggr, set<transGraph*>& alternateGraphs): 
                transGraph::GraphNodeFunctor(graph), mergeAggr(mergeAggr), alternateGraphs(alternateGraphs) {
    assert(graph->curTo == mergeAggr);
  }
  
  void operator()(aggregate* curAggr) {
    // Don't merge mergeAggr into itself
    if(curAggr == mergeAggr) return;

    //attrTrue aTrue;
    set<aggregate*> extensions = curAggr->add(*mergeAggr);
    scope sm(txt()<<"mergeFunctor: a="<<curAggr->str()<<", #extensions="<<extensions.size());
    // Create an alternate graph for each possible extension
    for(set<aggregate*>::iterator e=extensions.begin(); e!=extensions.end(); e++) {
      dbg << "extension="<<(*e)->str()<<endl;
      // Create an alternate graph where the current extended aggregate replaces the original one
      transGraph* newG = graph->clone(curAggr, *e);

      //dbg << "newG="<<newG<<endl; graph::genGraph(*newG);
      
      // Delete the node in the cloned graph that corresponds to mergeAggr (the target of the 
      // graph's current edge) and update the edges to go through this node to *e.
      aggregate* newMergeAggr = newG->curTo;
      
      // Reconnect all the edges that point to newG->curTo so that they point to *e;
      /*{ scope s(txt()<<"Predecessors of "<<newMergeAggr->str());
      for(set<aggregate*>::const_iterator p=newMergeAggr->getPred().begin(); p!=newMergeAggr->getPred().end(); p++) {
        scope s(txt()<<(*p)->str()<<" : "<<*p, scope::low);
        for(set<aggregate*>::const_iterator n=(*p)->getNext().begin(); n!=(*p)->getNext().end(); n++) {
          dbg << (*n)->str()<<endl;
        }
      } }*/
      set<aggregate*> preds = newMergeAggr->getPred();
      for(set<aggregate*>::const_iterator p=preds.begin(); p!=preds.end(); p++) {
        (*p)->rmEdgeTo(newMergeAggr);
        (*p)->addEdgeTo(*e);
      }
      if(newG->curFrom == newG->curTo)
        newG->curFrom = *e;
      newG->curTo = *e;
      //dbg << "merged newG="<<newG<<endl; graph::genGraph(*newG);
      
      // Delete the now irrelevant mergeAggr variant in newG
      delete newMergeAggr;
      
      // Add this alternate graph to the set
      alternateGraphs.insert(newG);
    }
  }
  
  // NOTE: The resulting set of alternate graphs does not include the original graph
}; // class mergeFunctor


class addFunctor : public transGraph::GraphNodeFunctor
{
  point pt;
  public:
  set<transGraph*>& alternateGraphs;
  addFunctor(transGraph* graph, const std::vector<int>& pt, set<transGraph*>& alternateGraphs): 
    transGraph::GraphNodeFunctor(graph), pt(pt, 1), alternateGraphs(alternateGraphs) {}
  
  void operator()(aggregate* a) {
    //attrTrue aTrue;
    set<aggregate*> extensions = a->add(pt);
    scope sa(txt()<< "addFunctor: a="<<a->str()<<", #extensions="<<extensions.size());
    // Create an alternate graph for each possible extension
    for(set<aggregate*>::iterator e=extensions.begin(); e!=extensions.end(); e++) {
      dbg << "extension="<<(*e)->str()<<endl;
      dbg << "graph="<<graph<<endl; graph::genGraph(*graph);
      // Create an alternate graph where the current extended aggregate replaces the original one
      transGraph* newG = graph->clone(a, *e);
      
      dbg << "newG="<<newG<<endl; graph::genGraph(*newG);
      
      // Add an edge in the new graph from the current edge's target to *e
      // and advance the graph's current edge to be this edge
      newG->curTo->addEdgeTo(*e);
      newG->curFrom = newG->curTo;
      newG->curTo = *e;
      //graph::genGraph(*newG);
      
      dbg << "newG="<<newG<<endl; graph::genGraph(*newG);
      
      // Add this alternate graph to the set
      alternateGraphs.insert(newG);
      
      // Also create alternate graphs for merging the new extension with a prior node
      mergeFunctor mf(newG, *e, alternateGraphs);
      newG->mapNode(mf);
      
      dbg << "newG="<<newG<<endl; graph::genGraph(*newG);
    }
  }
  
  // NOTE: The resulting set of alternate graphs does not include the original graph
}; // class addFunctor

// Add a new point to the graph.
// This graph is adjusted such that the new point is a new node. Further,
// the function returns a set of alternate graphs where the new point is 
// merged into an existing graph node.
set<transGraph*> transGraph::add(const std::vector<int>& pt)
{
  scope pa(txt()<<"transGraph::add("<<col2Str(pt)<<")");
  // If this is the first point in the graph
  if(curTo==NULL) {
    curTo = new point(pt, 1);
    value = curTo->getValue();
    entry->addEdgeTo(curTo);
    dbg << "curTo==NULL, so creating new node. new curTo="<<curTo->str()<<", value="<<value<<endl;
    return set<transGraph*>();
  } else {
    dbg << "curTo!=NULL"<<endl;
    
    // Consider adding this point to all the nodes in the graph
    set<transGraph*> alternateGraphs;
    addFunctor af(this, pt, alternateGraphs);
    mapNode(af);
    
    // Also consider creating a new graph node for this point, modifying the original graph
    aggregate* newPt = new point(pt, 1);
    curFrom = curTo;
    curFrom->addEdgeTo(newPt);
    curTo = newPt;
    value += newPt->getValue();
    
    // Return the alternate graphs that correspond to the extensions of each graph node with pt.
    // The returned set does not include this graph since that would be redundant
    return af.alternateGraphs;
  }
}

class cloneFunctor : public transGraph::GraphEdgeFunctor
{
  aggregate* oldA;
  aggregate* newA;
  
  // The graph that will be a clone of the original
  transGraph* clone;
  
  // Maps nodes in the old graph to corresponding nodes in the new graph
  map<aggregate*, aggregate*> old2New;
  
  public:
  cloneFunctor(transGraph* graph, aggregate* oldA, aggregate* newA): transGraph::GraphEdgeFunctor(graph), oldA(oldA), newA(newA) 
  {
    clone = new transGraph();
    old2New[graph->entry] = clone->entry;
  }
  
  void operator()(aggregate* from, aggregate* to) {
    /*scope s("cloneFunctor()");
    dbg << "from="<<from->str()<<endl;
    dbg << "to="<<to->str()<<endl;*/
    
    // We must have already seen this edge's source
    assert(old2New.find(from) != old2New.end());
    //assert(old2New.find(to)   == old2New.end());
    
    aggregate* newFrom = old2New[from];
    //dbg << "newFrom="<<newFrom->str()<<endl;
    aggregate* newTo;
    
    // If we have not yet seen this edge's target, clone it
    if(old2New.find(to) == old2New.end()) {
      // If the edge's destination is oldA, replace it with newA in the clone
      if(to == oldA) newTo = newA;
      // Otherwise, dynamically allocate a copy of the edge's destination and add an edge to it in the clone graph
      else           newTo = to->copy();
      
      clone->addValue(newTo->getValue());
      old2New[to] = newTo;
    } else
      newTo = old2New[to];

    // Manage edges
    
    // Adjust curFrom/curTo as needed
    if(from == graph->curFrom && to == graph->curTo) {
      clone->curFrom = newFrom;
      clone->curTo   = newTo;
    }

    // Add the from->to edge to the cloned graph
    newFrom->addEdgeTo(newTo);
  }
  
  // Returns the cloned graph
  transGraph* getClone() const
  { return clone; }
};

// Return a clone of this graph, except where oldA is replaced with newA (accounting for any changes in graph value)
transGraph* transGraph::clone(aggregate* oldA, aggregate* newA) {
  cloneFunctor cf(this, oldA, newA);
  mapEdge(cf);
  return cf.getClone();
}

// Apply the given functor to all the nodes in the graph
void transGraph::mapNode(GraphNodeFunctor& f) {
  /*attrIf aif(new attrGE("mapVerbose", (long)1));
  scope s("transGraph::mapNode");*/
  list<aggregate*> worklist;
  set<aggregate*> visited;
  
  // Initialize the worklist to the entry node
  worklist.push_back(entry);
  
  while(worklist.size()>0) {
    //scope sw(txt()<<"#worklist="<<worklist.size());
    
    aggregate* cur = worklist.front(); worklist.pop_front();
    //dbg << "cur="<<cur->str()<<endl;
    f(cur);
    visited.insert(cur);
    
    //dbg << "#cur.next="<<cur->getNext().size()<<endl;
    for(set<aggregate*>::const_iterator n=cur->getNext().begin(); n!=cur->getNext().end(); n++) {
      if(visited.find(*n) == visited.end()) worklist.push_back(*n);
    }
    //dbg << "final: #worklist="<<worklist.size()<<endl;
  }
}

// Apply the given functor to all the nodes in the graph
void transGraph::mapEdge(GraphEdgeFunctor& f) {
  //attrIf aif(new attrGE("mapVerbose", (long)1));
  //scope s("transGraph::mapEdge");
  
  list<pair<aggregate*, aggregate*> > worklist;
  set<pair<aggregate*, aggregate*> > visited;
  
  // Initialize the worklist to all the outgoing edges from the entry node
  //dbg << "Outgoing edges from entry: "<<entry->getNext().size()<<endl;
  for(std::set<aggregate*>::const_iterator n=entry->getNext().begin(); n!=entry->getNext().end(); n++) {
    worklist.push_back(make_pair(entry, *n));
  }
  
  while(worklist.size()>0) {
    //scope sw(txt()<<"#worklist="<<worklist.size());
    // Get the next edge
    pair<aggregate*, aggregate*> cur = worklist.front(); worklist.pop_front();
    
    // Skip this edge if it has already been visited
    if(visited.find(cur) != visited.end()) continue;
    
    //dbg << "from="<<cur.first->str()<<endl;
    //dbg << "to="<<cur.second->str()<<endl;
    f(cur.first, cur.second);
    visited.insert(cur);
    
    // Add the outgoing edges of the current edge's target node to the worklist
    for(set<aggregate*>::const_iterator n=cur.second->getNext().begin(); n!=cur.second->getNext().end(); n++) {
      pair<aggregate*, aggregate*> nextEdge = make_pair(cur.second, *n);
      //dbg << "nextEdge=<"<<nextEdge.first->str()<<" => "<<nextEdge.second->str()<<" visited="<<(visited.find(nextEdge) != visited.end())<<endl;
      // Add this edge if it has not yet been visited
      if(visited.find(nextEdge) == visited.end()) worklist.push_back(nextEdge);
    }
  }
}

double transGraph::getValue() const
{ return value; }

// Adds the given amount to the graph's value
void transGraph::addValue(double inc)
{ value+=inc; }

class node2DOTFunctor : public transGraph::GraphNodeFunctor
{ 
  ostringstream& dotStream;
  public:
  node2DOTFunctor(transGraph* graph, ostringstream& dotStream): transGraph::GraphNodeFunctor(graph), dotStream(dotStream) {}
  
  void operator()(aggregate* a) {
    if(a->getAggrType() == aggregate::noneT) return;
    
    dotStream << "node_"<<a<<" [label=\""<<a->str()<<"\", shape=box];"<<endl;
  }
};

class edge2DOTFunctor : public transGraph::GraphEdgeFunctor
{ 
  ostringstream& dotStream;
  public:
  edge2DOTFunctor(transGraph* graph, ostringstream& dotStream): transGraph::GraphEdgeFunctor(graph), dotStream(dotStream) {}
  
  void operator()(aggregate* from, aggregate* to) {
    if(from->getAggrType() == aggregate::noneT) return;
    if(to->getAggrType() == aggregate::noneT) return;
    
    dotStream << "node_"<<from<<" -> node_"<<to;
    if(from == graph->curFrom && to == graph->curTo)
      dotStream << "[color=red]";
    dotStream <<";"<<endl;
  }
};

// Returns a string that containts the representation of the object as a graph in the DOT language
// that has the given name
std::string transGraph::toDOT(std::string graphName) {
  ostringstream oss;
  oss << "digraph transGraph {"<<endl;
  
  node2DOTFunctor n2d(this, oss);
  mapNode(n2d);
  
  edge2DOTFunctor e2d(this, oss);
  mapEdge(e2d);

  oss << " }";
  return oss.str();
}

/*******************
 ***** pattern *****
 *******************/

pattern::pattern(int maxAlts): maxAlts(maxAlts) {
  assert(maxAlts>0);
  
  transGraph* initGraph = new transGraph();
  alt.insert(make_pair(initGraph->getValue(), initGraph));
  
  dbg << "initGraph:";
  graph::genGraph(*initGraph);
}

void pattern::add(const std::vector<int>& pt) {
  scope pa(txt()<<"pattern::add("<<col2Str(pt)<<"), #alt="<<alt.size());
  // The new set of alternate graphs that will be produced as a result of adding this point
  multimap<double, transGraph*> newAlt;
  
  // Add this point to all the current alternate graphs
  for(multimap<double, transGraph*>::iterator g=alt.begin(); g!=alt.end(); g++) {
    scope s(txt() << "Adding to graph (value="<<g->second->getValue()<<"):"); graph::genGraph(*(g->second));
    
    set<transGraph*> additions = g->second->add(pt);
    
    dbg << "Base Alternate (value="<<g->second->getValue()<<"):"; graph::genGraph(*(g->second));
    newAlt.insert(make_pair(g->second->getValue(), g->second));
    
    for(set<transGraph*>::iterator a=additions.begin(); a!=additions.end(); a++) {
      dbg << "Extended Alternate (value="<<(*a)->getValue()<<"):"; graph::genGraph(*(*a));
      newAlt.insert(make_pair((*a)->getValue(), *a));
    }
  }
  
  // Update alt to include the new alternate graphs
  alt = newAlt;
  
  // If there are too many alternate graphs, remove those with the smallest value
  while(alt.size() > maxAlts) { 
    dbg << "Removing (value="<<alt.begin()->first<<"):"; graph::genGraph(*(alt.begin()->second));
    double lowestVal = alt.begin()->first;

//!    delete alt.begin()->second;
    alt.erase(alt.begin());
  }
}

void pattern::report() {
  scope s("Patterns");
  for(multimap<double, transGraph*>::iterator g=alt.begin(); g!=alt.end(); g++) {
    dbg << "Value "<<g->first<<endl;
    graph::genGraph(*(g->second));
  }
}

int main(int argc, char** argv) {
  SightInit(argc, argv);
  //attr mapVerbose("mapVerbose", (long)1);
  
  pattern p(2);
  for(int i=0; i<2; i++) {
    scope s(txt()<<"Adding i="<<i, scope::medium);
    for(int j=i; j<5; j++) {
      scope s(txt()<<"Adding i="<<i<<" j="<<j, scope::medium);
      vector<int> v; v.push_back(j);
      p.add(v);
    }
  }
  p.report();
}
