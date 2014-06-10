/// \file
/// Initialize the atom configuration.

#ifndef __INIT_ATOMS_H
#define __INIT_ATOMS_H

#include "mytype.h"
#include "sight.h"

struct SimFlatSt;
struct LinkCellSt;

#ifdef DISTILL_MONITOR
// The number of radius shells around each particle
extern const int numRadiusShells;
extern const double shellRadii[];
// The squares of the values in shellRadii;
extern double shellRadiiSq[];
// The number of direction shells in each dimension around each particle
extern const int shellDirsPerDim;
// The total number of direction shells in all directions
extern const int numShellDirs;

class shells;

class shell {
  friend class shells;
  double r2; // Sum of squares of the distances from the particle to its neighbors in the shell
  real3 dr; // Sum of distances along each dimension from the particle to its neighbors in the shell
  int count; // Number of neighbor particles in the shell
  
  public:
  shell() {}
  shell(double r2, real3 r, int count);

  // Initialize all the properties in the shell to 0  
  void reset();
  
  // Returns the context associated with shell shellIdx
  context getCtxt(int shellIdx) const;
}; // class shell

class shells
{
  protected:
  shell* sh;
  
  public:
  shells();
  
  ~shells();
  
  // Initialize all the properties in all the shells to 0  
  void reset();

  // Update the shells for particle at position myR based on its relation to particle
  // at position neighR.
  void update(real3 myR, real3 neighR);
  
  // Returns the context associated with shell shellIdx
  context getCtxt() const;
}; // class shells
#endif

/// Atom data
typedef struct AtomsSt
{
   // atom-specific data
   int nLocal;    //!< total number of atoms on this processor
   int nGlobal;   //!< total number of atoms in simulation

   int* gid;      //!< A globally unique id for each atom
   int* iSpecies; //!< the species index of the atom

   real3*  r;     //!< positions
   real3*  p;     //!< momenta of atoms
   real3*  f;     //!< forces 
   real_t* U;     //!< potential energy per atom
   
   #ifdef DISTILL_MONITOR
   // The shells around each particle
   shells* sh;
   #endif
} Atoms;


/// Allocates memory to store atom data.
Atoms* initAtoms(struct LinkCellSt* boxes);
void destroyAtoms(struct AtomsSt* atoms);

void createFccLattice(int nx, int ny, int nz, real_t lat, struct SimFlatSt* s);

void setVcm(struct SimFlatSt* s, real_t vcm[3]);
void setTemperature(struct SimFlatSt* s, real_t temperature);
void randomDisplacements(struct SimFlatSt* s, real_t delta);
#endif
