/***************************************************************************
 * FILE: mpi_wave.c
 * OTHER FILES: draw_wave.c
 * DESCRIPTION:
 *   MPI Concurrent Wave Equation - C Version
 *   Point-to-Point Communications Example
 *   This program implements the concurrent wave equation described
 *   in Chapter 5 of Fox et al., 1988, Solving Problems on Concurrent
 *   Processors, vol 1.
 *   A vibrating string is decomposed into points.  Each processor is
 *   responsible for updating the amplitude of a number of points over
 *   time. At each iteration, each processor exchanges boundary points with
 *   nearest neighbors.  This version uses low level sends and receives
 *   to exchange boundary points.
 *  AUTHOR: Blaise Barney. Adapted from Ros Leibensperger, Cornell Theory
 *    Center. Converted to MPI: George L. Gusciora, MHPCC (1/95)
 * LAST REVISED: 07/05/05
***************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pnmpimod.h>

#include "sight.h"
using namespace sight;
using namespace std;

#define MASTER 0
#define TPOINTS 800
#define MAXSTEPS  10000
#define PI 3.14159265

int RtoL = 10;
int LtoR = 20;
int OUT1 = 30;
int OUT2 = 40;

void init_master(void);
void init_workers(void);
void init_line(void);
void update (int left, int right);
void output_master(void);
void output_workers(void);
extern void draw_wave(double *);

#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define HEIGHT         500
#define WIDTH          1000

typedef struct {
   Window window;
   XSizeHints hints;
   XColor backcolor;
   XColor bordcolor;
   int    bordwidth;
} MYWINDOW;

char baseword[] = {"draw_wave"},
     exitword[] = {"Exit"},
     text[10];

int	taskid,               /* task ID */
	numtasks,             /* number of processes */
	nsteps,               /* number of time steps */
	npoints,              /* number of points handled by this processor */
	first;                /* index of 1st point handled by this processor */
double	etime,                /* elapsed time in seconds */
	values[TPOINTS+2],  /* values at time t */
	oldval[TPOINTS+2],  /* values at time (t-dt) */
	newval[TPOINTS+2];  /* values at time (t+dt) */

/*  ------------------------------------------------------------------------
 *  Master obtains timestep input value from user and broadcasts it
 *  ------------------------------------------------------------------------ */
void init_master(void) {
   char tchar[8];


   /* Set number of number of time steps and then print and broadcast*/
   { scope s(txt() << "User sets the number of time steps");
	   nsteps = 0;
	   while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
		  printf("Enter number of time steps (1-%d): \n",MAXSTEPS);
		  scanf("%s", tchar);
		  nsteps = atoi(tchar);
		  if ((nsteps < 1) || (nsteps > MAXSTEPS))
			 printf("Enter value between 1 and %d\n", MAXSTEPS);
		  }
	   dbg << nsteps << " time steps";
	   }
	   { scope s(txt() << "Share timestep value");
		   MPI_Bcast(&nsteps, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	   }
   }

/*  -------------------------------------------------------------------------
 *  Workers receive timestep input value from master
 *  -------------------------------------------------------------------------*/
