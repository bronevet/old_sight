/// \file
/// Initialize the atom configuration.

#include "initAtoms.h"

#include <math.h>
#include <assert.h>

#include "constants.h"
#include "decomposition.h"
#include "parallel.h"
#include "random.h"
#include "linkCells.h"
#include "timestep.h"
#include "memUtils.h"
#include "performanceTimers.h"

#include "sight.h"
using namespace sight;
using namespace std;

// The number of radius shells around each particle
const int numRadiusShells=2;
const double shellRadii[numRadiusShells] = {0, 1.5};//, 2, 2.23,  2.3, 2.5,  2.7,  3 , 5, 8, 12, 15, 18, 20};
// The squares of the values in shellRadii;
double shellRadiiSq[numRadiusShells];
// The number of direction shells in each dimension around each particle
const int shellDirsPerDim = 1;//2
// The total number of direction shells in all directions
const int numShellDirs = 1;//6

static void computeVcm(SimFlat* s, real_t vcm[3]);
#ifdef DISTILL_MONITOR
/*****************
 ***** shell *****
 *****************/

shell::shell(double r2, real3 dr, int count) : r2(r2), count(count)
{
  // Initialize shellRadiiSq if this has not yet been done
  static bool shellRadiiSqInitialized=false;
  if(!shellRadiiSqInitialized) {
    for(int i=0; i<numRadiusShells; i++)
      shellRadiiSq[i] = shellRadii[i] * shellRadii[i];
    shellRadiiSqInitialized = true;
  }
  
  for(int m=0; m<3; m++)
    this->dr[m] = dr[m];
}

// Initialize all the properties in the shell to 0  
void shell::reset() {
  r2 = 0;
  for(int m=0; m<3; m++)
    dr[m] = 0;
  count = 0;
}

// Returns the context associated with shell shellIdx
context shell::getCtxt(int shellIdx) const {
  return context(txt()<<"shell"<<shellIdx<<":r",     sqrt(r2)/count,
                 txt()<<"shell"<<shellIdx<<":d0",    dr[0]/count,
                 txt()<<"shell"<<shellIdx<<":d2",    dr[1]/count,
                 txt()<<"shell"<<shellIdx<<":d2",    dr[2]/count,
                 txt()<<"shell"<<shellIdx<<":count", count);
}
  
/******************
 ***** shells *****
 ******************/
shells::shells() {
  //sh = (shell*)comdMalloc(numRadiusShells*numShellDirs*sizeof(shell));
  sh = new shell[numRadiusShells*numShellDirs];
}

shells::~shells() {
  delete sh;
}

// Initialize all the properties in all the shells to 0  
void shells::reset() {
  for(int i=0; i<numRadiusShells*numShellDirs; i++) {
    sh[i].reset();
  }
}

// Update the shells for particle at position myR based on its relation to particle
// at position neighR.
void shells::update(real3 myR, real3 neighR) {
  // Compute the distance between rA and rB
  real_t dr[3]; // The distance vector
  real_t r2 = 0.0; // The square of the distance
  for(int m=0; m<3; m++) {
    dr[m] = myR[m]-neighR[m];
    r2+=dr[m]*dr[m];
  }
  
  // Find the radius shell within which this neighbor belongs
  int radiusIdx;
  for(radiusIdx=0; radiusIdx<numRadiusShells-1; radiusIdx++) {
    // If r2 fits tightly into the outer boundary of the current shell, we're done
    if(r2>=shellRadiiSq[radiusIdx+1])
      break;
  }
  
  // Find the direction shell within which this neighbor belongs
  int dirIdx=0;
  if(shellDirsPerDim==1) { // Undirected shells
    dirIdx=0;
  } else if(shellDirsPerDim==2) { // Currently, only supported directional option
    // For each dimension, set its bit to 1 if dr in that dimension is non-negative and
    // set its bit to 0 if it is negative.
    for(int m=0; m<3; m++)
      dirIdx <<= (dr[m]>=0? 1: 0);
  } else
    exit(-1);
      
  // The full shell index combines the radius and direction idexes
  int shellIdx = dirIdx*numRadiusShells + radiusIdx;
  
  // Add this neighbor's information to the appropriate shell
  sh[shellIdx].r2 += r2;
  for(int m=0; m<3; m++)
    sh[shellIdx].dr[m] += dr[m];
  sh[shellIdx].count++;
}

