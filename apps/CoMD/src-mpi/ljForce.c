/// \file
/// Computes forces for the 12-6 Lennard Jones (LJ) potential.
///
/// The Lennard-Jones model is not a good representation for the
/// bonding in copper, its use has been limited to constant volume
/// simulations where the embedding energy contribution to the cohesive
/// energy is not included in the two-body potential
///
/// The parameters here are taken from Wolf and Phillpot and fit to the
/// room temperature lattice constant and the bulk melt temperature
/// Ref: D. Wolf and S.Yip eds. Materials Interfaces (Chapman & Hall
///      1992) Page 230.
///
/// Notes on LJ:
///
/// http://en.wikipedia.org/wiki/Lennard_Jones_potential
///
/// The total inter-atomic potential energy in the LJ model is:
///
/// \f[
///   E_{tot} = \sum_{ij} U_{LJ}(r_{ij})
/// \f]
/// \f[
///   U_{LJ}(r_{ij}) = 4 \epsilon
///           \left\{ \left(\frac{\sigma}{r_{ij}}\right)^{12}
///           - \left(\frac{\sigma}{r_{ij}}\right)^6 \right\}
/// \f]
///
/// where \f$\epsilon\f$ and \f$\sigma\f$ are the material parameters in the potential.
///    - \f$\epsilon\f$ = well depth
///    - \f$\sigma\f$   = hard sphere diameter
///
///  To limit the interation range, the LJ potential is typically
///  truncated to zero at some cutoff distance. A common choice for the
///  cutoff distance is 2.5 * \f$\sigma\f$.
///  This implementation can optionally shift the potential slightly
///  upward so the value of the potential is zero at the cuotff
///  distance.  This shift has no effect on the particle dynamics.
///
///
/// The force on atom i is given by
///
/// \f[
///   F_i = -\nabla_i \sum_{jk} U_{LJ}(r_{jk})
/// \f]
///
/// where the subsrcipt i on the gradient operator indicates that the
/// derivatives are taken with respect to the coordinates of atom i.
/// Liberal use of the chain rule leads to the expression
///
/// \f{eqnarray*}{
///   F_i &=& - \sum_j U'_{LJ}(r_{ij})\hat{r}_{ij}\\
///       &=& \sum_j 24 \frac{\epsilon}{r_{ij}} \left\{ 2 \left(\frac{\sigma}{r_{ij}}\right)^{12}
///               - \left(\frac{\sigma}{r_{ij}}\right)^6 \right\} \hat{r}_{ij}
/// \f}
///
/// where \f$\hat{r}_{ij}\f$ is a unit vector in the direction from atom
/// i to atom j.
/// 
///

#include "ljForce.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "constants.h"
#include "mytype.h"
#include "parallel.h"
#include "linkCells.h"
#include "memUtils.h"
#include "CoMDTypes.h"
#include "sight.h"
using namespace sight;
using namespace std;
#define POT_SHIFT 1.0

/// Derived struct for a Lennard Jones potential.
/// Polymorphic with BasePotential.
/// \see BasePotential
typedef struct LjPotentialSt
{
   real_t cutoff;          //!< potential cutoff distance in Angstroms
   real_t mass;            //!< mass of atoms in intenal units
   real_t lat;             //!< lattice spacing (angs) of unit cell
   char latticeType[8];    //!< lattice type, e.g. FCC, BCC, etc.
   char  name[3];	   //!< element name
   int	 atomicNo;	   //!< atomic number  
   int  (*force)(SimFlat* s); //!< function pointer to force routine
   void (*print)(FILE* file, BasePotential* pot);
   void (*destroy)(BasePotential** pot); //!< destruction of the potential
   real_t sigma;
   real_t epsilon;
} LjPotential;

static int ljForce(SimFlat* s);
static void ljPrint(FILE* file, BasePotential* pot);

void ljDestroy(BasePotential** inppot)
{
   if ( ! inppot ) return;
   LjPotential* pot = (LjPotential*)(*inppot);
   if ( ! pot ) return;
   comdFree(pot);
   *inppot = NULL;

   return;
}

