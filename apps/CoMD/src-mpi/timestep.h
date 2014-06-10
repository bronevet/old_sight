/// \file
/// Leapfrog time integrator

#ifndef __LEAPFROG_H
#define __LEAPFROG_H

#include "CoMDTypes.h"

#if defined(TRACE_PATH)
extern trace** particleTraces;
#endif

double timestep(SimFlat* s, int printRate, int curTime, real_t dt, int iStep, int nSteps);
void computeForce(SimFlat* s);
void kineticEnergy(SimFlat* s);

/// Update local and remote link cells after atoms have moved.
void redistributeAtoms(struct SimFlatSt* sim);

#endif
