/// \file
/// Leapfrog time integrator

#include "timestep.h"

#include "CoMDTypes.h"
#include "linkCells.h"
#include "parallel.h"
#include "performanceTimers.h"
#include "memUtils.h"
#include "sight.h"
using namespace sight;
using namespace std;
trace** particleTraces;

// Gathers the given property from the atoms on this processor into a single contiguous buffer and returns 
// a pointer to this buffer as well as the number of elements in the buffer.
// Callers do not need to deallocate it and it will be reused in subsequent calls to getAllAtoms().
std::pair<real3*, int> getAllAtoms(SimFlat* s, int nBoxes, real3* property);

// Returns the mean of the values in a given 3d vector
real_t mean(real3 d);

// Computing the standard deviation of the given property of all the particles in each spatial dimension
void computeParticleStdDev(SimFlat* s, int nBoxes, real3* property, real3 relStdDev);
static void advanceVelocity(SimFlat* s, int nBoxes, real_t dt);
static void advancePosition(SimFlat* s, int nBoxes, real_t dt);

/// Advance the simulation time to t+dt using a leap frog method
/// (equivalent to velocity verlet).
///
/// Forces must be computed before calling the integrator the first time.
///
///  - Advance velocities half time step using forces
///  - Advance positions full time step using velocities
///  - Update link cells and exchange remote particles
///  - Compute forces
///  - Update velocities half time step using forces
///
/// This leaves positions, velocities, and forces at t+dt, with the
/// forces ready to perform the half step velocity update at the top of
/// the next call.
///
/// After nSteps the kinetic energy is computed for diagnostic output.
double timestep(SimFlat* s, int printRate, int curTime, real_t dt, int iStep, int nSteps)
{
   for (int ii=0; ii<printRate && iStep<nSteps; ++ii,++iStep)
   {
     attr tsA("time", iStep*dt);
     
#if defined(TRACE_POS)
      scope traceScope(txt()<<"Time "<<(iStep*dt), scope::high);
      trace posTrace("Positions", trace::showBegin, trace::scatter3d);
#endif

      real3 posStdDev;
      computeParticleStdDev(s, s->boxes->nLocalBoxes, s->atoms->r, posStdDev);
      real3 momStdDev;
      computeParticleStdDev(s, s->boxes->nLocalBoxes, s->atoms->p, momStdDev);
     
      std::vector<port> externalOutputs;
#if defined(MODULES)
#if defined(MOD_COMP)
      compModule tsModule(instance("TimeStep", 1, 2), 
                         inputs(port(context("curTime", curTime))),
#else
      module tsModule(instance("TimeStep", 2, 1), 
                        inputs(port(context("dt", dt)),
                               port(context("posStdDev",  mean(posStdDev),
                                            "momStdDev",  mean(momStdDev),
                                            "ePotential", s->ePotential,
                                            "eKinetic",   s->eKinetic))),
#endif // MOD_COMP
                         externalOutputs,
#if defined(MOD_COMP)
                         dt==1 && s->lat==3.615, // isReference
                         context("dt",  dt,
                                 "lat", s->lat), // options
                         compNamedMeasures("time", new timeMeasure(), LkComp(2, attrValue::floatT, true),
                                           "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS)), LkComp(2, attrValue::intT, true))
#else
                         namedMeasures("time", new timeMeasure(),
                                       "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS)))
#endif // MOD_COMP 
                        );
#endif // MODULES
     
      { 
#if defined(MODULES)
         module advModule(instance("AdvanceVel1", 1, 0), 
                          inputs(port(context("ii", ii))),
                          namedMeasures(
                              "time", new timeMeasure(),
                              "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES
         startTimer(velocityTimer);
         advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt); 
         stopTimer(velocityTimer);
      }

      {
#if defined(MODULES)
         module posModule(instance("AdvPos", 1, 0), 
                          inputs(port(context("ii", ii))),
                          namedMeasures(
                              "time", new timeMeasure(),
                              "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES
         startTimer(positionTimer);
         advancePosition(s, s->boxes->nLocalBoxes, dt);
         stopTimer(positionTimer);
      }
      
      {
#if defined(MODULES)
         module redModule(instance("Redistribute", 1, 0), 
                         inputs(port(context("ii", ii))),
                         namedMeasures(
                             "time", new timeMeasure(),
                             "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES
         startTimer(redistributeTimer);
         redistributeAtoms(s);
         stopTimer(redistributeTimer);
      }

      {
#if defined(MODULES)
         module advModule(instance("Forces", 1, 0), 
                          inputs(port(context("ii", ii))),
                          namedMeasures(
                              "time", new timeMeasure(),
                              "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES
         startTimer(computeForceTimer);
         computeForce(s);
         stopTimer(computeForceTimer);
      }

      {
#if defined(MODULES)
         module advModule(instance("AdvanceVel2", 1, 0), 
                          inputs(port(context("ii", ii))),
                          namedMeasures(
                              "time", new timeMeasure(),
                              "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES
         startTimer(velocityTimer);
         advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt); 
         stopTimer(velocityTimer);
      }
      
      curTime+=dt;
      
      kineticEnergy(s);

#if defined(MODULES)
#if defined(MOD_COMP)
      tsModule.setOutCtxt(0, compContext("posStdDev",  mean(posStdDev), LkComp(2, attrValue::floatT, true),
                                         "momStdDev",  mean(momStdDev), LkComp(2, attrValue::floatT, true),
                                         "ePotential", s->ePotential,   LkComp(2, attrValue::floatT, true),
                                         "eKinetic",   s->eKinetic,     LkComp(2, attrValue::floatT, true)));

      std::pair<real3*, int> positionArray = getAllAtoms(s, s->boxes->nLocalBoxes, s->atoms->r);
      tsModule.setOutCtxt(1, compContext("positions",  
                                         sightArray(sightArray::dims(positionArray.second,3), (double*)positionArray.first), 
                                         LkComp(2, attrValue::floatT, true)));
#else
      computeParticleStdDev(s, s->boxes->nLocalBoxes, s->atoms->r, posStdDev);
      computeParticleStdDev(s, s->boxes->nLocalBoxes, s->atoms->p, momStdDev);
      
      tsModule.setOutCtxt(0, context("posStdDev",  mean(posStdDev),
                                     "momStdDev",  mean(momStdDev),
                                     "ePotential", s->ePotential,
                                     "eKinetic",   s->eKinetic));
#endif // MOD_COMP
#endif // MODULES

#if defined(TRACE_PATH)      
      // Trace the positions and properties of all particles
      for (int iBox=0, i=0, pathTraceCnt=0; iBox<s->boxes->nLocalBoxes; iBox++)
      {
         for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++,i++)
         {
           if(i%10000 != 0) continue;
           traceAttr(particleTraces[pathTraceCnt], 
                     trace::ctxtVals("x", s->atoms->r[iOff][0],
                                     "y", s->atoms->r[iOff][1],
                                     "z", s->atoms->r[iOff][2]),
                     trace::observation("U", s->atoms->U[iOff],
                                        "fx", s->atoms->f[iOff][0],
                                        "fy", s->atoms->f[iOff][1],
                                        "fz", s->atoms->f[iOff][2]
                                        ));
           pathTraceCnt++;
         }
      }
#endif // TRACE_PATH
      
#if defined(TRACE_POS)
      for (int iBox=0; iBox<s->boxes->nLocalBoxes; iBox++)
      {
         for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
         {
           if(rand()%100!=0) continue;
           traceAttr(&posTrace, 
                     trace::ctxtVals("x", s->atoms->r[iOff][0],
                                     "y", s->atoms->r[iOff][1],
                                     "z", s->atoms->r[iOff][2]),
                     trace::observation("U", s->atoms->U[iOff],
                                        "fx", s->atoms->f[iOff][0],
                                        "fy", s->atoms->f[iOff][1],
                                        "fz", s->atoms->f[iOff][2]
                                        ));
         }
      }
#endif // TRACE_POS
   }
   
   return s->ePotential;
}

// Gathers the given property from the atoms on this processor into a single contiguous buffer and returns 
// a pointer to this buffer as well as the number of elements in the buffer.
// Callers do not need to deallocate it and it will be reused in subsequent calls to getAllAtoms().
std::pair<real3*, int> getAllAtoms(SimFlat* s, int nBoxes, real3* property) {
  static real3* buf=NULL;
  
  if(buf == NULL) buf = (real3*)comdMalloc(MAXATOMS*s->boxes->nTotalBoxes*sizeof(real3));
  
  int idx=0;
  for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++, idx++)
      {
        for(int m=0; m<3; m++) buf[idx][m] = property[iOff][m];
      }
  }
  
  return std::make_pair(buf, idx);
}

// Returns the mean of the values in a given 3d vector
real_t mean(real3 d) {
  return (d[0] + d[1] + d[2])/3;
}

// Computing the standard deviation of the given property of all the particles in each spatial dimension
void computeParticleStdDev(SimFlat* s, int nBoxes, real3* property, real3 relStdDev)
{
   // The average property of all particles
   real3 avg;
   for(int m=0; m<3; m++) avg[m] = 0;
   
   // The total number of particles
   int count=0;
   
   // Compute the center of the particle properties
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
        for(int m=0; m<3; m++) avg[m] += property[iOff][m];
        count++;    
      }
   }
   
   for(int m=0; m<3; m++) avg[m] /= count;

   
   // Compute the variance of the particle properties relative to the center
   
   // The variance of particle positions
   real3 variance;
   for(int m=0; m<3; m++) variance[m] = 0;
   
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
        for(int m=0; m<3; m++) 
          variance[m] += (property[iOff][m] - avg[m])*
                         (property[iOff][m] - avg[m]);
      }
   }
   
   for(int m=0; m<3; m++) variance[m] /= count;
   
   // The standard deviation of particle properties relative to the average
   for(int m=0; m<3; m++) relStdDev[m] = sqrt(variance[m]) / (avg[m]<0? -avg[m]: avg[m]);
}

void computeForce(SimFlat* s)
{
   s->pot->force(s);
}


void advanceVelocity(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         s->atoms->p[iOff][0] += dt*s->atoms->f[iOff][0];
         s->atoms->p[iOff][1] += dt*s->atoms->f[iOff][1];
         s->atoms->p[iOff][2] += dt*s->atoms->f[iOff][2];
      }
   }
}

void advancePosition(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 1.0/s->species[iSpecies].mass;
         s->atoms->r[iOff][0] += dt*s->atoms->p[iOff][0]*invMass;
         s->atoms->r[iOff][1] += dt*s->atoms->p[iOff][1]*invMass;
         s->atoms->r[iOff][2] += dt*s->atoms->p[iOff][2]*invMass;
      }
   }
}

