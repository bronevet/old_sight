#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;

class dottableExample: public dottable
{
  public:
  // Returns a string that containts the representation of the object as a graph in the DOT language
  // that has the given name
  std::string toDOT(std::string graphName);
};

int main(int argc, char** argv)
{
	SightInit(argc, argv, "12.HoaVizAPI", "dbg.12.HoaVizAPI");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 12: HoaVizAPI</h1>" << endl;

  {
    {
      scope dotStr("This graph was generated from a string");
      //graph::genGraph(string("graph graphname {\n     a -- b -- c;\n     b -- d;\n}"));
      //flowgraph::genFlowGraph(string("graph ex1 {\n     a -- b -- c;\n     b -- d;\n}"));
      //hoaViz API
      flowgraph::genFlowGraph(string("graph ex1 {\n     a -- b -- c;\n     b -- d;\n}"));
    }

    {
      scope dotStr("This graph was generated from an object that extend the dottable class");
      dottableExample ex;
      graph::genGraph(ex);
      //flowgraph::genFlowGraph(ex);
    }
  }

  return 0;
}


// Returns a string that contains the representation of the object as a graph in the DOT language
// that has the given name
std::string dottableExample::toDOT(std::string graphName) {
  ostringstream oss;
  oss << "graph ex2 {"<<endl;
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



