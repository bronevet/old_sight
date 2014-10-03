#include "sight.h"

using namespace std;
using namespace sight;

string outDir;
string vizlist;
ofstream tFile;
ofstream inouFile;
ofstream datFile;
ofstream ioInfoFile;

// viz api
// add new node
void add_node(int nodeID, string nodeName, int num_inputs, int num_outputs, int parentID)
{
	ostringstream tFName;
	tFName   << outDir << "/node.txt";
	tFile.open(tFName.str().c_str(), std::fstream::app);
	tFile << nodeID <<":"<< nodeName <<":"<< num_inputs <<":"<<num_outputs<<":"<<parentID<<endl;
	tFile.close();
}

// input data for statistic visualization
void add_viz(int nodeID, int buttonID, string viz)
{
	ostringstream datFName;
	datFName << outDir << "/dat.txt";
	datFile.open(datFName.str().c_str(), std::fstream::app);
	datFile << nodeID << "," << buttonID << "," << viz << endl;
	datFile.close();
}
	
// connection between input fromID of from_nodeID and output toID of to_nodeID
void add_inout(int fromID, int from_nodeID, int toID, int to_nodeID)
{
	ostringstream inouFName;
	inouFName << outDir << "/inout.txt";
	inouFile.open(inouFName.str().c_str(), std::fstream::app);
	inouFile <<fromID<<"_output_"<<from_nodeID<<":"<<toID<<"_input_"<<to_nodeID<< endl;
	inouFile.close();
}

// data for input and output variable information of modules
void add_ioInfo(int nodeID, int num_polyFit, string fitText)
{
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo.txt";
	ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);
	ioInfoFile << nodeID <<";"<< num_polyFit<<";"<<fitText<<endl;
	ioInfoFile.close();
}

int main(int argc, char** argv)
{
	SightInit(argc, argv, "12.HoaVizAPI", "dbg.12.HoaVizAPI");

	// It is possible to write arbitrary text to the debug output
	dbg << "<h1>Example 12: HoaVizAPI</h1>" << endl;

	outDir = "hoaviz_api";
	vizlist = "scatter3d:ccp:pcp";
	
	// add nodes
	add_node(0, "Node1", 1, 1, -1);
	add_node(1, "Node2", 1, 0, 0);
	add_node(2, "Node3", 2, 1, 0);
	add_node(3, "Node4", 1, 0, 1);
	
	// add statistic visualization methods
	add_viz(0, 0, vizlist);	
	add_viz(1, 1, vizlist);
	add_viz(2, 2, vizlist);
	add_viz(3, 3, vizlist);

	// hoaViz
	dbg << "<iframe id=\"flGrFrame\" src=\"../../hoaviz_api/index.html\" width=\"1300\" height=\"1200\"></iframe>\n";

	return 0;
}


