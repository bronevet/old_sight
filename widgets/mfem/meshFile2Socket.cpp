//                                MFEM Example 1
//
// Compile with: make ex1
//
// Sample runs:  ex1 ../data/square-disc.mesh
//               ex1 ../data/star.mesh
//               ex1 ../data/escher.mesh
//               ex1 ../data/fichera.mesh
//               ex1 ../data/square-disc-p2.vtk
//               ex1 ../data/square-disc-p3.mesh
//               ex1 ../data/square-disc-nurbs.mesh
//               ex1 ../data/disc-nurbs.mesh
//               ex1 ../data/pipe-nurbs.mesh
//
// Description:  This example code demonstrates the use of MFEM to define a
//               simple isoparametric finite element discretization of the
//               Laplace problem -Delta u = 1 with homogeneous Dirichlet
//               boundary conditions. Specifically, we discretize with the
//               FE space coming from the mesh (linear by default, quadratic
//               for quadratic curvilinear mesh, NURBS for NURBS mesh, etc.)
//
//               The example highlights the use of mesh refinement, finite
//               element grid functions, as well as linear and bilinear forms
//               corresponding to the left-hand side and right-hand side of the
//               discrete linear system. We also cover the explicit elimination
//               of boundary conditions on all boundary edges, and the optional
//               connection to the GLVis tool for visualization.

#include <fstream>
#include "mfem.hpp"

int main (int argc, char *argv[])
{
   Mesh *mesh;

   if (argc == 1)
   {
      cout << "\nUsage: ex1 <mesh_file> <soln_file>\n" << endl;
      return 1;
   }

   // 1. Read the mesh from the given mesh file. 
   ifstream imesh(argv[1]);
   if (!imesh) {
      cerr << "\nCan not open mesh file: " << argv[1] << '\n' << endl;
      return 2;
   }
   mesh = new Mesh(imesh, 1, 1);
   imesh.close();

   // 2. If specified, read the solution from the solution file, providing it the mesh
   GridFunction* soln=NULL;
   if(argc==3) {
      ifstream isoln(argv[2]);
      if(!isoln) {
         cerr << "\nCan not open solution file: " << argv[2] << '\n' << endl;
         return 3;
      }
      soln = new GridFunction(mesh, isoln);
   }

   // 3. Send the mesh and solution by socket to a GLVis server.
   char vishost[] = "localhost";
   int  visport   = 19916;
   osockstream sock(visport, vishost);
   if(!sock.is_open()) {
      cerr << "\nERROR opening socket to the GLVis server at "<<vishost<<":"<<visport<<"!\n";
      return 4;
   }
   sock << "solution\n";
   sock.precision(8);
   mesh->Print(sock);
   if(soln)
      soln->Save(sock);
   sock.send();

   if(soln) delete soln;
   delete mesh;

   return 0;
}
