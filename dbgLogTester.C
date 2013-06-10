#include "dbglog.h"

using namespace std;
using namespace dbglog;

int fibscope(int a, scope::scopeLevel level, int verbosityLevel);
int fibIndent(int a, int verbosityLevel);

class dottableExample: public dottable
{
  public:
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  std::string toDOT(std::string graphName) {
    ostringstream oss;
    oss << "graph ethane {"<<endl;
    oss << "     C_0 -- H_0 [type=s];"<<endl;
    oss << "     C_0 -- H_1 [type=s];"<<endl;
    oss << "     C_0 -- H_2 [type=s];"<<endl;
    oss << "     C_0 -- C_1 [type=s];"<<endl;
    oss << "     C_1 -- H_3 [type=s];"<<endl;
    oss << "     C_1 -- H_4 [type=s];"<<endl;
    oss << "     C_1 -- H_5 [type=s];"<<endl;
    oss << " }";
    return oss.str();
  }
};

int main(int argc, char** argv)
{
  // It is possible to write arbitrary text to the debug output
  dbg << "Welcome to the dbgLogTester" << endl;
  
  // The emitted text can include arbitrary HTML.
  // Note that erroneous HTML, such as the use of '<' and '>' can produce erroneous debug output
  // and possibly hide messages.
  for(int i=1; i<=4; i++)
    dbg << "<h"<<i<<">This text is an H"<<i<<" header</h"<<i<<">";
  
  // Text can be indented as much as needed
  {
    indent indA;
    dbg << "This text was indented."<<endl;
  }
  dbg << "No indentation here"<<endl;
  
  // Any text can be used for indentation
  indent indB("###");
  dbg << "All subsequent text will have hashes prepended"<<endl;
  dbg << "Such as here\n";
  dbg << "And here"<<endl;
  
  // Here we see recursive function calls (recursive Fibonacci) that add more indentation at deeper levels of recursion
  {
    scope regFibIndent("Indentation due to recursive calls to fib");
    fibIndent(3, 0);
  }
  
  {
    scope regFibIndent("Indentation due to recursive calls to fib, 1 level of indent");
    fibIndent(3, 3);
  }
  
  // In addition to structuring output via indentation it is possible to organize it via visually distinct scopes

  // This creates a high-level scope that lasts only during the lexical scope of object regHigh.
  // A high-level scope creates a separate file and links that make it possible to load the file's
  // context into the parent HTML file or to open the file in a new tab or window.
  {
    scope regHigh("We've entered a top-level scope", scope::high);
    
    // This is a medium-level scope (default) that occurs inside the high-level scope. It also lasts until
    // the end of the scope of object regMed. This object's label is generated using the << syntax, which
    // is used by placing a label object at the left of all the <<'s.
    scope regMid(label("This is") << " a lower " << "level scope");
    
    // Write some text into this mid-level scope
    for(int i=0; i<5; i++)
      dbg << "i="<<i<<endl;
  }
  
  // Call the fibscope function, which generates a hierarchy of mid-level scopes, on
  {
    scope regFibIndent("Nested scopes due to recursive calls to fib");
    dbg << "<u>Medium level scopes, colors change</u>"<<endl;
    fibscope(4, scope::medium, 0);
  }
  
  // Call the fib function, which generates a single mid-level scope for a=6
  {
    scope regFibIndent("Nested scopes due to recursive calls to fib, 2 level of scope hierarchy");
    dbg << "<u>Low level scopes, colors do not change</u>"<<endl;
    fibscope(6, scope::low, 5);
  }
  
  // Dot graph
  dbg << "It is possible to generate dot graphs that describe relevant aspects of the code state.";
  
  {
    scope dotStr("This graph was generated from a string:", scope::medium);
    string imgPath = addDOT(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
    dbg << "imgPath=\""<<imgPath<<"\"\n";
  }
  
  {
    scope dotStr("This graph was generated from a dottable object:", scope::medium);
    dottableExample ex;
    string imgPath = addDOT(ex);
    dbg << "imgPath=\""<<imgPath<<"\"\n";
  }
  
  return 0;
}

int fibscope(int a, scope::scopeLevel level, int verbosityLevel) {
  // Each recursive call to fibscope generates a new mid-level scope. To reduce the amount of text printed, we only 
  // generate scopes if the value of a is >= verbosityLevel
  scope reg(label()<<"fib("<<a<<")", level, a, verbosityLevel);
  
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibscope(a-1, level, verbosityLevel) + fibscope(a-2, level, verbosityLevel);
    dbg << "="<<val<<endl;
    return val;
  }
}

int fibIndent(int a, int verbosityLevel) {
  // Each recursive call to fibscope adds an indent level, prepending ":" to text printed by deeper calls to fibIndent. 
  // To reduce the amount of text printed, we only add indentation if the value of a is >= verbosityLevel
  indent ind(": ", a, verbosityLevel);
  
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibIndent(a-1, verbosityLevel) + fibIndent(a-2, verbosityLevel);
    dbg << "="<<val<<endl;
    return val;
  }
}

