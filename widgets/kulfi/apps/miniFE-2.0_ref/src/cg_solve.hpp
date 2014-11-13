#ifndef _cg_solve_hpp_
#define _cg_solve_hpp_

//@HEADER
// ************************************************************************
//
// MiniFE: Simple Finite Element Assembly and Solve
// Copyright (2006-2013) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// ************************************************************************
//@HEADER

#include <cmath>
#include <limits>

#include <Vector_functions.hpp>
#include <mytimer.hpp>

#include <outstream.hpp>


#ifndef _LINREG
#define _LINREG
#include <iostream>
#include <math.h>
#include <float.h>

class Point2D
{
    public:
        Point2D(double X = 0.0, double Y = 0.0) : x(X), y(Y) { }

        void setPoint(double X, double Y) { x = X; y = Y; }
        void setX(double X) { x = X; }
        void setY(double Y) { y = Y; }

        double getX() const { return x; }
        double getY() const { return y; }

    private:
        double x, y;
};

class LinearRegression
{
    friend std::ostream& operator<<(std::ostream& out, LinearRegression& lr);

    public:
        // Constructor using an array of Point2D objects
        // This is also the default constructor
        LinearRegression(Point2D *p = 0, long size = 0);

        // Constructor using arrays of x values and y values
        LinearRegression(double *x, double *y, long size = 0);

virtual void addXY(const double& x, const double& y);
        void addPoint(const Point2D& p) { addXY(p.getX(), p.getY()); }

        // Must have at least 3 points to calculate
        // standard error of estimate.  Do we have enough data?
        int haveData() const { return (n > 2 ? 1 : 0); }
        long items() const { return n; }

virtual double getA() const { return a; }
virtual double getB() const { return b; }

        double getCoefDeterm() const  { return coefD; }
        double getCoefCorrel() const { return coefC; }
        double getStdErrorEst() const { return stdError; }
virtual double estimateY(double x) const { return (a + b * x); }

    protected:
        long n;             // number of data points input so far
        double sumX, sumY;  // sums of x and y
        double sumXsquared, // sum of x squares
               sumYsquared; // sum y squares
        double sumXY;       // sum of x*y

        double a, b;        // coefficients of f(x) = a + b*x
        double coefD,       // coefficient of determination
               coefC,       // coefficient of correlation
               stdError;    // standard error of estimate

        void Calculate();   // calculate coefficients
};

LinearRegression::LinearRegression(Point2D *p, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addPoint(p[i]);
}

LinearRegression::LinearRegression(double *x, double *y, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addXY(x[i], y[i]);
}

void LinearRegression::addXY(const double& x, const double& y)
{
    n++;
    sumX += x;
    sumY += y;
    sumXsquared += x * x;
    sumYsquared += y * y;
    sumXY += x * y;
    Calculate();
}

void LinearRegression::Calculate()
{
    if (haveData())
    {
        if (fabs( double(n) * sumXsquared - sumX * sumX) > DBL_EPSILON)
        {
            b = ( double(n) * sumXY - sumY * sumX) /
                ( double(n) * sumXsquared - sumX * sumX);
            a = (sumY - b * sumX) / double(n);

            double sx = b * ( sumXY - sumX * sumY / double(n) );
            double sy2 = sumYsquared - sumY * sumY / double(n);
            double sy = sy2 - sx;

            coefD = sx / sy2;
            coefC = sqrt(coefD);
            stdError = sqrt(sy / double(n - 2));
        }
        else
        {
            a = b = coefD = coefC = stdError = 0.0;
        }
    }
}

std::ostream& operator<<(std::ostream& out, LinearRegression& lr)
{
    if (lr.haveData())
        out << "f(x) = " << lr.getA()
            << " + ( " << lr.getB()
            << " * x )";
    return out;
}

#endif





