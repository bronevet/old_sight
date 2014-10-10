#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;


int main(int argc, char** argv)
{
	SightInit(argc, argv, "12.HoaVizAPI", "dbg.12.HoaVizAPI");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 12: HoaVizAPI</h1>" << endl;

    {
      scope dotStr("This graph was generated from a string");
      //hoaViz API
      flowgraph::genFlowGraph(string("a-b-c;b-d-e-f;e-k;e-m"));
    }

  return 0;
}




