#include <iostream>
#include <iomanip>

#include "kmedoids.h"
#include "matrix_utils.h"
#include "point.h"
#include "point_set.h"
#include "Timer.h"
#include <fstream>

using namespace cluster;
using namespace std;

int main(int argc, char **argv) {

	size_t i, j, nprocs,
		k; 	// # of clusters

    kmedoids cluster;
    dissimilarity_matrix distance;

	//input: #processes
    if (argc != 3) {
    	cout << "Error" << endl;
    	return 0;
    }
    k=atoi(argv[2]);
    nprocs=atoi(argv[1]);
    cout<<k<<"  "<<nprocs<<endl;
	//read file, get distances

	string line;
	ifstream myfile ("/g/g92/polyzou1/examples/example21/distances.txt");
	if (myfile.is_open())
	{
		distance.resize(nprocs,nprocs);
		for (i=0; i < nprocs; i++) {
			//distance(i,i)=0;
			for (j=0; j < i; j++) {
				getline(myfile,line);
				distance(i,j) = atof(line.c_str());
				cout << i<<"	"<<j<<":"<<distance(i,j)<<endl;
		    }
		}
/*		while ( getline (myfile,line) )
		{
			cout << line << '\n';
		} */
		myfile.close();
	}	else cout << "Unable to open file";


    //clustering
    cerr << "Starting Test with k=" << k <<endl;
    cluster.pam(distance, k);
    cerr << "Finished."    << endl;

//    ostringstream label;
//	label << "Trial 1";
//	draw(label.str(), points.points(), cluster);
	for (i=0; i < nprocs; i++) {
		cout<< i << "  " << cluster.cluster_ids[i]<< "    " << cluster.is_medoid(i) << endl;
	}
}