// Returns the context associated with shell shellIdx
context shells::getCtxt() const {
  context c;
  for(int shellIdx=0; shellIdx<numRadiusShells*numShellDirs; shellIdx++) {
    c.add(sh[shellIdx].getCtxt(shellIdx));
  }
  return c;
}
#endif

/// \details
/// Call functions such as createFccLattice and setTemperature to set up
/// initial atom positions and momenta.
Atoms* initAtoms(LinkCell* boxes)
{
   Atoms* atoms = (Atoms*)comdMalloc(sizeof(Atoms));

   int maxTotalAtoms = MAXATOMS*boxes->nTotalBoxes;

   atoms->gid =      (int*)   comdMalloc(maxTotalAtoms*sizeof(int));
   atoms->iSpecies = (int*)   comdMalloc(maxTotalAtoms*sizeof(int));
   atoms->r =        (real3*) comdMalloc(maxTotalAtoms*sizeof(real3));
   atoms->p =        (real3*) comdMalloc(maxTotalAtoms*sizeof(real3));
   atoms->f =        (real3*) comdMalloc(maxTotalAtoms*sizeof(real3));
   atoms->U =        (real_t*)comdMalloc(maxTotalAtoms*sizeof(real_t));
   
   #ifdef DISTILL_MONITOR
   //atoms->sh =       (shells*)comdMalloc(maxTotalAtoms*sizeof(shells));
   atoms->sh = new shells[maxTotalAtoms];
   #endif
   
   atoms->nLocal = 0;
   atoms->nGlobal = 0;

   for (int iOff = 0; iOff < maxTotalAtoms; iOff++)
   {
      atoms->gid[iOff] = 0;
      atoms->iSpecies[iOff] = 0;
      zeroReal3(atoms->r[iOff]);
      zeroReal3(atoms->p[iOff]);
      zeroReal3(atoms->f[iOff]);
      atoms->U[iOff] = 0.;
   }
   return atoms;
}

void destroyAtoms(Atoms *atoms)
{
   freeMe(atoms,gid);
   freeMe(atoms,iSpecies);
   freeMe(atoms,r);
   freeMe(atoms,p);
   freeMe(atoms,f);
   freeMe(atoms,U);
   comdFree(atoms);
}

/// Creates atom positions on a face centered cubic (FCC) lattice with
/// nx * ny * nz unit cells and lattice constant lat.
/// Set momenta to zero.
void createFccLattice(int nx, int ny, int nz, real_t lat, SimFlat* s)
{
   const real_t* localMin = s->domain->localMin; // alias
   const real_t* localMax = s->domain->localMax; // alias
   
   int nb = 4; // number of atoms in the basis
   real3 basis[4] = { {0.25, 0.25, 0.25},
      {0.25, 0.75, 0.75},
      {0.75, 0.25, 0.75},
      {0.75, 0.75, 0.25} };

   // create and place atoms
   int begin[3];
   int end[3];
   for (int ii=0; ii<3; ++ii)
   {
      begin[ii] = floor(localMin[ii]/lat);
      end[ii]   = ceil (localMax[ii]/lat);
   }

   real_t px,py,pz;
   px=py=pz=0.0;
   for (int ix=begin[0]; ix<end[0]; ++ix)
      for (int iy=begin[1]; iy<end[1]; ++iy)
         for (int iz=begin[2]; iz<end[2]; ++iz)
            for (int ib=0; ib<nb; ++ib)
            {
               real_t rx = (ix+basis[ib][0]) * lat;
               real_t ry = (iy+basis[ib][1]) * lat;
               real_t rz = (iz+basis[ib][2]) * lat;
               if (rx < localMin[0] || rx >= localMax[0]) continue;
               if (ry < localMin[1] || ry >= localMax[1]) continue;
               if (rz < localMin[2] || rz >= localMax[2]) continue;
               int id = ib+nb*(iz+nz*(iy+ny*(ix)));
               putAtomInBox(s->boxes, s->atoms, id, 0, rx, ry, rz, px, py, pz);
            }

   // set total atoms in simulation
   startTimer(commReduceTimer);
   addIntParallel(&s->atoms->nLocal, &s->atoms->nGlobal, 1);
   stopTimer(commReduceTimer);

   assert(s->atoms->nGlobal == nb*nx*ny*nz);
}