/// Initialize an Lennard Jones potential for Copper.
BasePotential* initLjPot(void)
{
   LjPotential *pot = (LjPotential*)comdMalloc(sizeof(LjPotential));
   pot->force = ljForce;
   pot->print = ljPrint;
   pot->destroy = ljDestroy;
   pot->sigma = 2.315;	                  // Angstrom
   pot->epsilon = 0.167;                  // eV
   pot->mass = 63.55 * amuToInternalMass; // Atomic Mass Units (amu)

   pot->lat = 3.615;                      // Equilibrium lattice const in Angs
   strcpy(pot->latticeType, "FCC");       // lattice type, i.e. FCC, BCC, etc.
   pot->cutoff = 2.5*pot->sigma;          // Potential cutoff in Angs

   strcpy(pot->name, "Cu");
   pot->atomicNo = 29;

   return (BasePotential*) pot;
}

void ljPrint(FILE* file, BasePotential* pot)
{
   LjPotential* ljPot = (LjPotential*) pot;
   dbgprintf("<tr><td>Potential type</td><td>Lennard-Jones</td></tr>");
   dbgprintf("<tr><td>Species name</td><td>%s</td></tr>", ljPot->name);
   dbgprintf("<tr><td>Atomic number</td><td>%d</td></tr>", ljPot->atomicNo);
   dbgprintf("<tr><td>Mass</td><td>"FMT1" amu</td></tr>", ljPot->mass / amuToInternalMass); // print in amu
   dbgprintf("<tr><td>Lattice Type</td><td>%s</td></tr>", ljPot->latticeType);
   dbgprintf("<tr><td>Lattice spacing</td><td>"FMT1" Angstroms</td></tr>", ljPot->lat);
   dbgprintf("<tr><td>Cutoff</td><td>"FMT1" Angstroms</td></tr>", ljPot->cutoff);
   dbgprintf("<tr><td>Epsilon</td><td>"FMT1" eV</td></tr>", ljPot->epsilon);
   dbgprintf("<tr><td>Sigma</td><td>"FMT1" Angstroms</td></tr>", ljPot->sigma);
}