void init_workers(void) {
	{ scope s(txt() << "Receive timestep value");
		MPI_Bcast(&nsteps, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	}
}

/*  ------------------------------------------------------------------------
 *  All processes initialize points on line
 *  --------------------------------------------------------------------- */
void init_line(void) {
   int nmin, nleft, npts, i, j, k;
   double x, fac;

   /* calculate initial values based on sine curve */
   {scope("Calculate initial values based on sine curve", scope::minimum);
   }
   nmin = TPOINTS/numtasks;
   nleft = TPOINTS%numtasks;
   fac = 2.0 * PI;
   for (i = 0, k = 0; i < numtasks; i++) {
      npts = (i < nleft) ? nmin + 1 : nmin;
      if (taskid == i) {
         first = k + 1;
         npoints = npts;
         printf ("task=%3d  first point=%5d  npoints=%4d\n", taskid,
                 first, npts);
         for (j = 1; j <= npts; j++, k++) {
            x = (double)k/(double)(TPOINTS - 1);
            values[j] = sin (fac * x);
            }
         }
      else k += npts;
      }
   for (i = 1; i <= npoints; i++)
      oldval[i] = values[i];
   }

/*  -------------------------------------------------------------------------
 *  All processes update their points a specified number of times
 *  -------------------------------------------------------------------------*/
void update(int left, int right) {
   int i, j;
   double dtime, c, dx, tau, sqtau;
   MPI_Status status;

   dtime = 0.3;
   c = 1.0;
   dx = 1.0;
   tau = (c * dtime / dx);
   sqtau = tau * tau;

   /* Update values for each point along string */
   for (i = 1; i <= nsteps; i++) {
      /* Exchange data with "left-hand" neighbor */
      if (first != 1) {
    	  { scope s(txt() << "Exchange data with \"left-hand\" neighbor");
			 MPI_Send(&values[1], 1, MPI_DOUBLE, left, RtoL, MPI_COMM_WORLD);
			 MPI_Recv(&values[0], 1, MPI_DOUBLE, left, LtoR, MPI_COMM_WORLD,
					  &status);
         }
      }
      /* Exchange data with "right-hand" neighbor */
      if (first + npoints -1 != TPOINTS) {
    	  { scope s(txt() << "Exchange data with \"right-hand\" neighbor");
			 MPI_Send(&values[npoints], 1, MPI_DOUBLE, right, LtoR, MPI_COMM_WORLD);
			 MPI_Recv(&values[npoints+1], 1, MPI_DOUBLE, right, RtoL,
					   MPI_COMM_WORLD, &status);
         }
      }
      /* Update points along line */
      {scope(txt()<<"Update points along line", scope::minimum);}
      for (j = 1; j <= npoints; j++) {
         /* Global endpoints */
         if ((first + j - 1 == 1) || (first + j - 1 == TPOINTS))
            newval[j] = 0.0;
         else
            /* Use wave equation to update points */
            newval[j] = (2.0 * values[j]) - oldval[j]
               + (sqtau * (values[j-1] - (2.0 * values[j]) + values[j+1]));
         }
      for (j = 1; j <= npoints; j++) {
         oldval[j] = values[j];
         values[j] = newval[j];
         }
      MPI_Barrier(MPI_COMM_WORLD);

      }
   }

/*  ------------------------------------------------------------------------
 *  Master receives results from workers and prints
 *  ------------------------------------------------------------------------ */
void output_master(void) {
   int i, j, source, start, npts, buffer[2];
   double results[TPOINTS];
   MPI_Status status;

   { scope s(txt() << "Store worker's results");
	   /* Store worker's results in results array */
	   for (i = 1; i < numtasks; i++) {
		   { scope s(txt() << "From Worker " << i);
			  /* Receive first point, number of points and results */
			  MPI_Recv(buffer, 2, MPI_INT, i, OUT1, MPI_COMM_WORLD, &status);
			  start = buffer[0];
			  npts = buffer[1];
			  MPI_Recv(&results[start-1], npts, MPI_DOUBLE, i, OUT2,
					   MPI_COMM_WORLD, &status);
		  }
	   }
   }
   /* Store master's results in results array */
   for (i = first; i < first + npoints; i++)
      results[i-1] = values[i];

   { scope s(txt() <<  "Print final amplitude values for all points" , scope::minimum);
   	   dbg << "Drawing graph";
   }
   j = 0;
   printf("***************************************************************\n");
   printf("Final amplitude values for all points after %d steps:\n",nsteps);
   for (i = 0; i < TPOINTS; i++) {
      printf("%6.2f ", results[i]);
      j = j++;
      if (j == 10) {
         printf("\n");
         j = 0;
         }
      }
   printf("***************************************************************\n");
   printf("\nDrawing graph...\n");
   printf("Click the EXIT button or use CTRL-C to quit\n");

   /* display results with draw_wave routine */
   //draw_wave(&results[0]);
   }

/*  -------------------------------------------------------------------------
 *  Workers send the updated values to the master
 *  -------------------------------------------------------------------------*/

void output_workers(void) {
   int buffer[2];
   MPI_Status status;
   { scope s(txt() << "Back to Master");
   	   dbg << "Send first point, number of points and results to master";
	   /* Send first point, number of points and results to master */
	   buffer[0] = first;
	   buffer[1] = npoints;
	   MPI_Send(&buffer, 2, MPI_INT, MASTER, OUT1, MPI_COMM_WORLD);
	   MPI_Send(&values[1], npoints, MPI_DOUBLE, MASTER, OUT2, MPI_COMM_WORLD);
   }
}

/*  ------------------------------------------------------------------------
 *  Main program
 *  ------------------------------------------------------------------------ */

int main (int argc, char *argv[])
{
int left, right, rc;
PNMPI_modHandle_t handle;
/* Initialize MPI */
MPI_Init(&argc,&argv);
//comparison comp(-1);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
if (numtasks < 2) {
  printf("ERROR: Number of MPI tasks set to %d\n",numtasks);
  printf("Need at least 2 tasks!  Quitting...\n");
  MPI_Abort(MPI_COMM_WORLD, rc);
  exit(0);
  }
{ scope s(txt() << "Task #" << taskid);
}
/* Determine left and right neighbors */
{ scope s(txt() << "Determine neighbors");
	if (taskid == numtasks-1)
	   right = 0;
	else
	   right = taskid + 1;

	if (taskid == 0)
	   left = numtasks - 1;
	else
	   left = taskid - 1;
	dbg << "left : " << left << endl << "right :" << right;
}

/* Get program parameters and initialize wave values */
if (taskid == MASTER) {
	{ scope s(txt() << "Starting mpi_wave using " << numtasks << " tasks", scope::minimum);
	  dbg << "Using " << TPOINTS << " points on the vibrating string";
	}
   printf ("Starting mpi_wave using %d tasks.\n", numtasks);
   printf ("Using %d points on the vibrating string.\n", TPOINTS);
   init_master();
   }
else
   init_workers();

init_line();

/* Update values along the line for nstep time steps */
update(left, right);

/* Master collects results from workers and prints */
if (taskid == MASTER)
   output_master();
else
   output_workers();

MPI_Finalize();
return 0;
}

/* To call from Fortran: uncomment the routine below with the extra underscore in the name
void draw_wave_(double * results) {
*/
/* To call from C, use the routine below - without the extra underscore */
void draw_wave(double * results) {

/* Note extra underscore in draw_wave routine - needed for Fortran */

float 	scale, point, coloratio = 65535.0 / 255.0;
int 	i,j,k,y, zeroaxis, done, myscreen, points[WIDTH];

MYWINDOW base, quit;
Font 	font,font2;
GC 		itemgc,textgc,pointgc,linegc;
XColor 	red,yellow,blue,green,black,white;
XEvent 	myevent;
Colormap cmap;
KeySym 	mykey;
Display *mydisp;


/* Set rgb values for colors */
red.red= (int) (255 * coloratio);
red.green= (int) (0 * coloratio);
red.blue = (int) (0 * coloratio);

yellow.red= (int) (255 * coloratio);
yellow.green= (int) (255 * coloratio);
yellow.blue= (int) (0 * coloratio);

blue.red= (int) (0 * coloratio);
blue.green= (int) (0 * coloratio);
blue.blue= (int) (255 * coloratio);

green.red= (int) (0 * coloratio);
green.green= (int) (255 * coloratio);
green.blue= (int) (0 * coloratio);

black.red= (int) (0 * coloratio);
black.green= (int) (0 * coloratio);
black.blue= (int) (0 * coloratio);

white.red= (int) (255 * coloratio);
white.green= (int) (255 * coloratio);
white.blue= (int) (255 * coloratio);

mydisp = XOpenDisplay("");
if (!mydisp) {
   fprintf (stderr, "Hey! Either you don't have X or something's not right.\n");
   fprintf (stderr, "Guess I won't be showing the graph.  No big deal.\n");
   exit(1);
   }
myscreen = DefaultScreen(mydisp);
cmap = DefaultColormap (mydisp, myscreen);
XAllocColor (mydisp, cmap, &red);
XAllocColor (mydisp, cmap, &yellow);
XAllocColor (mydisp, cmap, &blue);
XAllocColor (mydisp, cmap, &black);
XAllocColor (mydisp, cmap, &green);
XAllocColor (mydisp, cmap, &white);

/* Set up for creating the windows */
/* XCreateSimpleWindow uses defaults for many attributes,   */
/* thereby simplifying the programmer's work in many cases. */

/* base window position and size */
base.hints.x = 50;
base.hints.y = 50;
base.hints.width = WIDTH;
base.hints.height = HEIGHT;
base.hints.flags = PPosition | PSize;
base.bordwidth = 5;

/* window Creation */
/* base window */
base.window = XCreateSimpleWindow (mydisp, DefaultRootWindow (mydisp),
              base.hints.x, base.hints.y, base.hints.width,
              base.hints.height, base.bordwidth, black.pixel,
              black.pixel);
XSetStandardProperties (mydisp, base.window, baseword, baseword, None,
                        NULL, 0, &base.hints);

/* quit window position and size (subwindow of base) */
quit.hints.x = 5;
quit.hints.y = 450;
quit.hints.width = 70;
quit.hints.height = 30;
quit.hints.flags = PPosition | PSize;
quit.bordwidth = 5;

quit.window = XCreateSimpleWindow (mydisp, base.window, quit.hints.x,
              quit.hints.y, quit.hints.width, quit.hints.height,
              quit.bordwidth, green.pixel, yellow.pixel);
XSetStandardProperties (mydisp, quit.window, exitword, exitword, None,
             NULL, 0, &quit.hints);

/* Load fonts */
/*
font = XLoadFont (mydisp, "Rom28");
font2 = XLoadFont (mydisp, "Rom17.500");
*/
font = XLoadFont (mydisp, "fixed");
font2 = XLoadFont (mydisp, "fixed");

/* GC creation and initialization */
textgc = XCreateGC (mydisp, base.window, 0,0);
XSetFont (mydisp, textgc, font);
XSetForeground (mydisp, textgc, white.pixel);

linegc = XCreateGC (mydisp, base.window, 0,0);
XSetForeground (mydisp, linegc, white.pixel);

itemgc = XCreateGC (mydisp, quit.window, 0,0);
XSetFont (mydisp, itemgc, font2);
XSetForeground (mydisp, itemgc, black.pixel);

pointgc = XCreateGC (mydisp, base.window, 0,0);
XSetForeground (mydisp, pointgc, green.pixel);

/* The program is event driven; the XSelectInput call sets which */
/* kinds of interrupts are desired for each window.              */
/* These aren't all used. */
XSelectInput (mydisp, base.window,
              ButtonPressMask | KeyPressMask | ExposureMask);
XSelectInput (mydisp, quit.window,
              ButtonPressMask | KeyPressMask | ExposureMask);

/* window mapping -- this lets windows be displayed */
XMapRaised (mydisp, base.window);
XMapSubwindows (mydisp, base.window);

/* Scale each data point  */
zeroaxis = HEIGHT/2;
scale = (float)zeroaxis;
for(j=0;j<WIDTH;j++)
   points[j]  = zeroaxis - (int)(results[j] * scale);

/* Main event loop  --  exits when user clicks on "exit" */
done = 0;
while (! done) {
   XNextEvent (mydisp, &myevent);   /* Read next event */
   switch (myevent.type) {
   case Expose:
      if (myevent.xexpose.count == 0) {
         if (myevent.xexpose.window == base.window) {
            XDrawString (mydisp, base.window, textgc, 775, 30, "Wave",4);
            XDrawLine (mydisp, base.window, linegc, 1,zeroaxis,WIDTH,
                       zeroaxis);
            for (j=1; j<WIDTH; j++)
               XDrawPoint (mydisp, base.window, pointgc, j, points[j-1]);
            }

         else if (myevent.xexpose.window == quit.window) {
            XDrawString (mydisp, quit.window, itemgc, 12,20, exitword,
                         strlen(exitword));
            }
      }   /* case Expose */
      break;

   case ButtonPress:
      if (myevent.xbutton.window == quit.window)
         done = 1;
      break;

   case KeyPress:
/*
      i = XLookupString (&myevent, text, 10, &mykey, 0);
      if (i == 1 && text[0] == 'q')
         done = 1;
*/
      break;

   case MappingNotify:
/*
      XRefreshKeyboardMapping (&myevent);
*/
      break;

      }  /* switch (myevent.type) */

   }   /* while (! done) */

XDestroyWindow (mydisp, base.window);
XCloseDisplay (mydisp);
}
