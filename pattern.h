#include <iostream>
#include <set>
#include <vector>
#include <list>
#include <map>
#include <typeinfo>
#include <string>
#include "sight.h"

const int low=1;
const int high=10;

class aggregate
{
  // Successors in the transition graph
  std::set<aggregate*> next;
  
  // Predecessors in the transition graph
  std::set<aggregate*> pred;
  
  // Dimension of this aggregate
  int dim;
  
  // The lower-dimension aggregates that are contained inside this aggregatee
  std::list<aggregate*> sub;
  
  // The number of points this aggregate represents
  int numPts;

  public:  
  // The different sub-types of aggregates
  typedef enum {noneT, pointT, lineT} aggrType;
  
  protected:
  // When multiple aggregates are merged to create higher-dimensional aggregates it is important
  // to remember the type of the lower-dimensional aggregates they contain. We do this by maintaining
  // inside each higher-dimension aggregate a stack of aggrTypes of the aggregates it is made from.
  std::vector<aggrType> intStack;
  
  public:
  aggregate(int dim, int numPts) : dim(dim), numPts(numPts) {}
  aggregate(int dim, int numPts, const std::vector<aggrType>& intStack) : dim(dim), numPts(numPts), intStack(intStack) {}
  aggregate(const aggregate& that) : dim(that.getDim()), numPts(that.getNumPts()), intStack(that.getIntStack()) {}
  
  // Adds newAggr to this aggregate, returning the set of all the possible extensions of this aggregate with newAggr,
  // all of which are dynamically allocated. This is a base class method that should be called and in turn calls the
  // constructors and extend methods of derived classes.
  std::set<aggregate*> add(const aggregate& newAggr);
  
  protected:
  // Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
  // all of which are dynamically allocated.
  virtual std::set<aggregate*> extend(const aggregate& newAggr)=0;
  
  public:
  
  // Returns whether these two aggregates have the same descriptors
  static bool sameDescriptors(const aggregate& aggr1, const aggregate& aggr2);
  
  // Returns a point vector that describes this aggregate
  virtual const std::vector<int>& getDescriptor() const=0;
  
  // Returns the dimension of this aggregate
  int getDim() const { return dim; }
  
  // Returns the number of points denoted by this aggregate
  int getNumPts() const { return numPts; }
  
  // Sets the number of points in this aggregate
  void setNumPts(int n) { numPts = n; }
  
  const std::set<aggregate*>& getNext() const { return next; }
    
  const std::set<aggregate*>& getPred() const { return pred; }
  
  
  static std::string aggrType2Str(aggrType t);
  
  // Returns the type of aggregate this is
  virtual aggrType getAggrType() const=0;
  
  public:
  const std::vector<aggrType>& getIntStack() const { return intStack; }
    
  std::string intStack2Str() const;
  
  // Returns the value of using this aggregate to summarize its points
  virtual double getValue() const=0;
  
  // Returns a dynamically-allocated copy of this aggregate
  virtual aggregate* copy() const=0;
  
  // Graph functions
  
  // Add an edge from this aggregate to the given aggregate.
  void addEdgeTo(aggregate* to);
  
  // Remove an edge from this aggregate to the given aggregate, if one exists
  void rmEdgeTo(aggregate* to);
  
  // Returns a human-readable string representation of this aggregate. Used for debugging.
  virtual std::string str() const=0;
    
  // Returns a human-readable string representation of this aggregate's loop structure. 
  // Used for generating output to users.
  // Takes as input a vector of strings that denote the variables/constants defined by higher 
  // levels of the loop structure that will control the parameters of this aggregate.
  virtual std::string loopStr(std::vector<std::string> ctrl) const=0;
};

// The entry node into the graph. Has no functionality other than maintaining outgoing graph edges.
class entryNode: public aggregate
{
  protected:
  // Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
  // all of which are dynamically allocated.
  std::set<aggregate*> extend(const aggregate& newAggr) { return std::set<aggregate*>(); }
  
  public:
  entryNode(): aggregate(0, 0) {}
    
  std::vector<int> empty;
  // Returns a point vector that describes this aggregate
  const std::vector<int>& getDescriptor() const { return empty; }
  
  // Returns the type of aggregate this is
  aggrType getAggrType() const { return noneT; }
    
  // Returns the value of using this aggregate to summarize its points
  double getValue() const { return 0; }
  
  // Returns a dynamically-allocated copy of this aggregate
  aggregate* copy() const { return new entryNode(); }
  
  // Returns a human-readable string representation of this aggregate
  std::string str() const { return "entryNode"; }
    
  // Returns a human-readable string representation of this aggregate's loop structure. 
  // Used for generating output to users.
  // Takes as input a vector of strings that denote the variables/constants defined by higher 
  // levels of the loop structure that will control the parameters of this aggregate.
  std::string loopStr(std::vector<std::string> ctrl) const { return ""; }
};

