#include "sight.h"
#include <math.h>
#include <map>
#include <assert.h>
#include <unistd.h>
using namespace std;
using namespace sight;

class state {
  public:
  double mass;
  vector<double> pos;
  
  state(double mass, vector<double>& pos) : mass(mass), pos(pos) {}
    
  string str() const {
    ostringstream s;
    s << "[state mass="<<mass<<", pos=<";
    for(int i=0; i<pos.size(); i++) {
      if(i!=0) s << ", ";
      s << pos[i];
    }
    s << ">]";
    return s.str();
  }
};

// Calculates the average standard deviation of each dimension of particle positions
double posDev(const vector<state>& particles, int numDims);
double vecDist(vector<double>& p1, vector<double>& p2);
void initVec(vector<double>& vec, int numDims, double val);
void updateForce(state& p1, state& p2, vector<double>& force);

int main(int argc, char** argv)
{
  if(argc<=2) { cerr << "Usage: 8.Modules numDims numParticles"<<endl; exit(-1); }
  long numDims      = strtol(argv[1], NULL, 10);
  long numParticles = strtol(argv[2], NULL, 10);

  SightInit(argc, argv, txt()<<"8.Modules", 
                        txt()<<"dbg.8.Modules."<<numDims<<"_dims."<<numParticles<<"_particles");

  dbg << "<h1>Example 8: Modules, "<<numDims<<" dimensions, "<<numParticles<<" particles</h1>" << endl;

  dbg << "This example illustrates the use of modules, which are a way to identify and document the modular structure "<<
         "of application components and their inter-dependencies. Given such information about the application Sight "<<
         "presents a graphical representation of the application's modular structure, as well as a set of measurements "<<
         "taken during each module's execution (e.g. execution time, performance counters or values of module outputs. "<<
         "The Sight module visualization shows a graph with one node for each module. For each module Sight shows its "<<
         "inputs and outputs and how the outputs of one module connect to the inputs of another. Users may optionally "<<
         "provide more detailed information about the properties of each output of a module, such as the sparsity of a "<<
         "matrix or the height of a tree. Sight shows the properties provided for each input of a module and further, "<<
         "presents a polynomial (inferred using regression techniques) that predicts the value of each measurement value "<<
         "(e.g. time or outputs) as a function of the properties of the inputs. Finally, to make this relationship more "<<
         "visual Sight allows users to click on each input property/measurement pair to see the plot of the measurement "<<
         "as a function of the values taken by this property of this input."<<endl<<endl;

  dbg << "Modules are used by declaring a variable of type module for each application code module. All code executed "<<
         "while this variable is in-scope is considered to be part of the module. The module's name "<<
         "and number of inputs and outputs is provided as the first argument (ex: instance(\"Initialization\", 1, 1)). The "<<
         "next argument identifies the module's inputs, which should be the outputs of other modules "<<
         "(ex: inputs(initModule.outPort(0))). The next argument provides space where information about the the module's "<<
         "outputs will be stored. It is a reference to a vector<port>, which is filled by the module with information detailing the outputs. This is "<<
         "useful because modules that use a given module's outputs often start after the module completes and its variable "<<
         "goes out of scope. Placing information about its outputs in a separate vector that outlives the module makes it easier "<<
         "for other modules to identify the way its outputs connect to their inputs. The final argument is a list of "<<
         "measurements that should be taken during the module's execution and displayed in the graphical view "<<
         "(ex: namedMeasures(\"time\", new timeMeasure(), \"PAPI\", new PAPIMeasure(papiEvents(PAPI_TOT_INS))."<<endl<<endl;

  dbg << "This example code is a mock-up of a molecular dynamics simulation, with modules for initialization, force computation "<<
         "and computation of nearest-neighbor lists. This output corresponds to 9 runs of this code with 1, 30 or 100 particles, "<<
         "each containing 1, 2 or 3 dimensional positions. The outputs of these runs are merged, including their input/output "<<
         "relations and module measurements."<<endl;    
  
  modularApp mdApp("Molecular Dynamics", 
                   namedMeasures("time",      new timeMeasure(),
                                 "timestamp", new timeStampMeasure())); 
  
  // List of particle positions
  double neighRadius = .2;
  int neghRefreshPeriod=10;
  int numTS=100;
  vector<state> particles;
  
  srand(time(NULL));
  
  // Generate the initial particle positions
  std::vector<port> initOutputs;
  { module initModule(instance("Initialization", 0, 1), initOutputs, namedMeasures("PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS, PAPI_L2_TC_MR))));
    for(int p=0; p<numParticles; p++) {
      vector<double> curPos;
      for(int d=0; d<numDims; d++) curPos.push_back(((double)rand()/(double)RAND_MAX));
      particles.push_back(state((double)rand()/(double)RAND_MAX, curPos));
    }
    initModule.setOutCtxt(0, context("deviation", posDev(particles, numDims),
                                     "numParticles", numParticles,
                                     "numDims", numDims));
  }
  
  std::vector<port> forceOutputs;
  std::vector<port> neighOutputs;
  map<int, set<int> > neighbors;
  for(int t=0; t<numTS; t++) {
    module timeStepModule(instance("TimeStep", 1, 0), inputs(port(context("t", t))), 
                          namedMeasures("PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS, PAPI_L2_TC_MR))));

    //scope s(txt()<<"Iteration "<<t);
    if(t%neghRefreshPeriod==0) {
      //scope s("Computing neighbors");
      // Find each particle's nearest neighbors
      
      module neighModule(instance("Neighbors", 1, 1), 
                               inputs(// particles
                                      (t==0? initOutputs[0]: forceOutputs[0])),
                               neighOutputs,
                               namedMeasures("PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS, PAPI_L2_TC_MR))));
      
      // Maps each particles (idx in particle) to the set of its neighbors
      
      int totalNeighbors=0;
      for(int p1=0; p1<particles.size(); p1++) {
      for(int p2=0; p2<particles.size(); p2++) {
        if(p1==p2) continue;
        
        double d = vecDist(particles[p1].pos, particles[p2].pos);
        if(d<=neighRadius) {
          neighbors[p1].insert(p2);
          totalNeighbors++;
        }
      } }
      
      neighModule.setOutCtxt(0, context("totalNeighbors", totalNeighbors));
      
      /*for(map<int, set<int> >::iterator p=neighbors.begin(); p!=neighbors.end(); p++) {
        dbg << p->first << ":";
        for(set<int>::iterator n=p->second.begin(); n!=p->second.end(); n++)
          dbg << " "<<*n;
        dbg << endl;
      }*/
    }
    
    // Compute the forces on all particles from their neighbors and update their positions
    {
      //scope s("Computing forces");
      module forceModule(instance("Forces", 2, 1), 
                         inputs(// particles
                                (t==0? initOutputs[0]: forceOutputs[0]),
                                // neighbors
                                neighOutputs[0]),
                         forceOutputs,
                         namedMeasures("PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS, PAPI_L2_TC_MR))));
  
      
      for(map<int, set<int> >::iterator p=neighbors.begin(); p!=neighbors.end(); p++) {
        // Initialize the force vector to 0
        vector<double> force;
        initVec(force, numDims, 0);
        
        for(set<int>::iterator n=p->second.begin(); n!=p->second.end(); n++) {
          updateForce(particles[p->first], particles[*n], force);
        }
        /*dbg << p->first << ": "<<particles[p->first].str()<<", force=";
        for(int i=0; i<force.size();i++)
          dbg << " " << force[i];
        dbg << endl;*/
        
        // Update the particle's positions
        for(int d=0; d<numDims; d++)
          particles[p->first].pos[d] += force[d];
      }
      
      /*for(int p=0; p<particles.size(); p++)
        dbg << p << ": "<<particles[p].str()<<endl;*/
      
      forceModule.setOutCtxt(0, context("deviation", posDev(particles, numDims),
                                        "numParticles", numParticles,
                                        "numDims", numDims));
    }
  } //} }
}

// Calculates the average standard deviation of each dimension of particle positions
double posDev(const vector<state>& particles, int numDims) {
  double avgDev=0;
  for(int d=0; d<numDims; d++) {
    double avg=0;
    for(vector<state>::const_iterator p=particles.begin(); p!=particles.end(); p++)
      avg += p->pos[d];
    avg /= particles.size();
    
    double dev=0;
    for(vector<state>::const_iterator p=particles.begin(); p!=particles.end(); p++)
      dev += (p->pos[d] - avg) * (p->pos[d] - avg);
    dev /= sqrt(dev / particles.size());
    
    avgDev += dev;
  }
  
  return avgDev / numDims;
}

double vecDist(vector<double>& p1, vector<double>& p2) {
  assert(p1.size() == p2.size());
  
  double d=0;
  for(int i=0; i<p1.size(); i++)
    d += (p1[i] - p2[i]) * (p1[i] - p2[i]);
  return sqrt(d/p1.size());
}

void initVec(vector<double>& vec, int numDims, double val) {
  for(int i=0; i<numDims; i++)
    vec.push_back(val);
}

void updateForce(state& p1, state& p2, vector<double>& force) {
  assert(p1.pos.size() == p2.pos.size());
  
  for(int i=0; i<p1.pos.size(); i++) {
    double dist = (p1.pos[i] - p2.pos[i]);
    force[i] += p1.mass * p2.mass / dist;
  }
}




