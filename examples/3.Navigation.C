#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;


int fibScopeLinks(int a, scope::scopeLevel level, list<int>& stack, 
                  map<list<int>, anchor>& InFW, 
                  map<list<int>, anchor>& InBW, 
                  map<list<int>, anchor>& OutFW, 
                  map<list<int>, anchor>& OutBW,
                  bool doFWLinks);

// Fibonacci, where we create graph edges from each call to its children.
int fibGraph(int a, graph& g, anchor* parent);

int main(int argc, char** argv)
{
  SightInit(argc, argv, "3.Navigation", "dbg.3.Navigation");

  dbg << "<h1>Example 3: Navigation</h1>" << endl;
  
  dbg << "The order of the log text depends on the iteration order of the application being debugged and not "<<
         "its logical structure. In cases where the log grows long it is very valuable to be able to add "<<
         "a way to view the logical relationships between distant locations in the log and navigate easily from "<<
         "one to another."<<endl;
  
  {
    scope s("Computing primes with no support for navigation", scope::high);
    dbg << "Note: click on the title link of this scope to minimize it when done viewing."<<endl;
    
    int totalNums=30;
    // Holds the prime status of each number. Initially all numbers are considered to be prime.
    bool* notPrime = (bool*)calloc(sizeof(bool), totalNums);
    
    // Iterate through all numbers, eliminating their multiples as being definitely not prime
    for(int i=2; i<totalNums; i++) {
      scope s(txt()<<"Eliminating multiples of "<<i);
      for(int j=i; j<totalNums; j+=i) {
        dbg << j << " is not prime"<<endl;
        notPrime[j] = true;
      }
    }
  }
  
  
  {
    scope s("We make output more navigable by linking related outer iterations", scope::high);
    dbg << "Note: click on the title link of this scope to minimize it when done viewing."<<endl;
    
    int totalNums=30;
    // Holds the prime status of each number. Initially all numbers are considered to be prime.
    bool* notPrime = (bool*)calloc(sizeof(bool), totalNums);
    
    // Maps each iteration number to the link anchors that will point to this iteration's scope
    map<int, set<anchor> > pointsTo;
    // Maps each iteration number fo the link anchors of the factors of this number
    map<int, set<pair<int, anchor> > > factorAnchors;
    
    // Iterate through all numbers, eliminating their multiples as being definitely not prime
    for(int i=2; i<totalNums; i++) {
      // Create iteration i's scope, anchoring all incoming links to it
      scope si(txt()<<"Eliminating multiples of "<<i, pointsTo[i]);
      
      // Link backwards to all the factors of i
      {
        scope sFac(txt()<<"Factors of "<<i, scope::low);
        for(set<pair<int, anchor> >::iterator a=factorAnchors[i].begin(); a!=factorAnchors[i].end(); a++)
        { dbg << a->first<<" "; a->second.linkImg(); }
      }
      
      // We can now erase all the anchors that refer to this scope from pointsTo[] since we've successfully terminated them.
      pointsTo[i].clear(); 
      
      for(int j=i*2; j<totalNums; j+=i) {
        // This is an anchor that will be anchored at iteration j of the outer loop once we reach it.
        anchor toAnchor;
        pointsTo[j].insert(toAnchor);
        
        // Record the anchor of iteration i's scope in factorAnchors so that iteration j can link back to it
        factorAnchors[j].insert(make_pair(i, si.getAnchor()));

        dbg << j << " is not prime. ";
        notPrime[j] = true;
        
        // Add a forward link to iteration that considers the number we've just shown is not a prime
        toAnchor.linkImg("Iteration of invalidated prime"); dbg << endl;
      }
    }
  }
  
  {
    scope s("A graph can summarize structure at a high level", scope::high);
    dbg << "Note: click on the title link of this scope to minimize it when done viewing."<<endl;
    
    // The graph that will summarize structure and will be shown at this point in the log output
    graph g(true);
    
    int totalNums=30;
    // Holds the prime status of each number. Initially all numbers are considered to be prime.
    bool* notPrime = (bool*)calloc(sizeof(bool), totalNums);
    
    // Maps each iteration number to the link anchors that will point to this iteration's scope
    map<int, set<anchor> > pointsTo;
    
    // Iterate through all numbers, eliminating their multiples as being definitely not prime
    for(int i=2; i<totalNums; i++) {
      // Create iteration i's scope, anchoring all incoming links to it
      scope si(txt()<<i, pointsTo[i]);

      // We can now erase all the anchors that refer to this scope from pointsTo[] since we've successfully terminated them.
      pointsTo[i].clear(); 
      
      for(int j=i*2; j<totalNums; j+=i) {
        // This is an anchor that will be anchored at iteration j of the outer loop once we reach it.
        anchor toAnchor;
        pointsTo[j].insert(toAnchor);
        
        // Add a graph edge from the current scope to the iteration of the number proven to be not prime
        g.addDirEdge(si.getAnchor(), toAnchor);
        
        dbg << j << " is not prime."<<endl;
        notPrime[j] = true;
      }
    }
  }
  
  // Call the fibScopeLinks function, which generates two hierarchies of high-level scopes with scopes
  // in each level of one hierarchy linking to the same level in the other hierarchy
  {
    scope s("Multiple connected fibonacci recursion trees", scope::high);
    
    dbg << "Links work both within a file and across them. In this example we call Fibonacci(5) twice, with each "<<
           "recursive call in a different high-level scope (and thus file). Further, we create a link from each "<<
           "recursive sub-call to Fibonacci in one one recursion tree to the corresponding sub-call in the other."<<
           "By clicking on such a link it is possible to navigate across related portions of the two recursion trees, "<<
           "loading sub-files as needed."<<endl;
    
    list<int> stack;
    map<list<int>, anchor> InFW, InBW, OutFW, OutBW;
    fibScopeLinks(5, scope::high, stack, InFW, InBW, OutFW, OutBW, true);
    assert(stack.size()==0);
    map<list<int>, anchor> OutBW2, OutFW2;
    fibScopeLinks(5, scope::high, stack, OutFW, OutBW, OutBW2, OutFW2, false);
  }
  
  // Call a recursive Fibonacci, where we create a graph of the recursion hierarchy
  {
    scope s("Recursive Fibonacci", scope::high);
    graph g;
    
    fibGraph(5, g, NULL);
  }
    
  return 0;
}


