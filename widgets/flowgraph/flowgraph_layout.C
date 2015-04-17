// Licence information included in file LICENCE
#include "../../sight_layout.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;

namespace sight {
namespace layout {

// Record the layout handlers in this file
void* flowgraphEnterHandler(properties::iterator props) { return new flowgraph(props); }
void  flowgraphExitHandler(void* obj) { flowgraph* g = static_cast<flowgraph*>(obj); delete g; }
  
flowgraphLayoutHandlerInstantiator::flowgraphLayoutHandlerInstantiator() {
  (*layoutEnterHandlers)["flowgraph"]   = &flowgraphEnterHandler;
  (*layoutExitHandlers )["flowgraph"]   = &flowgraphExitHandler;
  (*layoutEnterHandlers)["dataEncodingFG"] = &flowgraph::setFlowGraphEncoding;
  (*layoutExitHandlers )["dataEncodingFG"] = &defaultExitHandler;
  (*layoutEnterHandlers)["dirEdgeFG"]     = &flowgraph::addDirEdgeFG;
  (*layoutExitHandlers )["dirEdgeFG"]     = &defaultExitHandler;  
  (*layoutEnterHandlers)["nodeHyperlink"]     = &flowgraph::addNodeHyperlink;
  (*layoutExitHandlers )["nodeHyperlink"]     = &defaultExitHandler;
  (*layoutEnterHandlers)["undirEdgeFG"]   = &flowgraph::addUndirEdgeFG;
  (*layoutExitHandlers )["undirEdgeFG"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["invisEdgeFG"]   = &flowgraph::addInvisEdgeFG;
  (*layoutExitHandlers )["invisEdgeFG"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["nodeFG"]        = &flowgraph::addNodeFG;
  (*layoutExitHandlers )["nodeFG"]        = &defaultExitHandler;
  (*layoutEnterHandlers)["subGraphFG"]    = &flowgraph::startSubFlowGraph;
  (*layoutExitHandlers )["subGraphFG"]    = &flowgraph::endSubFlowGraph;
}
flowgraphLayoutHandlerInstantiator flowgraphLayoutHandlerInstance;

// The path the directory where output files of the graph widget are stored
// Relative to current path
std::string flowgraph::outDir="";
// Relative to root of HTML document
std::string flowgraph::htmlOutDir="";

// Maps the IDs of the currently active graphs to their graph objects
std::map<int, flowgraph*> flowgraph::active;

flowgraph::flowgraph(properties::iterator props) : block(properties::next(props)) {
  dbg.enterBlock(this, false, true);
  //imgPath = dbg.addImage("svg");
  maxNodeID = 0;
  
  initEnvironment();  
    
  flowgraphID = properties::getInt(props, "flowgraphID");
  
  dbg.ownerAccessing();
  dbg << "<div id=\"graph_container_"<<flowgraphID<<"\"></div>\n";
  dbg.userAccessing();
  

  // If the dataText encoding of the graph is already provided, emit it immediately
  if(props.exists("dataText")) {
    outputDataFlowGraph(properties::get(props, "dataText"));
    flowgraphOutput = true;
  // Otherwise, wait to observe the nodes and edges of the graph before emitting it in the destructor
  } else
    flowgraphOutput = false;
 
  subFlowGraphCounter=0;
  
  // Add the current graph to the map of ative graphs
  active[flowgraphID] = this;

  	// hoa edit
	// create hoaviz canvas
	ostringstream hoavizCanvasFName;
	hoavizCanvasFName << outDir << "/hoaviz_canvas2.txt";
	hoavizCanvasFile.open(hoavizCanvasFName.str().c_str());
	hoavizCanvasFile <<"<canvas id=\"flGra\" data-processing-sources=\"widgets/flowgraph/flGra.pde\" width=\"100%\" height=\"100%\"> </canvas>"<< endl;
	hoavizCanvasFile.close();

    //dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html\" width=\"2300\" height=\"1200\"></iframe>\n";
	//dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html?graName="<< graphName << "\" width=\"1600\" height=\"1000\"></iframe>\n";
    /*
    // node file
	ostringstream tFName;
	tFName   << outDir << "/node.txt";
	tFile.open(tFName.str().c_str());
	// input_output file
	ostringstream inouFName;
	inouFName << outDir << "/inout.txt";
	inouFile.open(inouFName.str().c_str());
	// data for statistic visualization
	ostringstream datFName;
	datFName << outDir << "/dat.txt";
	datFile.open(datFName.str().c_str());
	// data for input and output variable information of modules
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo.txt";
	ioInfoFile.open(ioInfoFName.str().c_str());
	*/
}

// Initialize the environment within which generated graphs will operate, including
// the JavaScript files that are included as well as the directories that are available.
void flowgraph::initEnvironment() {
  static bool initialized=false;
  
  if(initialized) return;
  initialized = true;
  
  pair<string, string> paths = dbg.createWidgetDir("flowgraph");
  outDir = paths.first;
  htmlOutDir = paths.second;
  
  dbg.includeFile("flowgraph/processing.js"); dbg.includeWidgetScript("flowgraph/processing.js", "text/javascript");
  dbg.includeFile("flowgraph/flgr.js"); dbg.includeWidgetScript("flowgraph/flgr.js", "text/javascript");
  //dbg.includeFile("flowgraph/hoaViz.pde");
  dbg.includeFile("flowgraph/flGra.pde");
  dbg.includeFile("flowgraph/index.html");

}

flowgraph::~flowgraph() {
  if(!flowgraphOutput) {
    outputDataFlowGraph(genDataFlowGraph());
  }

  // hoa edit
  tFile.close();
  inouFile.close();
  datFile.close();
  ioInfoFile.close();
  
  dbg.exitBlock();
  
  // Remove the current graph from the map of active graphs
  assert(active.size()>0);
  assert(active.find(flowgraphID) != active.end());
  assert(active[flowgraphID] == this);
  active.erase(flowgraphID);
}

// Generates and returns the data graph code for this graph
string flowgraph::genDataFlowGraph() {


	// hoa edit
	// node file
	ostringstream tFName;
	tFName   << outDir << "/node_"<< flowgraphID <<".txt";
	tFile.open(tFName.str().c_str(), std::fstream::app);
	// input_output file
	ostringstream inouFName;
	inouFName << outDir << "/inout_"<< flowgraphID <<".txt";
	inouFile.open(inouFName.str().c_str(), std::fstream::app);
	// data for statistic visualization
	ostringstream datFName;
	datFName << outDir << "/dat_"<< flowgraphID <<".txt";
	datFile.open(datFName.str().c_str(), std::fstream::app);
	// data for input and output variable information of modules
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo_"<< flowgraphID <<".txt";
	ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);

	ostringstream vhFName;
	vhFName << outDir << "/vert_hori_"<<flowgraphID<<".txt";
	ofstream vhFile;
	vhFile.open(vhFName.str().c_str(), std::fstream::app);
	vhFile.close();
  
	
	ostringstream linkFName;
	linkFName << outDir << "/link_"<<flowgraphID<<".txt";
	ofstream linkFile;
	linkFile.open(linkFName.str().c_str(), std::fstream::app);
	
  ostringstream data;

  for(map<anchor, string>::iterator b=nodes.begin(); b!=nodes.end(); b++)
  {
	data <<  b->first.getID() <<":"<< b->first.getID() <<":"<< "1" <<":"<<"1"<<":"<<b->second << endl;
    
    // hoa edit
    //tFile << i <<":"<< b->first.getID() <<":"<< "1" <<":"<<"1"<<":"<<b->second << ":" << b->first.getLinkJS() << endl;
    linkFile << b->second << ":" << b->first.getLinkJS() << endl;
    //linkFile << b->first.getID() << ":" << b->second << ":" << ":" << b->first.getLinkJS() << endl;
  }
  // Between the time when an edge was inserted into edges and now, the anchors on both sides of each
  // edge should have been located (attached to a concrete location in the output). This means that
  // some of the edges are now redundant (e.g. multiple forward edges from one location that end up
  // arriving at the same location). We thus create a new set of edges based on the original list.
  // The set's equality checks will eliminate all duplicates.

  set<flowgraphEdge> uniqueEdges;
  for(list<flowgraphEdge>::iterator e=edges.begin(); e!=edges.end(); e++)
    uniqueEdges.insert(*e);
  
  //map<anchor, string>::iterator b = nodes.begin();
  //tFile << b->first.getID() << ":" << b->second << ":0:0:-1" << endl;
    
  for(set<flowgraphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++)
  {
  	// hoa edit
	//inouFile <<"0"<<"_output_"<<e->from.getID()<<":"<<"0"<<"_input_"<<e->to.getID()<< endl;

    data << "0"<<"_output_"<<e->to.getID()<<":"<<"0"<<"_input_"<<e->to.getID()<< endl;

    int tmp = 1;
	for(set<flowgraphEdge>::iterator k=uniqueEdges.begin(); k!=e; k++)
  		if(k->from.getID() == e->from.getID() || k->to.getID() == e->from.getID())
  			tmp = 0;
		
	 
    for(map<anchor, string>::iterator b = nodes.begin(); b!=nodes.end(); b++)
  	{
  		if(tmp == 1)
  			if(b->first.getID() == e->from.getID())
  			{
  				tFile << b->first.getID() << ":" << b->second << ":0:0:-1" << endl;	
  				tmp = 0;
  			}

  		if(b->first.getID() == e->to.getID())
  		{
	    	tFile << e->to.getID() << ":" << b->second << ":0:0:" << e->from.getID() << endl;	
	    	tmp = 0;	
		}
	}
	

    ostringstream style; bool emptyStyle=true;
    if(!e->directed) { if(!emptyStyle) { style << " "; } style << "dir=none";    emptyStyle=false; }
    if(!e->visible)  { if(!emptyStyle) { style << " "; } style << "style=invis"; emptyStyle=false; }
    if(!emptyStyle) data << "[" << style.str() << "]";
  }

  linkFile.close();
  //dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html?graName="<< flowgraphID << "\" width=\"1600\" height=\"1000\"></iframe>\n";

  return data.str();
}

// Given a string representation of a data flowgraph, emits the graph's visual representation
void flowgraph::outputDataFlowGraph(std::string graphdata) {

    std::string texNode="";
	std::string t1, t2;
	int ind = 0;
	int drawNodeGraph = 0;
	std::string vertID, horiID;
	std::string nodeName;
	std::string toAnch;
	

	if(graphdata.find("graphNodeStart:") == 0 || graphdata.find("graphNodeEnd:") == 0 ||  graphdata.find("verhorNodeStart") == 0 || graphdata.find("drawNodeGraph:") == 0)
	{
		std::string graphtmp;
		std::istringstream grd(graphdata);
		ind=0;
		while(std::getline(grd, t1, ':'))	{
			if(ind == 1)
				graphtmp = t1;
			ind++;
		}
		// get graph name and data
		std::istringstream grdat(graphtmp);
		ind=0;
		int in2 = 0;
		while(std::getline(grdat, t2, '{'))	{
			if(ind == 0)
				graphName = t2;
            else if(ind == 1)
                texNode = t2;
			else
			{
				if(graphdata.find("verhorNodeStart") == 0)
				{
					std::istringstream s(t2);
					std::string t3;
					while(std::getline(s, t3 , '-')) {
						if(in2 == 0)
							nodeName = t3;
						else if(in2 == 1)
							vertID = t3;
						else
							horiID = t3;
						in2 += 1;
					}
				}
				else
					nodeName = t2;
			}	
			ind++;
		}

if(graphdata.find("graphNodeStart:") == 0 || graphdata.find("verhorNodeStart") == 0)
{
        ostringstream linkFName;
		linkFName << outDir << "/link_"<<graphName<<".txt";
		ofstream linkFile;
		linkFile.open(linkFName.str().c_str(), std::fstream::app);
	if(texNode.compare("") != 0)
		linkFile << nodeName +"{" + texNode + "\n";
        linkFile.close();
}

/*
		ostringstream linkFName;
		linkFName << outDir << "/link_"<<graphName<<".txt";
		ofstream linkFile;
		linkFile.open(linkFName.str().c_str(), std::fstream::app);
		linkFile.close();
*/

		ostringstream dataFName;
		dataFName << outDir << "/graphNode_"<<graphName<<".txt";
		ofstream dataFile;
		dataFile.open(dataFName.str().c_str(), std::fstream::app);

		ostringstream vhFName;
		vhFName << outDir << "/vert_hori_"<<graphName<<".txt";
		ofstream vhFile;
		vhFile.open(vhFName.str().c_str(), std::fstream::app);

		if(graphdata.find("graphNodeStart:") == 0)
			dataFile << "["+nodeName;
		
		if(graphdata.find("verhorNodeStart:") == 0){
		    dataFile << "["+nodeName;
			vhFile << nodeName + ":" + vertID + ":" + horiID + "\n";
		}
		
		if(graphdata.find("graphNodeEnd:") == 0)
			dataFile << "]";
		dataFile.close();
		vhFile.close();

		std::string line,preNodes;
		ostringstream daFName;
		daFName << outDir << "/graphNode_"<<graphName<<".txt";
		ifstream daF;
		daF.open(daFName.str().c_str());
		while ( getline (daF,line) ) {
			preNodes += line;
		}
		daF.close();

		std:string tok;
		std::istringstream nod(preNodes);
		std::vector< std::string > nodeList;
		std::vector< int > countNode;
		std::vector< int > end_subgr;
		while(std::getline(nod, tok, '['))	{
			nodeList.push_back(tok);
			countNode.push_back(0);
			end_subgr.push_back(0);
		}

		std::string nodeRel;

		int i=1;
		int tam = 0;
		while(i < (int)nodeList.size())
		{
			size_t found = nodeList[i].find("]");
			int c = 0;
			for(int j=0; j<nodeList[i].length(); j++)
				if(found!=std::string::npos)
					c++;

			if(c > 0)
				c -= 1;
			countNode[i] = c;

			tam += countNode[i];
			if(tam == i)
				end_subgr[i] = 1;

			i++;
		}
		i = 1;
		while(i < (int)nodeList.size())
		{
			size_t found = nodeList[i].find("]");

			if(found!=std::string::npos)
			{
				nodeList[i] = nodeList[i].substr(0,found);
				nodeRel += nodeList[i]+";";
				nodeList[i] = nodeList[i-1];
				if(countNode[i] > 1)
					nodeList[i] = nodeList[i-countNode[i]-1];
				if(countNode[i] == 1)
					nodeList[i] = nodeList[i-1];

				if(i < ((int)nodeList.size()-1))
					i = i-1;
			}
			else
			{
				if(end_subgr[i] != 1)
					nodeRel += nodeList[i]+"-";
			}
			i++;
		}

		drawNodeGraph = 1;
		graphdata = graphName+"{"+nodeRel+"}";
	}

	if(graphdata.find("graphNodeStart:") != 0 && graphdata.find("graphNodeEnd:") != 0)
	{
		std::string datatmp_add, datatmp, data;
		int add_edge = 0;
		std::string tok1, tok2, tok3, tok4;
		//int ind = 0;

		if(graphdata.find("addnode:") == 0 || graphdata.find("addedge:") == 0 || graphdata.find("verhoraddnode:") == 0)
		{
			
			std::string graphtmp;
			std::istringstream grd(graphdata);
			std::string to;
			int ind=0;
			while(std::getline(grd, to, ':'))	{
				if(ind == 1)
					graphtmp = to;
				ind++;
			}
			// get graph name and data
			std::istringstream grdata1(graphtmp);
			ind=0;
			while(std::getline(grdata1, tok1, '{'))	{
				if(ind == 0)
					graphName = tok1;
				else{					
					if(graphdata.find("verhoraddnode") == 0){
						int in1=0;
						std::istringstream grdata2(tok1);
						while(std::getline(grdata2, t2, '|'))	{
							if(in1 == 0)
								datatmp_add = t2+"}";
							if(in1 == 1)
								vertID = t2;
							if(in1 == 2)
								horiID = t2;
							if(in1 == 3)
								toAnch = t2;
							in1++;

						}
					}
					else{
						datatmp_add = tok1;
					}

					std::istringstream grdata3(datatmp_add);
					int i1 = 0;
					int i2 = 0;
					while(std::getline(grdata3, tok3, '}'))	{
						if(i1 == 0){
							std::istringstream grdata3(tok3);
							while(std::getline(grdata3, tok4, '-'))	{
								if(i2 == 0)
									nodeName = tok4;
								if(i2 == 1)
									nodeName = tok4;
								i2++;
							}
						}
						i1++;
					}
				}
				ind++;
			}

				
			ostringstream vhFName;
			vhFName << outDir << "/vert_hori_"<<graphName<<".txt";
			ofstream vhFile;
			vhFile.open(vhFName.str().c_str(), std::fstream::app);

			std::string line,preData;
			ostringstream dataFName;
			dataFName << outDir << "/data_"<<graphName<<".txt";
			ifstream dataF;
			dataF.open(dataFName.str().c_str());
			while ( getline (dataF,line) ) {
			  preData += line;
			}
			dataF.close();

			ostringstream linkFName;
			linkFName << outDir << "/link_"<<graphName<<".txt";
			ofstream linkFile;
			linkFile.open(linkFName.str().c_str(), std::fstream::app);
			linkFile.close();

			if(graphdata.find("addnode:") == 0)
			{
				if(preData.compare("") == 0)
					graphdata = graphName+"{"+preData +datatmp_add;
				else
					graphdata = graphName+"{"+preData +";"+datatmp_add;
			}

			if(graphdata.find("verhoraddnode:") == 0){	
				std::string l, pre_nodeName;
				int rep =0;
				ifstream vhF;
				vhF.open(vhFName.str().c_str());
				while ( getline (vhF,l) ) {
					int in1=0;
					std::istringstream gr(l);
					while(std::getline(gr, t2, ':'))	{
						if(in1 == 0){
							pre_nodeName = t2;
							if(nodeName.compare(pre_nodeName) == 0)
								rep = 1;
						}
						in1++;
					}
				}
				vhF.close();
				
				if(rep == 0)
					vhFile << nodeName + ":" + vertID + ":" + horiID + "\n";
				
				if(preData.compare("") == 0)
					graphdata = graphName+"{"+preData +datatmp_add;
				else
					graphdata = graphName+"{"+preData +";"+datatmp_add;
			}
			vhFile.close();

			if(graphdata.find("addedge:") == 0)
			{
				add_edge = 1;
				graphdata = graphName+"{"+preData+"}";
			}

		}


		if(graphdata.find("drawgraph:") != 0)
		{
			// get graph name and data
			std::istringstream grdata(graphdata);
			ind=0;
			while(std::getline(grdata, tok2, '{'))	{
				if(ind == 0)
					graphName = tok2;
				else
					datatmp = tok2;
				ind++;
			}

			std::istringstream grdat(datatmp);
			std::string tok;
			ind = 0;
			while(std::getline(grdat, tok, '}'))	{
				if(ind == 0)
					data = tok;
				ind++;
			}

			std::string fnode(outDir + "/node_" + graphName + ".txt");
			remove(fnode.c_str());
			std::string fdat(outDir + "/dat_" + graphName + ".txt");
			remove(fdat.c_str());
			//std::string finout(outDir + "/inout_" + graphName + ".txt");
			//remove(finout.c_str());
			std::string finfo(outDir + "/ioInfo_" + graphName + ".txt");
			remove(finfo.c_str());

			ostringstream dataFName;   dataFName << outDir << "/data_"<<graphName<<".txt";
			ofstream dataFile;
			dataFile.open(dataFName.str().c_str());
			dataFile << data;
			dataFile.close();

			string vizlist = "scatter3d:ccp:pcp";
			int nodeID = 0;
			int parentID = -1;
			nodesFG.clear();
			parentsFG.clear();
			int br_num = 0;
			int nod_num = 0;
			std::istringstream ss(data);
			std::string token;
			int oldnode = 0;

			//process branches
			while(std::getline(ss, token, ';'))	{
				std::istringstream s(token);
				std::string t;
				br_num++;
				int rel_num = 0;
				while(std::getline(s, t, '-')) {
					if(oldnode == 0)
					{
						if(rel_num == 0)
							nod_num = -1;
						else
							nod_num = nodeID-1;
					}
					else
						oldnode = 0;

					if(nodeID == 0)	{
						nodesFG.push_back(std::make_pair(nodeID, t));
						parentsFG.push_back(std::make_pair(nodeID, parentID));
						nodeID++;
					}
					else {
						oldnode = 0;
						for(int i=0; i < (int)nodesFG.size(); i++) {
							if(t.compare(nodesFG[i].second)==0)
							{
								oldnode = 1;
								nod_num = nodesFG[i].first;
							}
						}
						if(oldnode == 0) {
							nodesFG.push_back(std::make_pair(nodeID, t));
							parentsFG.push_back(std::make_pair(nodeID, nod_num));
							nodeID++;


						}
					}

					rel_num++;
				}
			}

			int num_inputs = 0;
			int num_outputs = 0;
			if(add_edge == 1)
			{
				num_inputs = 1;
				num_outputs = 1;

				std::istringstream s(datatmp_add);
				std::string t, startNode, endNode;
				int from_nodeID, to_nodeID;
				int ind = 0;
				while(std::getline(s, t, '-')) {
					if(ind == 0)
						startNode = t;
					else
						endNode = t;
					ind++;
				}
				for(int i=0; i < (int)nodesFG.size(); i++) {
					if(startNode.compare(nodesFG[i].second) == 0)
						from_nodeID = nodesFG[i].first;
					if(endNode.compare(nodesFG[i].second) == 0)
						to_nodeID = nodesFG[i].first;
				}
				add_inout(from_nodeID, 1, to_nodeID, 0);
				add_edge = 0;
			}
			else
			{
				ostringstream inouFName;
				inouFName << outDir << "/inout_"<<graphName<<".txt";
				inouFile.open(inouFName.str().c_str(), std::fstream::app);
				inouFile.close();
			}

			for(int i=0; i < (int)nodesFG.size(); i++) {
				   add_node(nodesFG[i].first, nodesFG[i].second, num_inputs, num_outputs, parentsFG[i].second);
			}

			ostringstream datFName;
			datFName << outDir << "/dat_"<<graphName<<".txt";
			datFile.open(datFName.str().c_str(), std::fstream::app);
			datFile.close();

			ostringstream ioInfoFName;
			ioInfoFName << outDir << "/ioInfo_"<<graphName<<".txt";
			ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);
			ioInfoFile.close();
                        
            ostringstream vhFName;
            vhFName << outDir << "/vert_hori_"<<graphName<<".txt";
            ofstream vhFile;
            vhFile.open(vhFName.str().c_str(), std::fstream::app);
            vhFile.close();
			// end process branches
		}

		//if(graphdata.find("drawgraph:") == 0 || drawNodeGraph == 1)
		if(graphdata.find("drawgraph:") == 0)
		{
			std::istringstream grd(graphdata);
			std::string to;
			int ind=0;
			while(std::getline(grd, to, ':'))	{
				if(ind == 1)
					graphName = to;
				ind++;
			}

			dbg << "<iframe id=\"flGrFrame\" src=\"widgets/flowgraph/index.html?graName="<< graphName << "\" width=\"1600\" height=\"1000\"></iframe>\n";
		}
	}


    //if(toAnch.compare("}") != 0){		
		int fromNodID = 0;
		anchor toAnc(atoi(toAnch.c_str()));
		for(int i=0; i < (int)nodesFG.size(); i++) {
			if(nodeName.compare(nodesFG[i].second) == 0)
				fromNodID = nodesFG[i].first;
		}
		//std::cout << "nodeID=" << fromNodID << " nodeName" << nodeName << " toAnc_link = " << toAnc.getLinkJS() << std::endl;			
		anchor fromAnc(fromNodID);
		//addDirEdgeFG(fromAnc, toAnc);

	  	//edges.push_back(flowgraphEdge(fromAnc, toAnc, true, true));
		ostringstream linkFName;
		linkFName << outDir << "/link_"<<graphName<<".txt";
		ofstream linkFile;
		linkFile.open(linkFName.str().c_str(), std::fstream::app);
	    linkFile << fromNodID << ":" << toAnc.getLinkJS() << endl;
	    linkFile.close();
	//}
	//addNodeHyperlink(nodeName, toAnc);

	flowgraphOutput = true;
}

// Sets the structure of the current graph by specifying its encoding
void flowgraph::setFlowGraphEncoding(string dataText) {
  outputDataFlowGraph(dataText);
}

void* flowgraph::setFlowGraphEncoding(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->setFlowGraphEncoding(properties::get(props, "data"));
  return NULL;
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void flowgraph::addDirEdgeFG(anchor from, anchor to) {
  edges.push_back(flowgraphEdge(from, to, true, true));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addDirEdge() in the currently active graph
void* flowgraph::addDirEdgeFG(properties::iterator props) {
  anchor from(/*false,*/ properties::getInt(props, "from"));
  anchor to(/*false,*/ properties::getInt(props, "to"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addDirEdgeFG(from, to);
  return NULL;
}

void flowgraph::addNodeHyperlink(std::string fromNode, anchor to){
  int from_nodeID;
  for(int i=0; i < (int)nodesFG.size(); i++) {
  	//cout << "nodesFG[" << i << "]=" << nodesFG[i].first << " - " << nodesFG[i].second;
  if(fromNode.compare(nodesFG[i].second) == 0)
		from_nodeID = nodesFG[i].first;
  }
  anchor from(from_nodeID);
  edges.push_back(flowgraphEdge(from, to, true, true));
}

void* flowgraph::addNodeHyperlink(properties::iterator props){
  std::string fromNode = properties::get(props, "fromNode");
  anchor to(properties::getInt(props, "toAnchor"));  
  //cout << "fromNode = " << fromNode;
  //cout << "to = " << to.getID();
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());  
  active[flowgraphID]->addNodeHyperlink(fromNode, to);
  return NULL;
}

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void flowgraph::addUndirEdgeFG(anchor a, anchor b) {
  edges.push_back(flowgraphEdge(a, b, false, true));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* flowgraph::addUndirEdgeFG(properties::iterator props) {
  anchor a(/*false,*/ properties::getInt(props, "a"));
  anchor b(/*false,*/ properties::getInt(props, "b"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addUndirEdgeFG(a, b);
  return NULL;
}

// Add an invisible undirected edge between the location of the a anchor and the location of the b anchor
void flowgraph::addInvisEdgeFG(anchor a, anchor b) {
  edges.push_back(flowgraphEdge(a, b, false, false));
}

// Static version of the call that pulls the from/to anchor IDs from the properties iterator and calls addUndirEdge() in the currently active graph
void* flowgraph::addInvisEdgeFG(properties::iterator props) {
  anchor a(/*false,*/ properties::getInt(props, "a"));
  anchor b(/*false,*/ properties::getInt(props, "b"));
  
  int flowgraphID = properties::getInt(props, "flowgraphID");
  assert(active.find(flowgraphID) != active.end());
  
  active[flowgraphID]->addInvisEdgeFG(a, b);
  return NULL;
}

void flowgraph::add_node(int nodeID, string nodeName, int num_inputs, int num_outputs, int parentID)
{
	ostringstream tFName;
	tFName   << outDir << "/node_"<<graphName<<".txt";
	tFile.open(tFName.str().c_str(), std::fstream::app);
	tFile << nodeID <<":"<< nodeName <<":"<< num_inputs <<":"<<num_outputs<<":"<<parentID<<endl;
	tFile.close();
}

// input data for statistic visualization
void flowgraph::add_viz(int nodeID, int buttonID, string viz)
{
	ostringstream datFName;
	datFName << outDir << "/dat_"<<graphName<<".txt";
	datFile.open(datFName.str().c_str(), std::fstream::app);
	datFile << nodeID << "," << buttonID << "," << viz << endl;
	datFile.close();
}

// connection between input fromID of from_nodeID and output toID of to_nodeID
void flowgraph::add_inout(int from_nodeID, int fromID, int to_nodeID, int toID)
{
	ostringstream inouFName;
	inouFName << outDir << "/inout_"<<graphName<<".txt";
	inouFile.open(inouFName.str().c_str(), std::fstream::app);
	inouFile <<from_nodeID<<"_output_"<<fromID<<":"<<to_nodeID<<"_input_"<<toID<< endl;
	inouFile.close();
}

// data for input and output variable information of modules
void flowgraph::add_ioInfo(int nodeID, int num_polyFit, string fitText)
{
	ostringstream ioInfoFName;
	ioInfoFName << outDir << "/ioInfo_"<<graphName<<".txt";
	ioInfoFile.open(ioInfoFName.str().c_str(), std::fstream::app);
	ioInfoFile << nodeID <<";"<< num_polyFit<<";"<<fitText<<endl;
	ioInfoFile.close();
}

// Add a node to the graph
void flowgraph::addNodeFG(anchor a, string label) {
  nodes[a] = label;
  tFile << a.getID() <<":"<< label <<":"<< "1" <<":"<<"1"<<":"<<"1"<<endl;
  datFile << a.getID() << "," << a.getID() << "," << "scatter3d:ccp:pcp" << endl;
}

void* flowgraph::addNodeFG(properties::iterator props) {
  int flowgraphID = properties::getInt(props, "flowgraphID");

  if(active.find(flowgraphID) == active.end()) {
    cerr << "ERROR: flow graph with ID "<<flowgraphID<<" is not active when the following node was added: "<<props.str()<<endl;
    cerr << "active(#"<<active.size()<<")=<";
    for(map<int, flowgraph*>::const_iterator a=active.begin(); a!=active.end(); a++) {
      if(a!=active.begin()) cerr << ", ";
      cerr << a->first;
    }
    cerr << ">"<<endl;
    assert(active.find(flowgraphID) != active.end());
  }
  
  active[flowgraphID]->addNodeFG(anchor(/*false,*/ properties::getInt(props, "anchorID")),
                           properties::get(props, "label"));
  return NULL;
}

// Start a sub-graph
void flowgraph::startSubFlowGraph() {
  subFlowGraphCounter++;
}

void flowgraph::startSubFlowGraph(const std::string& label) {
  subFlowGraphCounter++;
}

void* flowgraph::startSubFlowGraph(properties::iterator props) {
  int flowgraphID = props.getInt("flowgraphID");
  if(props.exists("label"))
    active[flowgraphID]->startSubFlowGraph(props.get("label"));
  else
    active[flowgraphID]->startSubFlowGraph();
  return NULL;
}

// End a sub-graph
void flowgraph::endSubFlowGraph(void* obj) {

}


}; // namespace layout
}; // namespace sight
