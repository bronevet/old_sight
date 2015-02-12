#include "sight_api.h"
#include "stdio.h"

int fibIndent(int a);
int fibScope(int a, scope_level level);

  
int main(int argc, char** argv)
{
  SightInit(argc, argv, "1.StructuredFormatting", "dbg.1.StructuredFormatting");
   
  // It is possible to write arbitrary text to the debug output
  _dbg("<h1>Example 1: Structured Formatting</h1>");
  
  // The emitted text can include arbitrary HTML.
  // Note that erroneous HTML, such as the use of '<' and '>' can produce erroneous debug output
  // and possibly hide messages.
  int i;
  for(i=1; i<=4; i++)
      _dbgprintf("<h%d>This text is an H%d header</h%d>",i, i, i);
  
  // Text can be indented as much as needed
  {
    void* indA = _indent();
    _dbg("This text was indented.");
    _indent_out(indA);
  }
  _dbg("No indentation here");
  
  // Any text can be used for indentation
  void* indB = _indent2("###");
  _dbg("All subsequent text will have hashes prepended");
  _dbg("Such as here\n");
  _dbg("And here");
  
  _dbgprintf("<b>We can use both the C++ &lt;&lt;-style output via the dbg stream or use dbgprintf() for a C-style interface</b>\n");
  
  // Here we see recursive function calls (recursive Fibonacci) that add more indentation at deeper levels of recursion
  {
    void* regFibIndent= _scope("Indentation due to recursive calls to fib");
    fibIndent(3);
      _scope_out(regFibIndent);
  }
  
  // In addition to structuring output via indentation it is possible to organize it via visually distinct scopes

  // This creates a high-level scope that lasts only during the lexical scope of object regHigh.
  // A high-level scope creates a separate file and links that make it possible to load the file's
  // context into the parent HTML file or to open the file in a new tab or window.
  {
    void* regHigh= _scope_l("This is a high-level scope, the contents of which are loaded by clicking the down arrow", high);
    
    // This is a medium-level scope (default) that occurs inside the high-level scope. It also lasts until
    // the end of the scope of object regMed. This object's label is generated using the << syntax, which
    // is used by placing a txt object at the left of all the <<'s.
    void* regMid = _scope("This is a deeper scope");
    
    int i ;
    // Write some text into this mid-level scope
    for(i=0; i<5; i++)
        _dbgprintf("i=%d\n",i);

      _scope_out(regMid);
      _scope_out(regHigh);
  }
  
  // Call the fibScope function, which generates a hierarchy of mid-level scopes, on
  {
    void* regFibIndent = _scope("Nested medium-level scopes due to recursive calls to fib");
    _dbg("<u>In medium-level scopes, colors change</u>\n");
    fibScope(3, medium);

    _scope_out(regFibIndent);
  }
  
  _dbg("The left frame summarizes the structure of the main debug output. \
         It contains a single entry for each visual block. Clicking on a link  \
         in the summary frame moves the focus in the text frame to the corresponding  \
         block. Similarly, clicking in a block in the text pane moves the focus \
         in the summary pane to the corresponding entry. The blocks where your \
         mouse is currently located have a black border and the corresponding \
         entries in the summary frame are highlighted (try it out by moving the \
         mouse around!");
  
  // Call the fib function, with low-level scopes
  {
    void* regFibIndent= _scope("Nested low-level scopes");
    _dbg("<u>In low-level scopes, colors change</u>");
    fibScope(2, low);

    _scope_out(regFibIndent);
  }
  
  // Call the fib function, with min-level scopes
  {
    void* regFibIndent = _scope("Nested min-level scopes");
    _dbg("<u>Min-level scopes don't have formatted titles and do not change colors</u>");
    fibScope(2, minimum);

    _scope_out(regFibIndent);
  }
  
  {
    void* txtExplanation= _scope("String Creation");
    _dbg("One difficulty of scopes and indents is that they only accept strings as arguments. \
           The string + operator and sprintf() are a little more cumbersome for creating complex \
           formatted text as compared to the C++ &lt;&lt; operator. As such sight provides the txt() \
           object, which allows users to use &lt;&lt; formatting in places where strings are required.");
     int i;
     for(i=0; i<3; i++) {
        char str[100];
        snprintf(str, 100, "Scope %d", i);
        void* s = _scope(str);
        _dbg("The names of these scopes combine a string and an integer using the &lt;&lt; operator.");
        _scope_out(s);
    }

    _scope_out(txtExplanation);
  }
  
//  {
//    scope scopeGraphs("Graph Viewing");
//
//    _dbg << "sight allows applications to visualize information in terms of dot graphs. "<<
//           "These graphs are laid out using the graphviz dot renderer and presented in line "<<
//           "with the rest of the debug text."<<endl;
//
//    {
//      scope dotStr("This graph was generated from a string");
//      graph::genGraph(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
//    }
//
//    {
//      scope dotStr("This graph was generated from an object that extend the dottable class");
//      dottableExample ex;
//      graph::genGraph(ex);
//    }
//  }
  
  
  return 0;
}


//// Returns a string that containts the representation of the object as a graph in the DOT language
//// that has the given name
//std::string dottableExample::toDOT(std::string graphName) {
//  ostringstream oss;
//  oss << "graph ethane {"<<endl;
//  oss << "     C_0 -- H_0 [type=s];"<<endl;
//  oss << "     C_0 -- H_1 [type=s];"<<endl;
//  oss << "     C_0 -- H_2 [type=s];"<<endl;
//  oss << "     C_0 -- C_1 [type=s];"<<endl;
//  oss << "     C_1 -- H_3 [type=s];"<<endl;
//  oss << "     C_1 -- H_4 [type=s];"<<endl;
//  oss << "     C_1 -- H_5 [type=s];"<<endl;
//  oss << " }";
//  return oss.str();
//}

int fibIndent(int a) {
  void* ind = _indent2(":  ");
  int val ;
  if(a==0 || a==1) { 
    _dbg("=1");
    val =  1;
  } else {
    val = fibIndent(a-1) + fibIndent(a-2);
    char str[100];
    snprintf(str, 100, "=%d", val);
    _dbg(str);
  }

  _indent_out(ind);
  return val;
}

// Each recursive call to fibScope() generates a new scope at the desired level. 
// The scope level that is passed in controls the types of scopes that are recursively created
int fibScope(int a, scope_level level) {
    char str[100];
    snprintf(str, 100, "fib(%d)", a);
    void *reg = _scope_l(str, level);//(txt()<<"fib("<<a<<")", level);

    int val;
    if (a == 0 || a == 1) {
        _dbg("=1");
        val = 1;
    } else {
        val = fibScope(a - 1, level) +
                fibScope(a - 2, level);
        char tmp[100];
        snprintf(tmp, 100, "=%d", val);
        _dbg(tmp);
    }
    _scope_out(reg);
    return val;
}