/// Sets the center of mass velocity of the system.
/// \param [in] newVcm The desired center of mass velocity.
void setVcm(SimFlat* s, real_t newVcm[3])
{
   real_t oldVcm[3];
   computeVcm(s, oldVcm);

   real_t vShift[3];
   vShift[0] = (newVcm[0] - oldVcm[0]);
   vShift[1] = (newVcm[1] - oldVcm[1]);
   vShift[2] = (newVcm[2] - oldVcm[2]);

   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t mass = s->species[iSpecies].mass;

         s->atoms->p[iOff][0] += mass * vShift[0];
         s->atoms->p[iOff][1] += mass * vShift[1];
         s->atoms->p[iOff][2] += mass * vShift[2];
      }
   }
}

/// Sets the temperature of system.
///
/// Selects atom velocities randomly from a boltzmann (equilibrium)
/// distribution that corresponds to the specified temperature.  This
/// random process will typically result in a small, but non zero center
/// of mass velocity and a small difference from the specified
/// temperature.  For typical MD runs these small differences are
/// unimportant, However, to avoid possible confusion, we set the center
/// of mass velocity to zero and scale the velocities to exactly match
/// the input temperature.
void setTemperature(SimFlat* s, real_t temperature)
{
   // set initial velocities for the distribution
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         int iType = s->atoms->iSpecies[iOff];
         real_t mass = s->species[iType].mass;
         real_t sigma = sqrt(kB_eV * temperature/mass);
         uint64_t seed = mkSeed(s->atoms->gid[iOff], 123);
         s->atoms->p[iOff][0] = mass * sigma * gasdev(&seed);
         s->atoms->p[iOff][1] = mass * sigma * gasdev(&seed);
         s->atoms->p[iOff][2] = mass * sigma * gasdev(&seed);
      }
   }
   // compute the resulting temperature
   // kinetic energy  = 3/2 kB * Temperature 
   if (temperature == 0.0) return;
   real_t vZero[3] = {0., 0., 0.};
   setVcm(s, vZero);
   kineticEnergy(s);
   real_t temp = (s->eKinetic/s->atoms->nGlobal)/kB_eV/1.5;
   // scale the velocities to achieve the target temperature
   real_t scaleFactor = sqrt(temperature/temp);
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         s->atoms->p[iOff][0] *= scaleFactor;
         s->atoms->p[iOff][1] *= scaleFactor;
         s->atoms->p[iOff][2] *= scaleFactor;
      }
   }
   kineticEnergy(s);
   temp = s->eKinetic/s->atoms->nGlobal/kB_eV/1.5;
}

/// Add a random displacement to the atom positions.
/// Atoms are displaced by a random distance in the range
/// [-delta, +delta] along each axis.
/// \param [in] delta The maximum displacement (along each axis).
void randomDisplacements(SimFlat* s, real_t delta)
{
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         uint64_t seed = mkSeed(s->atoms->gid[iOff], 457);
         s->atoms->r[iOff][0] += (2.0*lcg61(&seed)-1.0) * delta;
         s->atoms->r[iOff][1] += (2.0*lcg61(&seed)-1.0) * delta;
         s->atoms->r[iOff][2] += (2.0*lcg61(&seed)-1.0) * delta;
      }
   }
}

/// Computes the center of mass velocity of the system.
void computeVcm(SimFlat* s, real_t vcm[3])
{
   real_t vcmLocal[4] = {0., 0., 0., 0.};
   real_t vcmSum[4] = {0., 0., 0., 0.};

   // sum the momenta and particle masses 
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; ++iBox)
   {
      for (int iOff=MAXATOMS*iBox, ii=0; ii<s->boxes->nAtoms[iBox]; ++ii, ++iOff)
      {
         vcmLocal[0] += s->atoms->p[iOff][0];
         vcmLocal[1] += s->atoms->p[iOff][1];
         vcmLocal[2] += s->atoms->p[iOff][2];

         int iSpecies = s->atoms->iSpecies[iOff];
         vcmLocal[3] += s->species[iSpecies].mass;
      }
   }

   startTimer(commReduceTimer);
   addRealParallel(vcmLocal, vcmSum, 4);
   stopTimer(commReduceTimer);

   real_t totalMass = vcmSum[3];
   vcm[0] = vcmSum[0]/totalMass;
   vcm[1] = vcmSum[1]/totalMass;
   vcm[2] = vcmSum[2]/totalMass;
}