/// Calculates total kinetic and potential energy across all tasks.  The
/// local potential energy is a by-product of the force routine.
void kineticEnergy(SimFlat* s)
{
/*#if defined(MODULES)
  module enModule(instance("Kinetic Energy", 0, 0), 
                     //inputs(port(context("ii", ii))),
                     namedMeasures(
                         "time", new timeMeasure(),
                         "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));
#endif // MODULES  */
   real_t eLocal[2];
   eLocal[0] = s->ePotential;
   eLocal[1] = 0;
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 0.5/s->species[iSpecies].mass;
         eLocal[1] += ( s->atoms->p[iOff][0] * s->atoms->p[iOff][0] +
         s->atoms->p[iOff][1] * s->atoms->p[iOff][1] +
         s->atoms->p[iOff][2] * s->atoms->p[iOff][2] )*invMass;
      }
   }

   real_t eSum[2];
   startTimer(commReduceTimer);
   addRealParallel(eLocal, eSum, 2);
   stopTimer(commReduceTimer);

   s->ePotential = eSum[0];
   s->eKinetic = eSum[1];
}

/// \details
/// This function provides one-stop shopping for the sequence of events
/// that must occur for a proper exchange of halo atoms after the atom
/// positions have been updated by the integrator.
///
/// - updateLinkCells: Since atoms have moved, some may be in the wrong
///   link cells.
/// - haloExchange (atom version): Sends atom data to remote tasks. 
/// - sort: Sort the atoms.
///
/// \see updateLinkCells
/// \see initAtomHaloExchange
/// \see sortAtomsInCell
void redistributeAtoms(SimFlat* sim)
{
   updateLinkCells(sim->boxes, sim->atoms);

   startTimer(atomHaloTimer);
   haloExchange(sim->atomExchange, sim);
   stopTimer(atomHaloTimer);

   for (int ii=0; ii<sim->boxes->nTotalBoxes; ++ii)
      sortAtomsInCell(sim->atoms, sim->boxes, ii);
}
