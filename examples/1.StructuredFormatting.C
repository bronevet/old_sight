#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

/*#include "../utils.h"
//#include "../sight_structure_internal.h"
#include "../widgets/graph_structure.h"
#include "../widgets/scope_structure.h"
//#include "../widgets/valSelector_structure.h"
//#include "../widgets/trace_structure.h"

using namespace sight::structure;
*/

int fibIndent(int a);
int fibScope(int a, scope::scopeLevel level);
  
class dottableExample: public dottable
{
  public:
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  std::string toDOT(std::string graphName);
};
  
int main(int argc, char** argv)
{
  SightInit(argc, argv, "1.StructuredFormatting", "dbg.1.StructuredFormatting");
   
  // It is possible to write arbitrary text to the debug output
  dbg << "<h1>Example 1: Structured Formatting</h1>" << endl;
  
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
  
  dbgprintf("<b>We can use both the C++ &lt;&lt;-style output via the dbg stream or use dbgprintf() for a C-style interface</b>\n");
  
  // Here we see recursive function calls (recursive Fibonacci) that add more indentation at deeper levels of recursion
  {
    scope regFibIndent("Indentation due to recursive calls to fib");
    fibIndent(3);
  }
  
  // In addition to structuring output via indentation it is possible to organize it via visually distinct scopes

  // This creates a high-level scope that lasts only during the lexical scope of object regHigh.
  // A high-level scope creates a separate file and links that make it possible to load the file's
  // context into the parent HTML file or to open the file in a new tab or window.
  {
    scope regHigh("This is a high-level scope, the contents of which are loaded by clicking the down arrow", scope::high);
    
    // This is a medium-level scope (default) that occurs inside the high-level scope. It also lasts until
    // the end of the scope of object regMed. This object's label is generated using the << syntax, which
    // is used by placing a txt object at the left of all the <<'s.
    scope regMid("This is a deeper scope");
    
    // Write some text into this mid-level scope
    for(int i=0; i<5; i++)
      dbg << "i="<<i<<endl;
  }
  
  // Call the fibScope function, which generates a hierarchy of mid-level scopes, on
  {
    scope regFibIndent("Nested medium-level scopes due to recursive calls to fib");
    dbg << "<u>In medium-level scopes, colors change</u>"<<endl;
    fibScope(3, scope::medium);
  }
  
  dbg << "The left frame summarizes the structure of the main debug output. "<<
         "It contains a single entry for each visual block. Clicking on a link "<<
         "in the summary frame moves the focus in the text frame to the corresponding "<<
         "block. Similarly, clicking in a block in the text pane moves the focus "<<
         "in the summary pane to the corresponding entry. The blocks where your "<<
         "mouse is currently located have a black border and the corresponding "<<
         "entries in the summary frame are highlighted (try it out by moving the "<<
         "mouse around!"<<endl;
  
  // Call the fib function, with low-level scopes
  {
    scope regFibIndent("Nested low-level scopes");
    dbg << "<u>In low-level scopes, colors change</u>"<<endl;
    fibScope(2, scope::low);
  }
  
  // Call the fib function, with min-level scopes
  {
    scope regFibIndent("Nested min-level scopes");
    dbg << "<u>Min-level scopes don't have formatted titles and do not change colors</u>"<<endl;
    fibScope(2, scope::minimum);
  }
  
  {
    scope txtExplanation("String Creation");
    dbg << "One difficulty of scopes and indents is that they only accept strings as arguments. "<<
           "The string + operator and sprintf() are a little more cumbersome for creating complex "<<
           "formatted text as compared to the C++ &lt;&lt; operator. As such sight provides the txt() "<<
           "object, which allows users to use &lt;&lt; formatting in places where strings are required."<<endl;
    for(int i=0; i<3; i++) {
      scope s(txt()<<"Scope "<<i);
      dbg << "The names of these scopes combine a string and an integer using the &lt;&lt; operator."<<endl;
    }
  }
  
  {
    scope scopeGraphs("Graph Viewing");
    
    dbg << "sight allows applications to visualize information in terms of dot graphs. "<<
           "These graphs are laid out using the graphviz dot renderer and presented in line "<<
           "with the rest of the debug text."<<endl;
    
    {
      scope dotStr("This graph was generated from a string");
      graph::genGraph(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
    }
    
    {
      scope dotStr("This graph was generated from an object that extend the dottable class");
      dottableExample ex;
      graph::genGraph(ex);
    }
  }
  
  
  return 0;
}


// Returns a string that containts the representation of the object as a graph in the DOT language
// that has the given name
std::string dottableExample::toDOT(std::string graphName) {
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

// Each recursive call to fibScope() generates a new scope at the desired level. 
// The scope level that is passed in controls the types of scopes that are recursively created
int fibScope(int a, scope::scopeLevel level) {
  scope reg(txt()<<"fib("<<a<<")", level);
  
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


