#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#if USE_MPI
#include <mpi.h>
#endif
#include "lulesh.h"


#ifdef COMP
compNamedMeasures getMeasures() {
   return compNamedMeasures(
#ifdef RAPL
                              "RAPL", new RAPLMeasure(), noComp()
#else
                              "time", new timeMeasure(), noComp()
#endif
#else // not COMP
namedMeasures getMeasures() {
   return namedMeasures(
#ifdef RAPL
                              "RAPL", new RAPLMeasure()
#else
                              "time", new timeMeasure()
#endif
#endif
                             );
}

/* Helper function for converting strings to ints, with error checking */
int StrToInt(const char *token, int *retVal)
{
   const char *c ;
   char *endptr ;
   const int decimal_base = 10 ;

   if (token == NULL)
      return 0 ;
   
   c = token ;
   *retVal = (int)strtol(c, &endptr, decimal_base) ;
   if((endptr != c) && ((*endptr == ' ') || (*endptr == '\0')))
      return 1 ;
   else
      return 0 ;
}

/* Helper function for converting strings to doubles, with error checking */
int StrToLDbl(const char *token, Real_t *retVal)
{
   const char *c ;
   char *endptr ;

   if (token == NULL)
      return 0 ;
   
   c = token ;
   *retVal = (Real_t)strtold(c, &endptr) ;
   if((endptr != c) && ((*endptr == ' ') || (*endptr == '\0')))
      return 1 ;
   else
      return 0 ;
}


static void PrintCommandLineOptions(char *execname, int myRank)
{
   if (myRank == 0) {

      printf("Usage: %s [opts]\n", execname);
      printf(" where [opts] is one or more of:\n");
      printf(" -q              : quiet mode - suppress all stdout\n");
      printf(" -i <iterations> : number of cycles to run\n");
      printf(" -s <size>       : length of cube mesh along side\n");
      printf(" -r <numregions> : Number of distinct regions (def: 11)\n");
      printf(" -b <balance>    : Load balance between regions of a domain (def: 1)\n");
      printf(" -c <cost>       : Extra cost of more expensive regions (def: 1)\n");
      printf(" -f <numfiles>   : Number of files to split viz dump into (def: (np+10)/9)\n");
      printf(" -p              : Print out progress\n");
      printf(" -v              : Output viz file (requires compiling with -DVIZ_MESH\n");
      printf(" -h              : This message\n");
      printf("\n\n");
   }
}

static void ParseError(const char *message, int myRank)
{
   if (myRank == 0) {
      printf("%s\n", message);
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1);
#else
      exit(-1);
#endif
   }
}

void ParseCommandLineOptions(int argc, char *argv[],
                             int myRank, struct cmdLineOpts *opts)
{
  opts->dtfixed   = Real_t(-1.0e-6) ; // Negative means use courant condition
  opts->dthydro   = Real_t(1.0e+20);
  opts->dtcourant = Real_t(1.0e+20);
  opts->dtmax     = Real_t(1.0e-2);
  opts->powercap = 0;
  #ifdef COMP
  opts->isReference = false;
  #endif
  
  if(argc > 1) {
      int i = 1;
      while(i < argc) {
         int ok;
         printf("argv[%d]=%s\n", i, argv[i]);
         /* -i <iterations> */
         if(strcmp(argv[i], "-i") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -i", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->its));
            if(!ok) {
               ParseError("Parse Error on option -i integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -s <size, sidelength> */
         else if(strcmp(argv[i], "-s") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -s\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->nx));
            if(!ok) {
               ParseError("Parse Error on option -s integer value required after argument\n", myRank);
            }
            i+=2;
         }
         // dtfixed
         else if(strcmp(argv[i], "-dtfixed") == 0) {
            if (i+1 >= argc) ParseError("Missing integer argument to -dtfixed\n", myRank);
            ok = StrToLDbl(argv[i+1], &(opts->dtfixed));
            if(!ok) ParseError("Parse Error on option -dtfixed integer value required after argument\n", myRank);
            i+=2;
         }
         // dthydro
         else if(strcmp(argv[i], "-dthydro") == 0) {
            if (i+1 >= argc) ParseError("Missing integer argument to -dthydro\n", myRank);
            ok = StrToLDbl(argv[i+1], &(opts->dthydro));
            if(!ok) ParseError("Parse Error on option -dthydro integer value required after argument\n", myRank);
            i+=2;
         }
         // dtcourant
         else if(strcmp(argv[i], "-dtcourant") == 0) {
            if (i+1 >= argc) ParseError("Missing integer argument to -dtcourant\n", myRank);
            ok = StrToLDbl(argv[i+1], &(opts->dtcourant));
            if(!ok) ParseError("Parse Error on option -dtcourant integer value required after argument\n", myRank);
            i+=2;
         }
         // dtmax
         else if(strcmp(argv[i], "-dtmax") == 0) {
            if (i+1 >= argc) ParseError("Missing integer argument to -dtmax\n", myRank);
            ok = StrToLDbl(argv[i+1], &(opts->dtmax));
            if(!ok) ParseError("Parse Error on option -dtmax integer value required after argument\n", myRank);
            i+=2;
         }
         // powercap
         else if(strcmp(argv[i], "-powercap") == 0) {
            if (i+1 >= argc) ParseError("Missing integer argument to -powercap\n", myRank);
            ok = StrToLDbl(argv[i+1], &(opts->powercap));
            if(!ok) ParseError("Parse Error on option -powercap integer value required after argument\n", myRank);
            i+=2;
         }
         #ifdef COMP
         /* isReference */
         else if (strcmp(argv[i], "-isReference") == 0) {
            opts->isReference = true;
            i++;
         }
         #endif
	 /* -r <numregions> */
         else if (strcmp(argv[i], "-r") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -r\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->numReg));
            if (!ok) {
               ParseError("Parse Error on option -r integer value required after argument\n", myRank);
            }
            i+=2;
         }
	 /* -f <numfilepieces> */
         else if (strcmp(argv[i], "-f") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -f\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->numFiles));
            if (!ok) {
               ParseError("Parse Error on option -f integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -p */
         else if (strcmp(argv[i], "-p") == 0) {
            opts->showProg = 1;
            i++;
         }
         /* -q */
         else if (strcmp(argv[i], "-q") == 0) {
            opts->quiet = 1;
            i++;
         }
         else if (strcmp(argv[i], "-b") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -b\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->balance));
            if (!ok) {
               ParseError("Parse Error on option -b integer value required after argument\n", myRank);
            }
            i+=2;
         }
         else if (strcmp(argv[i], "-c") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -c\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->cost));
            if (!ok) {
               ParseError("Parse Error on option -c integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -v */
         else if (strcmp(argv[i], "-v") == 0) {
#if VIZ_MESH            
            opts->viz = 1;
#else
            ParseError("Use of -v requires compiling with -DVIZ_MESH\n", myRank);
#endif
            i++;
         }
         /* -h */
         else if (strcmp(argv[i], "-h") == 0) {
            PrintCommandLineOptions(argv[0], myRank);
#if USE_MPI            
            MPI_Abort(MPI_COMM_WORLD, 0);
#else
            exit(0);
#endif
         }
         else {
            char msg[80];
            PrintCommandLineOptions(argv[0], myRank);
            sprintf(msg, "ERROR: Unknown command line argument: %s\n", argv[i]);
            ParseError(msg, myRank);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////

void VerifyAndWriteFinalOutput(Real_t elapsed_time,
                               Domain& locDom,
                               Int_t nx,
                               Int_t numRanks,
                               sightModule* mod, int outputNum, bool verbose)
{
   // GrindTime1 only takes a single domain into account, and is thus a good way to measure
   // processor speed indepdendent of MPI parallelism.
   // GrindTime2 takes into account speedups from MPI parallelism 
   Real_t grindTime1 = ((elapsed_time*1e6)/locDom.cycle())/(nx*nx*nx);
   Real_t grindTime2 = ((elapsed_time*1e6)/locDom.cycle())/(nx*nx*nx*numRanks);

   Index_t ElemId = 0;
   if(verbose) {
      printf("Run completed:  \n");
      printf("   Problem size        =  %i \n",    nx);
      printf("   MPI tasks           =  %i \n",    numRanks);
      printf("   Iteration count     =  %i \n",    locDom.cycle());
      #if defined(REAL10)
      printf("   Final Origin Energy = %12.6Le \n", locDom.e(ElemId));
      #else 
      printf("   Final Origin Energy = %12.6e \n", locDom.e(ElemId));
      #endif
   }

   Real_t   MaxAbsDiff = Real_t(0.0);
   Real_t TotalAbsDiff = Real_t(0.0);
   Real_t   MaxRelDiff = Real_t(0.0);

   for (Index_t j=0; j<nx; ++j) {
      for (Index_t k=j+1; k<nx; ++k) {
         Real_t AbsDiff = FABS(locDom.e(j*nx+k)-locDom.e(k*nx+j));
         TotalAbsDiff  += AbsDiff;

         if (MaxAbsDiff <AbsDiff) MaxAbsDiff = AbsDiff;

         Real_t RelDiff = AbsDiff / locDom.e(k*nx+j);

         if (MaxRelDiff <RelDiff)  MaxRelDiff = RelDiff;
      }
   }

   // Quick symmetry check
   if(verbose) {
      printf("   Testing Plane 0 of Energy Array on rank 0:\n");
      #if defined(REAL10)
      printf("        MaxAbsDiff   = %12.6Le\n",   MaxAbsDiff   );
      printf("        TotalAbsDiff = %12.6Le\n",   TotalAbsDiff );
      printf("        MaxRelDiff   = %12.6Le\n\n", MaxRelDiff   );

      // Timing information
      printf("\nElapsed time         = %10.2Lf (s)\n", elapsed_time);
      printf("Grind time (us/z/c)  = %10.8Lg (per dom)  (%10.8Lg overall)\n", grindTime1, grindTime2);
      printf("FOM                  = %10.8Lg (z/s)\n\n", 1000.0/grindTime2); // zones per second
      #else
      printf("        MaxAbsDiff   = %12.6e\n",   MaxAbsDiff   );
      printf("        TotalAbsDiff = %12.6e\n",   TotalAbsDiff );
      printf("        MaxRelDiff   = %12.6e\n\n", MaxRelDiff   );

      // Timing information
      printf("\nElapsed time         = %10.2f (s)\n", elapsed_time);
      printf("Grind time (us/z/c)  = %10.8g (per dom)  (%10.8g overall)\n", grindTime1, grindTime2);
      printf("FOM                  = %10.8g (z/s)\n\n", 1000.0/grindTime2); // zones per second
      #endif
   }

   if(mod) {
       mod->setOutCtxt(outputNum,
                       compContext("MaxAbsDiff",   (double)MaxAbsDiff,          noComp(),
                                   "TotalAbsDiff", (double)TotalAbsDiff,        noComp(),
                                   "MaxRelDiff",   (double)MaxRelDiff,          noComp(),
                                   "FOM",          (double)(1000.0/grindTime2), noComp()));
   }

   return ;
}