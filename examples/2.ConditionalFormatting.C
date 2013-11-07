#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

int fibConditionalIndent(int a);

int main(int argc, char** argv)
{
  SightInit(argc, argv, "2.ConditionalFormatting", "dbg.2.ConditionalFormatting");
   
  dbg << "<h1>Example 2: Conditional Formatting</h1>" << endl;
  
  dbg << "We can control whether a scope is shown or not by setting an attribute and then checking its value in the scope's contructor."<<endl;
  {
    attr choice("scopeChoice", "yes");
    scope s("Scope is enabled by setting an attribute", attrEQ("scopeChoice", "yes"));
    dbg << "Text inside the enabled scope"<<endl;
  }
  
  {
    attr choice("scopeChoice", "no");
    scope s("Scope is disabled by setting an attribute", attrEQ("scopeChoice", "yes"));
    dbg << "Text inside the disabled scope"<<endl;
  }
  
  dbg << "Indentation can also be controlled by attributes. For example, here we invoke recusive Fibonacci multiple times, "<<
         "each time deisabling indentation at higher and higher levels of the recursion hierarchy."<<endl;
  {
    scope s("The amount of indentation is controlled by a verbosity attribute");
    for(int verb=3; verb>=1; verb--) {
      scope s(txt()<<"Indentation due to recursive calls to fib. Verbosity="<<verb);
      dbg << "The attribute verbosity is set while the verbA variable is in-scope. Indents inside fibConditionalIndent() "<<
             "query its value to determine whether to perform the indent or not."<<endl;
      attr verbA("verbosity", verb);
      fibConditionalIndent(3);
    }
  }
  
  dbg << "In addition to controlling scopes and indents we can also use attributes to enable/disable all output at will."<<endl;
  
  {
    dbg << "First we set some attributes a==&gt;1, b==&gt;2, c==>&gt;\"cValue\""<<endl;
    attr a("a", 1);
    attr b("b", 2);
    attr c("c", "cValue");
    
    dbg << "We can now create logical expressions based on these attributes. Text is only printed if the last expression that "<<
           "was declared in the current scope evaluates to true."<<endl;
    
    dbg << "attrIf evaluates to true if the operation given to it evaluates to true."<<endl;
    {
      attrIf aI(new attrEQ("a", 1));
      dbg << "This text is printed out because a==1"<<endl;
      
      {
        dbg << "attrAnd takes the expression that is currently in-scope and ANDs another operation to it."<<endl;
        attrAnd aA(new attrGT("b", 0));
        scope s("This scope is created only if a==1 and b>0");
        
        {
          dbg << "attrOr takes the expression that is currently in-scope and ORs another operation to it."<<endl;
          attrOr aO(new attrEQ("c", "wrongValue"));
          scope s("This scope is enabled even though last condition is false because it is OR-ed to a true condition.");
        }
      }
      
      {
        attrAnd aA(new attrLT("b", 0));
        scope s("This scope is not created since b==2");
        
        attrIf aI(new attrLE("b", 2));
        dbg << "The attrIf expression ignores the expressions that are currently in-scope and begins a new expression. "<<
               "This text is therefore enabled even though prior text in the same scope is diabled."<<endl;
      }
    }
  }

  return 0;
}

int fibConditionalIndent(int a) {
  // Each recursive call to fibScopeLinks adds an indent level, prepending ":" to text printed by deeper calls to fibConditionalIndent. 
  // To reduce the amount of textprinted, we only add indentation if the verbosity level >= a
  indent ind(":  ", attrGE("verbosity", (long)a));
  
  if(a==0 || a==1) { 
    dbg << "=1"<<endl;
    return 1;
  } else {
    int val = fibConditionalIndent(a-1) + fibConditionalIndent(a-2);
    dbg << "="<<val<<endl;
    return val;
  }
}
