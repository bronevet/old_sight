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
    
  module rootModule(group("Root", 0, 1), namedMeasures("time", new timeMeasure())); 
  
  // List of particle positions
  //for(int numDims=1; numDims<=4; numDims++) {
  //int numDims=3;
  //for(int numParticles=10; numParticles<=1000; numParticles*=10) {
  //int numParticles=10;
  double neighRadius = .2;
  int neghRefreshPeriod=10;
  int numTS=100;
  vector<state> particles;
  
  srand(time(NULL));
  
  // Generate the initial particle positions
  module partInitModule(group("Initialization", 1, 1), inputs(rootModule.outPort(0)), namedMeasures("time", new timeMeasure()));
  for(int p=0; p<numParticles; p++) {
    vector<double> curPos;
    for(int d=0; d<numDims; d++) curPos.push_back(((double)rand()/(double)RAND_MAX));
    particles.push_back(state((double)rand()/(double)RAND_MAX, curPos));
  }
  partInitModule.setOutCtxt(0, context(config("deviation", posDev(particles, numDims),
                                              "numParticles", numParticles,
                                              "numDims", numDims)));
  
  std::vector<port> forceOutputs;
  std::vector<port> neighOutputs;
  map<int, set<int> > neighbors;
  for(int t=0; t<numTS; t++) {
    //scope s(txt()<<"Iteration "<<t);
    if(t%neghRefreshPeriod==0) {
      //scope s("Computing neighbors");
      // Find each particle's nearest neighbors
      
      module neighModule(group("Neighbors", 1, 1), 
                               inputs(// particles
                                      (t==0? partInitModule.outPort(0): forceOutputs[0])),
                               neighOutputs,
                               namedMeasures("time", new timeMeasure(),
                                             "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
      
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
      
      neighModule.setOutCtxt(0, context(config("totalNeighbors", totalNeighbors)));
      
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
      module forceModule(group("Forces", 2, 1), 
                         inputs(// particles
                                (t==0? partInitModule.outPort(0): forceOutputs[0]),
                                // neighbors
                                neighOutputs[0]),
                         forceOutputs,
                         namedMeasures("time", new timeMeasure(),
                                       "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
  
      
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
      
      forceModule.setOutCtxt(0, context(config("deviation", posDev(particles, numDims),
                                               "numParticles", numParticles,
                                               "numDims", numDims)));
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