// InFW links: links from prior fib call to this one. These need to be anchored to the regions in this next.
// InBW links: links from this fib call nest to the prior one. These have already been anchored to the prior nest.
// OutBW links: links from the next nest to this one. These are anchored to established regions.
// OutFW links: links from this nest to the next one. These are un-anchored and will be anchored to the next nest
int fibScopeLinks(int a, scope::scopeLevel level, list<int>& stack, 
                  map<list<int>, anchor>& InFW, 
                  map<list<int>, anchor>& InBW, 
                  map<list<int>, anchor>& OutFW,
                  map<list<int>, anchor>& OutBW,
                  bool doFWLinks) {
  stack.push_back(a); // Add this call to stack
  
  // Each recursive call to fibScopeLinks generates a new scope at the desired level. To reduce the amount of text printed, we only 
  // generate scopes if the value of a is >= verbosityLevel
  scope reg(txt()<<"fib("<<a<<")", 
            (InFW.find(stack)!=InFW.end()? InFW[stack]: anchor::noAnchor),
            level);
  
  OutBW[stack] = reg.getAnchor();
  
  if(a==0 || a==1) { 
    dbg << "=1."<<endl;
    if(doFWLinks) {
      anchor fwLink;
      fwLink.linkImg("Forward link"); dbg << endl;
      OutFW[stack] = fwLink;
    }
    if(InBW.find(stack)!=InBW.end())
    { InBW[stack].linkImg("Backward link"); dbg<<endl; }
    
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
    if(InBW.find(stack)!=InBW.end())
    { InBW[stack].linkImg("Backward link"); dbg<<endl; }
    
    //cout << "link="<<dbg.linkTo(linkScopes[stack], "go")<<endl;
    stack.pop_back(); // Remove this call from stack
    return val;
  }
}

// Fibonacci, where we create graph edges from each call to its children.
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