class point: public aggregate
{
  std::vector<int> pt;
  
  public:
  point(const aggregate& newAggr);
  point(const aggregate& aggr1, const aggregate& aggr2);
  
  point(const std::vector<int>& pt, int numPts=1);
  
  // Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
  // all of which are dynamically allocated.
  std::set<aggregate*> extend(const aggregate& newAggr);
  
  // Returns whether these two aggregates can be grouped into a valid point
  bool validAggregate(const aggregate& aggr1, const aggregate& aggr2);
  
  // Returns a point vector that describes this aggregate
  const std::vector<int>& getDescriptor() const;
  
  // Returns the type of aggregate this is
  aggrType getAggrType() const { return pointT; }
  
  // Returns the value of using this aggregate to summarize its points
  double getValue() const;
  
  // Returns a dynamically-allocated copy of this aggregate
  aggregate* copy() const;
  
  // Returns a human-readable string representation of this aggregate
  std::string str() const;
    
  // Returns a human-readable string representation of this aggregate's loop structure. 
  // Used for generating output to users.
  // Takes as input a vector of strings that denote the variables/constants defined by higher 
  // levels of the loop structure that will control the parameters of this aggregate.
  std::string loopStr(std::vector<std::string> ctrl) const;
};

class line: public aggregate
{
  // Vector that contains the line's slope and intercepts
  std::vector<int> slope;
  std::vector<int> first;
  
  // The descriptor of this line
  std::vector<int> descriptor;
  
  public:
  line(const aggregate& newAggr1, const aggregate& newAggr2);
  line(const std::vector<int>& first, const std::vector<int>& slope, int dim, int numPts, const std::vector<aggrType>& intStack);
  line(const line& that);
  
  // Returns a set of all the possible extensions of this aggregate with the points in the given aggregate, 
  // all of which are dynamically allocated.
  std::set<aggregate*> extend(const aggregate& newAggr);
    
  // Returns a point vector that describes this aggregate
  const std::vector<int>& getDescriptor() const;
  
  // Returns the type of aggregate this is
  aggrType getAggrType() const { return lineT; }
  
  // Returns the value of using this aggregate to summarize its points
  double getValue() const;
  
  // Returns a dynamically-allocated copy of this aggregate
  aggregate* copy() const;
  
  // Returns a human-readable string representation of this aggregate
  std::string str() const;
    
  // Returns a human-readable string representation of this aggregate's loop structure. 
  // Used for generating output to users.
  // Takes as input a vector of strings that denote the variables/constants defined by higher 
  // levels of the loop structure that will control the parameters of this aggregate.
  std::string loopStr(std::vector<std::string> ctrl) const;
};

class transGraph: public sight::dottable
{
  double value;
  
  public:
  // The graph's entry node
  aggregate* entry;
  
  // The current graph edge
  aggregate* curFrom;
  aggregate* curTo;
  
  transGraph();
  ~transGraph();
  
  // Functor that can be applied to graph nodes 
  class GraphNodeFunctor
  {
    protected:
    transGraph* graph;
    public:
    GraphNodeFunctor(transGraph* graph) : graph(graph) {}
      
    virtual void operator()(aggregate* a)=0;
  };
  
  // Functor that can be applied to graph edges
  class GraphEdgeFunctor
  {
    protected:
    transGraph* graph;
    public:
    GraphEdgeFunctor(transGraph* graph) : graph(graph) {}
      
    virtual void operator()(aggregate* from, aggregate* to)=0;
  };
  
  // Add a new point to the graph.
  // This graph is adjusted such that the new point is a new node. Further,
  // the function returns a set of alternate graphs where the new point is 
  // merged into an existing graph node.
  std::set<transGraph*> add(const std::vector<int>& pt);
  
  // Return a clone of this graph, except where oldA is replaced with newA (accounting for any changes in graph value)
  transGraph* clone(aggregate* oldA, aggregate* newA);
  
  // Apply the given functor to all the nodes in the graph
  void mapNode(GraphNodeFunctor& f);
  
  // Apply the given functor to all the nodes in the graph
  void mapEdge(GraphEdgeFunctor& f);
  
  // Returns the aggregate value of all the nodes in the graph
  double getValue() const;
  
  // Adds the given amount to the graph's value
  void addValue(double inc);
  
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  std::string toDOT(std::string graphName);
};

class pattern
{
  private:
  // The maximum number of alternatives that may be maintained at one time
  int maxAlts;
  
  // Records all alternative transition graphs that are currently considered to be 
  // possible as a mapping from their values to their pointers
  std::multimap<double, transGraph*> alt;
  
  public:
  pattern(int maxAlts);
    
  void add(const std::vector<int>& pt);
    
  void report();
};