namespace miniFE {

template<typename Scalar>
void print_vec(const std::vector<Scalar>& vec, const std::string& name)
{
  for(size_t i=0; i<vec.size(); ++i) {
    std::cout << name << "["<<i<<"]: " << vec[i] << std::endl;
  }
}

template<typename VectorType>
bool breakdown(typename VectorType::ScalarType inner,
               const VectorType& v,
               const VectorType& w)
{
  typedef typename VectorType::ScalarType Scalar;
  typedef typename TypeTraits<Scalar>::magnitude_type magnitude;

//This is code that was copied from Aztec, and originally written
//by my hero, Ray Tuminaro.
//
//Assuming that inner = <v,w> (inner product of v and w),
//v and w are considered orthogonal if
//  |inner| < 100 * ||v||_2 * ||w||_2 * epsilon

  magnitude vnorm = std::sqrt(dot(v,v));
  magnitude wnorm = std::sqrt(dot(w,w));
  return std::abs(inner) <= 100*vnorm*wnorm*std::numeric_limits<magnitude>::epsilon();
}

template<typename OperatorType,
         typename VectorType,
         typename Matvec>
void
cg_solve(OperatorType& A,
         const VectorType& b,
         VectorType& x,
         Matvec matvec,
         typename OperatorType::LocalOrdinalType max_iter,
         typename TypeTraits<typename OperatorType::ScalarType>::magnitude_type& tolerance,
         typename OperatorType::LocalOrdinalType& num_iters,
         typename TypeTraits<typename OperatorType::ScalarType>::magnitude_type& normr,
         timer_type* my_cg_times, kulfiModule& m, int outputIdx, const std::vector<port>& cfgOutPorts)
{
  typedef typename OperatorType::ScalarType ScalarType;
  typedef typename OperatorType::GlobalOrdinalType GlobalOrdinalType;
  typedef typename OperatorType::LocalOrdinalType LocalOrdinalType;
  typedef typename TypeTraits<ScalarType>::magnitude_type magnitude_type;

  timer_type t0 = 0, tWAXPY = 0, tDOT = 0, tMATVEC = 0, tMATVECDOT = 0;
  timer_type total_time = mytimer();

  LinearRegression residLR;
  
  int myproc = 0;
#ifdef HAVE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &myproc);
#ifdef USE_MPI_PCONTROL
  MPI_Pcontrol(1);
#endif
#endif

  if (!A.has_local_indices) {
    std::cerr << "miniFE::cg_solve ERROR, A.has_local_indices is false, needs to be true. This probably means "
       << "miniFE::make_local_matrix(A) was not called prior to calling miniFE::cg_solve."
       << std::endl;
    return;
  }

  size_t nrows = A.rows.size();
  LocalOrdinalType ncols = A.num_cols;

  VectorType r(b.startIndex, nrows);
  VectorType p(0, ncols);
  VectorType Ap(b.startIndex, nrows);

  normr = 0;
  magnitude_type rtrans = 0;
  magnitude_type oldrtrans = 0;

  LocalOrdinalType print_freq = max_iter/10;
  if (print_freq>50) print_freq = 50;
  if (print_freq<1)  print_freq = 1;

  ScalarType one = 1.0;
  ScalarType zero = 0.0;

  TICK(); waxpby(one, x, zero, x, p); TOCK(tWAXPY);

//  print_vec(p.coefs, "p");

  TICK();
  matvec(A, p, Ap);
  TOCK(tMATVEC);

  TICK(); waxpby(one, b, -one, Ap, r); TOCK(tWAXPY);

  TICK(); rtrans = dot(r, r); TOCK(tDOT);

#if defined(USE_SIGHT)
//std::cout << "rtrans="<<rtrans<<std::endl;
#if !(defined(KULFI))
  trace residTrace("Residual Trace", trace::showBegin, trace::lines);
#endif
#endif
  
  normr = std::sqrt(rtrans);

  if (myproc == 0) {
    std::cout << "Initial Residual = "<< normr << std::endl;
  }
#if defined(USE_SIGHT)
#if !(defined(KULFI))
  traceAttr(&residTrace, trace::ctxtVals("k", 0), trace::observation("normr", normr));
#endif
#endif

  magnitude_type brkdown_tol = std::numeric_limits<magnitude_type>::epsilon();

#ifdef MINIFE_DEBUG
  std::ostream& os = outstream();
  os << "brkdown_tol = " << brkdown_tol << std::endl;
#endif

  for(LocalOrdinalType k=1; k <= max_iter && normr > tolerance; ) {
#if defined(KULFI)
  std::vector<port> cgStepOutputs;
  kulfiModule m(instance(txt()<<"CG Step", 8, 1),
           inputs(cfgOutPorts[0],  // nx, ny, nz
                  cfgOutPorts[1], cfgOutPorts[2], cfgOutPorts[3], cfgOutPorts[4], cfgOutPorts[5],
                  port(context("k", k)),
                  port(compContext("x", sightArray(sightArray::dims(x.coefs.size()), &(x.coefs[0])), LkComp(2, attrValue::floatT, true),
                                   "r", sightArray(sightArray::dims(r.coefs.size()), &(r.coefs[0])), LkComp(2, attrValue::floatT, true),
                                   "p", sightArray(sightArray::dims(p.coefs.size()), &(p.coefs[0])), LkComp(2, attrValue::floatT, true)))),
                  //cfgOutPorts[4]), // load_imbalance
           cgStepOutputs, module::context("EXP_ID", getenv("EXP_ID")), 
           compNamedMeasures("time", new timeMeasure(), LkComp(2, attrValue::floatT, true)));
#else
#endif

  for(LocalOrdinalType k2=1; k2 <= 5 && k <= max_iter && normr > tolerance; ++k2, k++) {
    if (k == 1) {
      TICK(); waxpby(one, r, zero, r, p); TOCK(tWAXPY);
    }
    else {
      oldrtrans = rtrans;
      TICK(); rtrans = dot(r, r); TOCK(tDOT);
      magnitude_type beta = rtrans/oldrtrans;
      TICK(); waxpby(one, r, beta, p, p); TOCK(tWAXPY);
    }

    normr = std::sqrt(rtrans);

    if (myproc == 0 && (k%print_freq==0 || k==max_iter)) {
      std::cout << "Iteration = "<<k<<"   Residual = "<<normr<<std::endl;
    }
    residLR.addPoint(Point2D(k, log(normr)/log(10)));

    magnitude_type alpha = 0;
    magnitude_type p_ap_dot = 0;

#ifdef MINIFE_FUSED
    TICK();
    p_ap_dot = matvec_and_dot(A, p, Ap);
    TOCK(tMATVECDOT);
#else
    TICK(); matvec(A, p, Ap); TOCK(tMATVEC);

    TICK(); p_ap_dot = dot(Ap, p); TOCK(tDOT);
#endif

#ifdef MINIFE_DEBUG
    os << "iter " << k << ", p_ap_dot = " << p_ap_dot;
    os.flush();
#endif
    if (p_ap_dot < brkdown_tol) {
      if (p_ap_dot < 0 || breakdown(p_ap_dot, Ap, p)) {
        std::cerr << "miniFE::cg_solve ERROR, numerical breakdown!"<<std::endl;
#ifdef MINIFE_DEBUG
        os << "ERROR, numerical breakdown!"<<std::endl;
#endif
        //update the timers before jumping out.
        my_cg_times[WAXPY] = tWAXPY;
        my_cg_times[DOT] = tDOT;
        my_cg_times[MATVEC] = tMATVEC;
        my_cg_times[TOTAL] = mytimer() - total_time;
        return;
      }
      else brkdown_tol = 0.1 * p_ap_dot;
    }
    alpha = rtrans/p_ap_dot;
#ifdef MINIFE_DEBUG
    os << ", rtrans = " << rtrans << ", alpha = " << alpha << std::endl;
#endif

#ifdef MINIFE_FUSED
    TICK();
    fused_waxpby(one, x, alpha, p, x, one, r, -alpha, Ap, r);
    TOCK(tWAXPY);
#else
    TICK(); waxpby(one, x, alpha, p, x);
            waxpby(one, r, -alpha, Ap, r); TOCK(tWAXPY);
#endif

#if defined(USE_SIGHT)
#if !(defined(KULFI))
    traceAttr(&residTrace, trace::ctxtVals("k", k), trace::observation("normr", normr));
#endif
#endif

    num_iters = k;
  }
#if defined(USE_SIGHT)
#if defined(KULFI)
  m.setOutCtxt(0, compContext("x", sightArray(sightArray::dims(x.coefs.size()), &(x.coefs[0])), LkComp(2, attrValue::floatT, true),
                              "r", sightArray(sightArray::dims(r.coefs.size()), &(r.coefs[0])), LkComp(2, attrValue::floatT, true),
                              "p", sightArray(sightArray::dims(p.coefs.size()), &(p.coefs[0])), LkComp(2, attrValue::floatT, true)));
#endif
#endif

  }

#ifdef HAVE_MPI
#ifdef USE_MPI_PCONTROL
  MPI_Pcontrol(0);
#endif
#endif

  my_cg_times[WAXPY] = tWAXPY;
  my_cg_times[DOT] = tDOT;
  my_cg_times[MATVEC] = tMATVEC;
  my_cg_times[MATVECDOT] = tMATVECDOT;
  my_cg_times[TOTAL] = mytimer() - total_time;

  // std::cout << "residLR="<<residLR<<std::endl;
  m.setOutCtxt(outputIdx, 
               compContext("residIntercept", residLR.getA(),           noComp(), 
                           "residSlope",     residLR.getB(),           noComp(), 
                           "residLinErr",    residLR.getStdErrorEst(), noComp(), 
                           "residCoeffDet",  residLR.getCoefDeterm(),  noComp(), 
                           "residCoeffCor",  residLR.getCoefCorrel(),  noComp()));
}

}//namespace miniFE



#endif