int ljForce(SimFlat* s)
{
   LjPotential* pot = (LjPotential *) s->pot;
   real_t sigma = pot->sigma;
   real_t epsilon = pot->epsilon;
   real_t rCut = pot->cutoff;
   real_t rCut2 = rCut*rCut;


//   scope s("LJ Force", scope::high);
#if defined(MODULES)
/*   module ljModule(instance("Lennard Jones", 1, 1), 
                              inputs(port(context("ePotential", s->ePotential,
                                                  "eKinetic",   s->eKinetic))),
                              namedMeasures(
                                  "time", new timeMeasure(),
                                  "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));*/
#endif
   
   // zero forces and energy
   real_t ePot = 0.0;
   s->ePotential = 0.0;
   int fSize = s->boxes->nTotalBoxes*MAXATOMS;
   for (int ii=0; ii<fSize; ++ii)
   {
      zeroReal3(s->atoms->f[ii]);
      s->atoms->U[ii] = 0.;
   }
   
   real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;

   real_t rCut6 = s6 / (rCut2*rCut2*rCut2);
   real_t eShift = POT_SHIFT * rCut6 * (rCut6 - 1.0);

   int nbrBoxes[27];
   
   #ifdef DISTILL_MONITOR
   int numParticlesMonitored=0;
   #endif
   
   // loop over local boxes
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; iBox++)
   {
      int nIBox = s->boxes->nAtoms[iBox];
      if ( nIBox == 0 ) continue;
      
      #ifdef DISTILL_MONITOR
      // Reset the shells state of all the particles in this box
      for (int iOff=iBox*MAXATOMS,ii=0; ii<nIBox; ii++,iOff++)
        s->atoms->sh[iOff].reset();
      #endif

      int nNbrBoxes = getNeighborBoxes(s->boxes, iBox, nbrBoxes);
      // loop over neighbors of iBox
      for (int jTmp=0; jTmp<nNbrBoxes; jTmp++)
      {
         int jBox = nbrBoxes[jTmp];
         
         assert(jBox>=0);
         
         int nJBox = s->boxes->nAtoms[jBox];
         if ( nJBox == 0 ) continue;
         
         // loop over atoms in iBox
         for (int iOff=iBox*MAXATOMS,ii=0; ii<nIBox; ii++,iOff++)
         {
            int iId = s->atoms->gid[iOff];
            // loop over atoms in jBox
            for (int jOff=MAXATOMS*jBox,ij=0; ij<nJBox; ij++,jOff++)
            {
              real_t dr[3];
               int jId = s->atoms->gid[jOff];  
               if (jBox < s->boxes->nLocalBoxes && jId <= iId )
                  continue; // don't double count local-local pairs.

/*               module interactModule(instance("LJ Interaction", 2, 1), 
                              inputs(port(context("iBox", iBox)),
                                     port(context("jBox", jBox))),
                              namedMeasures(
                                  "time", new timeMeasure(),
                                  "PAPI", new PAPIMeasure(papiEvents(PAPI_TOT_INS))));*/

               real_t r2 = 0.0;
               for (int m=0; m<3; m++)
               {
                  dr[m] = s->atoms->r[iOff][m]-s->atoms->r[jOff][m];
                  r2+=dr[m]*dr[m];
               }

               if ( r2 > rCut2) {
/*                  interactModule.setOutCtxt(0, 
                                  context("fX", -1,
                                          "fY", -1,
                                          "fZ", -1));*/
               } else {
               // Important note:
               // from this point on r actually refers to 1.0/r
               r2 = 1.0/r2;
               real_t r6 = s6 * (r2*r2*r2);
               real_t eLocal = r6 * (r6 - 1.0) - eShift;
               s->atoms->U[iOff] += 0.5*eLocal;
               s->atoms->U[jOff] += 0.5*eLocal;

               // calculate energy contribution based on whether
               // the neighbor box is local or remote
               if (jBox < s->boxes->nLocalBoxes)
                  ePot += eLocal;
               else
                  ePot += 0.5 * eLocal;

               // different formulation to avoid sqrt computation
               real_t fr = - 4.0*epsilon*r6*r2*(12.0*r6 - 6.0);
               for (int m=0; m<3; m++)
               {
                  s->atoms->f[iOff][m] -= dr[m]*fr;
                  s->atoms->f[jOff][m] += dr[m]*fr;
               }
               
/*               interactModule.setOutCtxt(0, 
                                  context("fX", s->atoms->f[iOff][0],
                                          "fY", s->atoms->f[iOff][1],
                                          "fZ", s->atoms->f[iOff][2]));*/

               #ifdef DISTILL_MONITOR
               // Update the shell state of particle iOff with its interactions due to particle jOff
               s->atoms->sh[iOff].update(s->atoms->r[iOff], s->atoms->r[jOff]);
               #endif
               }
            } // loop over atoms in jBox
         } // loop over atoms in iBox
      } // loop over neighbor boxes
   
      #ifdef DISTILL_MONITOR
      // Loop over all the atoms within the current box to document their ininital state
      // and computed forces
      //cout << "iBox="<<iBox<<"/"<<s->boxes->nLocalBoxes<<", nIBox="<<nIBox<<endl;
      for (int iOff=iBox*MAXATOMS,ii=0; ii<nIBox; ii++,iOff++) {
        //if(ii%1000==0) { cout << "."; cout.flush(); }
        if(rand()%1000 == 0) {
          context c = s->atoms->sh[iOff].getCtxt();
          c.add(context("r0", s->atoms->r[iOff][0],
                        "r1", s->atoms->r[iOff][1],
                        "r2", s->atoms->r[iOff][2],
                        "p0", s->atoms->p[iOff][0],
                        "p1", s->atoms->p[iOff][1],
                        "p2", s->atoms->p[iOff][2]));

          module particleModule(instance("LJ Particle", 1, 1), 
                                inputs(port(c))
                                );

          particleModule.setOutCtxt(0, 
                                    context("f0", s->atoms->f[iOff][0],
                                            "f1", s->atoms->f[iOff][1],
                                            "f2", s->atoms->f[iOff][2],
                                            "U",  s->atoms->U[iOff]));
        }
      }
      //cout << "\n";
      #endif
      
   } // loop over local boxes in system
   
   ePot = ePot*4.0*epsilon;
   s->ePotential = ePot;
   
#if defined(MODULES)
//   ljModule.setOutCtxt(0, context("ePotential", ePot));
#endif
   
   return 0;
}
