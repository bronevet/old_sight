// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443211. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the MFEM library. For more information and source code
// availability see http://mfem.googlecode.com.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

// Finite Element classes

#include <math.h>
#include "fem.hpp"

#include "sight.h"
using namespace sight;


FiniteElement::FiniteElement(int D, int G, int Do, int O, int F)
   : Nodes(Do)
{
   Dim = D ; GeomType = G ; Dof = Do ; Order = O ; FuncSpace = F;
   RangeType = SCALAR;
}

void FiniteElement::CalcVShape (
   const IntegrationPoint &ip, DenseMatrix &shape) const
{
   mfem_error ("FiniteElement::CalcVShape (...)\n"
               "   is not implemented for this class!");
}

void FiniteElement::CalcVShape (
   ElementTransformation &Trans, DenseMatrix &shape) const
{
   mfem_error ("FiniteElement::CalcVShape 2 (...)\n"
               "   is not implemented for this class!");
}

void FiniteElement::CalcDivShape (
   const IntegrationPoint &ip, Vector &divshape) const
{
   mfem_error ("FiniteElement::CalcDivShape (...)\n"
               "   is not implemented for this class!");
}

void FiniteElement::CalcCurlShape(const IntegrationPoint &ip,
                                  DenseMatrix &curl_shape) const
{
   mfem_error ("FiniteElement::CalcCurlShape (...)\n"
               "   is not implemented for this class!");
}

void FiniteElement::GetFaceDofs(int face, int **dofs, int *ndofs) const
{
   mfem_error ("FiniteElement::GetFaceDofs (...)");
}

void FiniteElement::CalcHessian (const IntegrationPoint &ip,
                                 DenseMatrix &h) const
{
   mfem_error ("FiniteElement::CalcHessian (...) is not overloaded !");
}

void FiniteElement::GetLocalInterpolation (ElementTransformation &Trans,
                                           DenseMatrix &I) const
{
   mfem_error ("GetLocalInterpolation (...) is not overloaded !");
}

void FiniteElement::Project (
   Coefficient &coeff, ElementTransformation &Trans, Vector &dofs) const
{
   mfem_error ("FiniteElement::Project (...) is not overloaded !");
}

void FiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   mfem_error ("FiniteElement::Project (...) (vector) is not overloaded !");
}

void FiniteElement::ProjectDelta(int vertex, Vector &dofs) const
{
   mfem_error("FiniteElement::ProjectDelta(...) is not implemented for "
              "this element!");
}

void FiniteElement::Project(
   const FiniteElement &fe, ElementTransformation &Trans, DenseMatrix &I) const
{
   mfem_error("FiniteElement::Project(...) (fe version) is not implemented "
              "for this element!");
}

void FiniteElement::ProjectGrad(
   const FiniteElement &fe, ElementTransformation &Trans,
   DenseMatrix &grad) const
{
   mfem_error("FiniteElement::ProjectGrad(...) is not implemented for "
              "this element!");
}

void FiniteElement::ProjectCurl(
   const FiniteElement &fe, ElementTransformation &Trans,
   DenseMatrix &curl) const
{
   mfem_error("FiniteElement::ProjectCurl(...) is not implemented for "
              "this element!");
}

void FiniteElement::ProjectDiv(
   const FiniteElement &fe, ElementTransformation &Trans,
   DenseMatrix &div) const
{
   mfem_error("FiniteElement::ProjectDiv(...) is not implemented for "
              "this element!");
}


void NodalFiniteElement::NodalLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I,
   const NodalFiniteElement &fine_fe) const
{
   double v[3];
   Vector vv (v, Dim);
   IntegrationPoint f_ip;

#ifdef MFEM_USE_OPENMP
   Vector c_shape(Dof);
#endif

   for (int i = 0; i < fine_fe.Dof; i++)
   {
      Trans.Transform (fine_fe.Nodes.IntPoint (i), vv);
      f_ip.x = v[0];
      if (Dim > 1) { f_ip.y = v[1]; if (Dim > 2) f_ip.z = v[2]; }
      CalcShape (f_ip, c_shape);
      for (int j = 0; j < Dof; j++)
         if (fabs (I (i,j) = c_shape (j)) < 1.0e-12)
            I (i,j) = 0.0;
   }
}

void NodalFiniteElement::Project (
   Coefficient &coeff, ElementTransformation &Trans, Vector &dofs) const
{
   for (int i = 0; i < Dof; i++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(i);
      // some coefficients expect that Trans.IntPoint is the same
      // as the second argument of Eval
      Trans.SetIntPoint(&ip);
      dofs(i) = coeff.Eval (Trans, ip);
   }
}

void NodalFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double v[3];
   Vector x (v, vc.GetVDim());

   for (int i = 0; i < Dof; i++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(i);
      Trans.SetIntPoint(&ip);
      vc.Eval (x, Trans, ip);
      for (int j = 0; j < x.Size(); j++)
         dofs(Dof*j+i) = v[j];
   }
}

void NodalFiniteElement::Project(
   const FiniteElement &fe, ElementTransformation &Trans, DenseMatrix &I) const
{
   if (fe.GetRangeType() == SCALAR)
   {
      Vector shape(fe.GetDof());

      I.SetSize(Dof, fe.GetDof());
      for (int k = 0; k < Dof; k++)
      {
         fe.CalcShape(Nodes.IntPoint(k), shape);
         for (int j = 0; j < shape.Size(); j++)
            I(k,j) = (fabs(shape(j)) < 1e-12) ? 0.0 : shape(j);
      }
   }
   else
   {
      DenseMatrix vshape(fe.GetDof(), Dim);

      I.SetSize(Dim*Dof, fe.GetDof());
      for (int k = 0; k < Dof; k++)
      {
         Trans.SetIntPoint(&Nodes.IntPoint(k));
         fe.CalcVShape(Trans, vshape);
         for (int j = 0; j < vshape.Height(); j++)
            for (int d = 0; d < vshape.Width(); d++)
               I(k+d*Dof,j) = vshape(j,d);
      }
   }
}

void NodalFiniteElement::ProjectGrad(
   const FiniteElement &fe, ElementTransformation &Trans,
   DenseMatrix &grad) const
{
   DenseMatrix dshape(fe.GetDof(), Dim), grad_k(fe.GetDof(), Dim), Jinv(Dim);

   grad.SetSize(Dim*Dof, fe.GetDof());
   for (int k = 0; k < Dof; k++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(k);
      fe.CalcDShape(ip, dshape);
      Trans.SetIntPoint(&ip);
      CalcInverse(Trans.Jacobian(), Jinv);
      Mult(dshape, Jinv, grad_k);
      for (int j = 0; j < grad_k.Height(); j++)
         for (int d = 0; d < Dim; d++)
            grad(k+d*Dof,j) = grad_k(j,d);
   }
}

void NodalFiniteElement::ProjectDiv(
   const FiniteElement &fe,ElementTransformation &Trans,
   DenseMatrix &div) const
{
   double detJ;
   Vector div_shape(fe.GetDof());

   div.SetSize(Dof, fe.GetDof());
   for (int k = 0; k < Dof; k++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(k);
      fe.CalcDivShape(ip, div_shape);
      Trans.SetIntPoint(&ip);
      detJ = Trans.Weight();
      for (int j = 0; j < div_shape.Size(); j++)
         div(k,j) = (fabs(div_shape(j)) < 1e-12) ? 0.0 : div_shape(j)/detJ;
   }
}

void VectorFiniteElement::CalcShape (
   const IntegrationPoint &ip, Vector &shape ) const
{
   mfem_error ("Error: Cannot use scalar CalcShape(...) function with\n"
               "   VectorFiniteElements!");
}

void VectorFiniteElement::CalcDShape (
   const IntegrationPoint &ip, DenseMatrix &dshape ) const
{
   mfem_error ("Error: Cannot use scalar CalcDShape(...) function with\n"
               "   VectorFiniteElements!");
}

void VectorFiniteElement::CalcVShape_RT (
   ElementTransformation &Trans, DenseMatrix &shape) const
{
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
#endif
   CalcVShape(Trans.GetIntPoint(), vshape);

   MultABt(vshape, Trans.Jacobian(), shape);

   shape *= (1.0 / Trans.Weight());
}

void VectorFiniteElement::CalcVShape_ND (
   ElementTransformation &Trans, DenseMatrix &shape) const
{
   const DenseMatrix &J = Trans.Jacobian();

#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(J.Width(), J.Height());
#else
   Jinv.SetSize(J.Width(), J.Height());
#endif

   CalcInverse(J, Jinv);

   CalcVShape(Trans.GetIntPoint(), vshape);

   Mult(vshape, Jinv, shape);
}

void VectorFiniteElement::Project_RT(
   const double *nk, const Array<int> &d2n,
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   double vk[3];
   Vector xk(vk, Dim);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < Dof; k++)
   {
      Trans.SetIntPoint(&Nodes.IntPoint(k));
      // set Jinv = |J| J^{-1} = adj(J)
      CalcAdjugate(Trans.Jacobian(), Jinv);

      vc.Eval(xk, Trans, Nodes.IntPoint(k));
      // dof_k = nk^t adj(J) xk
      dofs(k) = Jinv.InnerProduct(vk, nk + d2n[k]*Dim);
   }
}

void VectorFiniteElement::Project_RT(
   const double *nk, const Array<int> &d2n, const FiniteElement &fe,
   ElementTransformation &Trans, DenseMatrix &I) const
{
   if (fe.GetRangeType() == SCALAR)
   {
      double vk[3];
      Vector shape(fe.GetDof());
#ifdef MFEM_USE_OPENMP
      DenseMatrix Jinv(Dim);
#endif

      I.SetSize(Dof, Dim*fe.GetDof());
      for (int k = 0; k < Dof; k++)
      {
         const IntegrationPoint &ip = Nodes.IntPoint(k);

         fe.CalcShape(ip, shape);
         Trans.SetIntPoint(&ip);
         CalcAdjugateTranspose(Trans.Jacobian(), Jinv);
         Jinv.Mult(nk + d2n[k]*Dim, vk);

         for (int j = 0; j < shape.Size(); j++)
         {
            double s = shape(j);
            if (fabs(s) < 1e-12)
               s = 0.0;
            for (int d = 0; d < Dim; d++)
               I(k,j+d*shape.Size()) = s*vk[d];
         }
      }
   }
   else
   {
      mfem_error("VectorFiniteElement::Project_RT (fe version)");
   }
}

void VectorFiniteElement::ProjectGrad_RT(
   const double *nk, const Array<int> &d2n, const FiniteElement &fe,
   ElementTransformation &Trans, DenseMatrix &grad) const
{
   if (Dim != 2)
      mfem_error("VectorFiniteElement::ProjectGrad_RT works only in 2D!");

   DenseMatrix dshape(fe.GetDof(), fe.GetDim());
   Vector grad_k(fe.GetDof());
   double tk[2];

   grad.SetSize(Dof, fe.GetDof());
   for (int k = 0; k < Dof; k++)
   {
      fe.CalcDShape(Nodes.IntPoint(k), dshape);
      tk[0] = nk[d2n[k]*Dim+1];
      tk[1] = -nk[d2n[k]*Dim];
      dshape.Mult(tk, grad_k);
      for (int j = 0; j < grad_k.Size(); j++)
         grad(k,j) = (fabs(grad_k(j)) < 1e-12) ? 0.0 : grad_k(j);
   }
}

void VectorFiniteElement::ProjectCurl_RT(
   const double *nk, const Array<int> &d2n, const FiniteElement &fe,
   ElementTransformation &Trans, DenseMatrix &curl) const
{
   DenseMatrix curl_shape(fe.GetDof(), Dim);
   Vector curl_k(fe.GetDof());

   curl.SetSize(Dof, fe.GetDof());
   for (int k = 0; k < Dof; k++)
   {
      fe.CalcCurlShape(Nodes.IntPoint(k), curl_shape);
      curl_shape.Mult(nk + d2n[k]*Dim, curl_k);
      for (int j = 0; j < curl_k.Size(); j++)
         curl(k,j) = (fabs(curl_k(j)) < 1e-12) ? 0.0 : curl_k(j);
   }
}

void VectorFiniteElement::Project_ND(
   const double *tk, const Array<int> &d2t,
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   double vk[3];
   Vector xk(vk, vc.GetVDim());

   for (int k = 0; k < Dof; k++)
   {
      Trans.SetIntPoint(&Nodes.IntPoint(k));

      vc.Eval(xk, Trans, Nodes.IntPoint(k));
      // dof_k = xk^t J tk
      dofs(k) = Trans.Jacobian().InnerProduct(tk + d2t[k]*Dim, vk);
   }
}

void VectorFiniteElement::Project_ND(
   const double *tk, const Array<int> &d2t, const FiniteElement &fe,
   ElementTransformation &Trans, DenseMatrix &I) const
{
   if (fe.GetRangeType() == SCALAR)
   {
      double vk[3];
      Vector shape(fe.GetDof());

      I.SetSize(Dof, Dim*fe.GetDof());
      for (int k = 0; k < Dof; k++)
      {
         const IntegrationPoint &ip = Nodes.IntPoint(k);

         fe.CalcShape(ip, shape);
         Trans.SetIntPoint(&ip);
         Trans.Jacobian().Mult(tk + d2t[k]*Dim, vk);

         for (int j = 0; j < shape.Size(); j++)
         {
            double s = shape(j);
            if (fabs(s) < 1e-12)
               s = 0.0;
            for (int d = 0; d < Dim; d++)
               I(k, j + d*shape.Size()) = s*vk[d];
         }
      }
   }
   else
   {
      mfem_error("VectorFiniteElement::Project_ND (fe version)");
   }
}

void VectorFiniteElement::ProjectGrad_ND(
   const double *tk, const Array<int> &d2t, const FiniteElement &fe,
   ElementTransformation &Trans, DenseMatrix &grad) const
{
   DenseMatrix dshape(fe.GetDof(), fe.GetDim());
   Vector grad_k(fe.GetDof());

   grad.SetSize(Dof, fe.GetDof());
   for (int k = 0; k < Dof; k++)
   {
      fe.CalcDShape(Nodes.IntPoint(k), dshape);
      dshape.Mult(tk + d2t[k]*Dim, grad_k);
      for (int j = 0; j < grad_k.Size(); j++)
         grad(k,j) = (fabs(grad_k(j)) < 1e-12) ? 0.0 : grad_k(j);
   }
}

void VectorFiniteElement::LocalInterpolation_RT(
   const double *nk, const Array<int> &d2n, ElementTransformation &Trans,
   DenseMatrix &I) const
{
   double vk[3];
   Vector xk(vk, Dim);
   IntegrationPoint ip;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

   // assuming Trans is linear; this should be ok for all refinement types
   Trans.SetIntPoint(&Geometries.GetCenter(GeomType));
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose(Trans.Jacobian(), Jinv);
   for (int k = 0; k < Dof; k++)
   {
      Trans.Transform(Nodes.IntPoint(k), xk);
      ip.Set3(vk);
      CalcVShape(ip, vshape);
      // xk = |J| J^{-t} n_k
      Jinv.Mult(nk + d2n[k]*Dim, vk);
      // I_k = vshape_k.adj(J)^t.n_k, k=1,...,Dof
      for (int j = 0; j < Dof; j++)
      {
         double Ikj = 0.;
         for (int i = 0; i < Dim; i++)
            Ikj += vshape(j, i) * vk[i];
         I(k, j) = (fabs(Ikj) < 1e-12) ? 0.0 : Ikj;
      }
   }
}

void VectorFiniteElement::LocalInterpolation_ND(
   const double *tk, const Array<int> &d2t, ElementTransformation &Trans,
   DenseMatrix &I) const
{
   double vk[3];
   Vector xk(vk, Dim);
   IntegrationPoint ip;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
#endif

   // assuming Trans is linear; this should be ok for all refinement types
   Trans.SetIntPoint(&Geometries.GetCenter(GeomType));
   const DenseMatrix &J = Trans.Jacobian();
   for (int k = 0; k < Dof; k++)
   {
      Trans.Transform(Nodes.IntPoint(k), xk);
      ip.Set3(vk);
      CalcVShape(ip, vshape);
      // xk = J t_k
      J.Mult(tk + d2t[k]*Dim, vk);
      // I_k = vshape_k.J.t_k, k=1,...,Dof
      for (int j = 0; j < Dof; j++)
      {
         double Ikj = 0.;
         for (int i = 0; i < Dim; i++)
            Ikj += vshape(j, i) * vk[i];
         I(k, j) = (fabs(Ikj) < 1e-12) ? 0.0 : Ikj;
      }
   }
}


PointFiniteElement::PointFiniteElement()
   : NodalFiniteElement(0, Geometry::POINT, 1, 0)
{
   Nodes.IntPoint(0).x = 0.0;
}

void PointFiniteElement::CalcShape(const IntegrationPoint &ip,
                                   Vector &shape) const
{
   shape(0) = 1.;
}

void PointFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                    DenseMatrix &dshape) const
{
   // doesn't make sense
}

Linear1DFiniteElement::Linear1DFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 2, 1)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
}

void Linear1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   shape(0) = 1. - ip.x;
   shape(1) = ip.x;
}

void Linear1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   dshape(0,0) = -1.;
   dshape(1,0) =  1.;
}

Linear2DFiniteElement::Linear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 3, 1)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
}

void Linear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   shape(0) = 1. - ip.x - ip.y;
   shape(1) = ip.x;
   shape(2) = ip.y;
}

void Linear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   dshape(0,0) = -1.; dshape(0,1) = -1.;
   dshape(1,0) =  1.; dshape(1,1) =  0.;
   dshape(2,0) =  0.; dshape(2,1) =  1.;
}

BiLinear2DFiniteElement::BiLinear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE , 4, 1, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 1.0;
}

void BiLinear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                        Vector &shape) const
{
   shape(0) = (1. - ip.x) * (1. - ip.y) ;
   shape(1) = ip.x * (1. - ip.y) ;
   shape(2) = ip.x * ip.y ;
   shape(3) = (1. - ip.x) * ip.y ;
}

void BiLinear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                         DenseMatrix &dshape) const
{
   dshape(0,0) = -1. + ip.y; dshape(0,1) = -1. + ip.x ;
   dshape(1,0) =  1. - ip.y; dshape(1,1) = -ip.x ;
   dshape(2,0) =  ip.y ;     dshape(2,1) = ip.x ;
   dshape(3,0) = -ip.y ;     dshape(3,1) = 1. - ip.x ;
}

void BiLinear2DFiniteElement::CalcHessian(
   const IntegrationPoint &ip, DenseMatrix &h) const
{
   h( 0,0) = 0.;   h( 0,1) =  1.;   h( 0,2) = 0.;
   h( 1,0) = 0.;   h( 1,1) = -1.;   h( 1,2) = 0.;
   h( 2,0) = 0.;   h( 2,1) =  1.;   h( 2,2) = 0.;
   h( 3,0) = 0.;   h( 3,1) = -1.;   h( 3,2) = 0.;
}


GaussLinear2DFiniteElement::GaussLinear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 3, 1, FunctionSpace::Pk)
{
   Nodes.IntPoint(0).x = 1./6.;
   Nodes.IntPoint(0).y = 1./6.;
   Nodes.IntPoint(1).x = 2./3.;
   Nodes.IntPoint(1).y = 1./6.;
   Nodes.IntPoint(2).x = 1./6.;
   Nodes.IntPoint(2).y = 2./3.;
}

void GaussLinear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                           Vector &shape) const
{
   const double x = ip.x, y = ip.y;

   shape(0) = 5./3. - 2. * (x + y);
   shape(1) = 2. * (x - 1./6.);
   shape(2) = 2. * (y - 1./6.);
}

void GaussLinear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                            DenseMatrix &dshape) const
{
   dshape(0,0) = -2.;  dshape(0,1) = -2.;
   dshape(1,0) =  2.;  dshape(1,1) =  0.;
   dshape(2,0) =  0.;  dshape(2,1) =  2.;
}

void GaussLinear2DFiniteElement::ProjectDelta(int vertex, Vector &dofs) const
{
   dofs(vertex)       = 2./3.;
   dofs((vertex+1)%3) = 1./6.;
   dofs((vertex+2)%3) = 1./6.;
}


// 0.5-0.5/sqrt(3) and 0.5+0.5/sqrt(3)
const double GaussBiLinear2DFiniteElement::p[] =
{ 0.2113248654051871177454256, 0.7886751345948128822545744 };

GaussBiLinear2DFiniteElement::GaussBiLinear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE, 4, 1, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = p[0];
   Nodes.IntPoint(0).y = p[0];
   Nodes.IntPoint(1).x = p[1];
   Nodes.IntPoint(1).y = p[0];
   Nodes.IntPoint(2).x = p[1];
   Nodes.IntPoint(2).y = p[1];
   Nodes.IntPoint(3).x = p[0];
   Nodes.IntPoint(3).y = p[1];
}

void GaussBiLinear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   const double x = ip.x, y = ip.y;

   shape(0) = 3. * (p[1] - x) * (p[1] - y);
   shape(1) = 3. * (x - p[0]) * (p[1] - y);
   shape(2) = 3. * (x - p[0]) * (y - p[0]);
   shape(3) = 3. * (p[1] - x) * (y - p[0]);
}

void GaussBiLinear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   const double x = ip.x, y = ip.y;

   dshape(0,0) = 3. * (y - p[1]);  dshape(0,1) = 3. * (x - p[1]);
   dshape(1,0) = 3. * (p[1] - y);  dshape(1,1) = 3. * (p[0] - x);
   dshape(2,0) = 3. * (y - p[0]);  dshape(2,1) = 3. * (x - p[0]);
   dshape(3,0) = 3. * (p[0] - y);  dshape(3,1) = 3. * (p[1] - x);
}

void GaussBiLinear2DFiniteElement::ProjectDelta(int vertex, Vector &dofs) const
{
#if 1
   dofs(vertex)       = p[1]*p[1];
   dofs((vertex+1)%4) = p[0]*p[1];
   dofs((vertex+2)%4) = p[0]*p[0];
   dofs((vertex+3)%4) = p[0]*p[1];
#else
   dofs = 1.0;
#endif
}


P1OnQuadFiniteElement::P1OnQuadFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE , 3, 1, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
}

void P1OnQuadFiniteElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   shape(0) = 1. - ip.x - ip.y;
   shape(1) = ip.x;
   shape(2) = ip.y;
}

void P1OnQuadFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   dshape(0,0) = -1.; dshape(0,1) = -1.;
   dshape(1,0) =  1.; dshape(1,1) =  0.;
   dshape(2,0) =  0.; dshape(2,1) =  1.;
}


Quad1DFiniteElement::Quad1DFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 3, 2)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(2).x = 0.5;
}

void Quad1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                    Vector &shape) const
{
   double x = ip.x;
   double l1 = 1.0 - x, l2 = x, l3 = 2. * x - 1.;

   shape(0) = l1 * (-l3);
   shape(1) = l2 * l3;
   shape(2) = 4. * l1 * l2;
}

void Quad1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                     DenseMatrix &dshape) const
{
   double x = ip.x;

   dshape(0,0) = 4. * x - 3.;
   dshape(1,0) = 4. * x - 1.;
   dshape(2,0) = 4. - 8. * x;
}


QuadPos1DFiniteElement::QuadPos1DFiniteElement()
   : FiniteElement(1, Geometry::SEGMENT, 3, 2)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(2).x = 0.5;
}

void QuadPos1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                       Vector &shape) const
{
   const double x = ip.x, x1 = 1. - x;

   shape(0) = x1 * x1;
   shape(1) = x * x;
   shape(2) = 2. * x * x1;
}

void QuadPos1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                        DenseMatrix &dshape) const
{
   const double x = ip.x;

   dshape(0,0) = 2. * x - 2.;
   dshape(1,0) = 2. * x;
   dshape(2,0) = 2. - 4. * x;
}

Quad2DFiniteElement::Quad2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 6, 2)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.5;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.5;
   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.5;
}

void Quad2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                    Vector &shape) const
{
   double x = ip.x, y = ip.y;
   double l1 = 1.-x-y, l2 = x, l3 = y;

   shape(0) = l1 * (2. * l1 - 1.);
   shape(1) = l2 * (2. * l2 - 1.);
   shape(2) = l3 * (2. * l3 - 1.);
   shape(3) = 4. * l1 * l2;
   shape(4) = 4. * l2 * l3;
   shape(5) = 4. * l3 * l1;
}

void Quad2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                     DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y;

   dshape(0,0) =
      dshape(0,1) = 4. * (x + y) - 3.;

   dshape(1,0) = 4. * x - 1.;
   dshape(1,1) = 0.;

   dshape(2,0) = 0.;
   dshape(2,1) = 4. * y - 1.;

   dshape(3,0) = -4. * (2. * x + y - 1.);
   dshape(3,1) = -4. * x;

   dshape(4,0) = 4. * y;
   dshape(4,1) = 4. * x;

   dshape(5,0) = -4. * y;
   dshape(5,1) = -4. * (x + 2. * y - 1.);
}

void Quad2DFiniteElement::CalcHessian (const IntegrationPoint &ip,
                                       DenseMatrix &h) const
{
   h(0,0) = 4.;
   h(0,1) = 4.;
   h(0,2) = 4.;

   h(1,0) = 4.;
   h(1,1) = 0.;
   h(1,2) = 0.;

   h(2,0) = 0.;
   h(2,1) = 0.;
   h(2,2) = 4.;

   h(3,0) = -8.;
   h(3,1) = -4.;
   h(3,2) =  0.;

   h(4,0) = 0.;
   h(4,1) = 4.;
   h(4,2) = 0.;

   h(5,0) =  0.;
   h(5,1) = -4.;
   h(5,2) = -8.;
}

void Quad2DFiniteElement::ProjectDelta(int vertex, Vector &dofs) const
{
#if 0
   dofs = 1.;
#else
   dofs = 0.;
   dofs(vertex) = 1.;
   switch (vertex)
   {
   case 0: dofs(3) = 0.25; dofs(5) = 0.25; break;
   case 1: dofs(3) = 0.25; dofs(4) = 0.25; break;
   case 2: dofs(4) = 0.25; dofs(5) = 0.25; break;
   }
#endif
}


const double GaussQuad2DFiniteElement::p[] =
{ 0.0915762135097707434595714634022015, 0.445948490915964886318329253883051 };

GaussQuad2DFiniteElement::GaussQuad2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 6, 2), A(6), D(6,2), pol(6)
{
   Nodes.IntPoint(0).x = p[0];
   Nodes.IntPoint(0).y = p[0];
   Nodes.IntPoint(1).x = 1. - 2. * p[0];
   Nodes.IntPoint(1).y = p[0];
   Nodes.IntPoint(2).x = p[0];
   Nodes.IntPoint(2).y = 1. - 2. * p[0];
   Nodes.IntPoint(3).x = p[1];
   Nodes.IntPoint(3).y = p[1];
   Nodes.IntPoint(4).x = 1. - 2. * p[1];
   Nodes.IntPoint(4).y = p[1];
   Nodes.IntPoint(5).x = p[1];
   Nodes.IntPoint(5).y = 1. - 2. * p[1];

   for (int i = 0; i < 6; i++)
   {
      const double x = Nodes.IntPoint(i).x, y = Nodes.IntPoint(i).y;
      A(0,i) = 1.;
      A(1,i) = x;
      A(2,i) = y;
      A(3,i) = x * x;
      A(4,i) = x * y;
      A(5,i) = y * y;
   }

   A.Invert();
}

void GaussQuad2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                         Vector &shape) const
{
   const double x = ip.x, y = ip.y;
   pol(0) = 1.;
   pol(1) = x;
   pol(2) = y;
   pol(3) = x * x;
   pol(4) = x * y;
   pol(5) = y * y;

   A.Mult(pol, shape);
}

void GaussQuad2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                          DenseMatrix &dshape) const
{
   const double x = ip.x, y = ip.y;
   D(0,0) = 0.;      D(0,1) = 0.;
   D(1,0) = 1.;      D(1,1) = 0.;
   D(2,0) = 0.;      D(2,1) = 1.;
   D(3,0) = 2. *  x; D(3,1) = 0.;
   D(4,0) = y;       D(4,1) = x;
   D(5,0) = 0.;      D(5,1) = 2. * y;

   Mult(A, D, dshape);
}


BiQuad2DFiniteElement::BiQuad2DFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE, 9, 2, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(5).x = 1.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(6).x = 0.5;
   Nodes.IntPoint(6).y = 1.0;
   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.5;
}

void BiQuad2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   double x = ip.x, y = ip.y;
   double l1x, l2x, l3x, l1y, l2y, l3y;

   l1x = (x - 1.) * (2. * x - 1);
   l2x = 4. * x * (1. - x);
   l3x = x * (2. * x - 1.);
   l1y = (y - 1.) * (2. * y - 1);
   l2y = 4. * y * (1. - y);
   l3y = y * (2. * y - 1.);

   shape(0) = l1x * l1y;
   shape(4) = l2x * l1y;
   shape(1) = l3x * l1y;
   shape(7) = l1x * l2y;
   shape(8) = l2x * l2y;
   shape(5) = l3x * l2y;
   shape(3) = l1x * l3y;
   shape(6) = l2x * l3y;
   shape(2) = l3x * l3y;
}

void BiQuad2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y;
   double l1x, l2x, l3x, l1y, l2y, l3y;
   double d1x, d2x, d3x, d1y, d2y, d3y;

   l1x = (x - 1.) * (2. * x - 1);
   l2x = 4. * x * (1. - x);
   l3x = x * (2. * x - 1.);
   l1y = (y - 1.) * (2. * y - 1);
   l2y = 4. * y * (1. - y);
   l3y = y * (2. * y - 1.);

   d1x = 4. * x - 3.;
   d2x = 4. - 8. * x;
   d3x = 4. * x - 1.;
   d1y = 4. * y - 3.;
   d2y = 4. - 8. * y;
   d3y = 4. * y - 1.;

   dshape(0,0) = d1x * l1y;
   dshape(0,1) = l1x * d1y;

   dshape(4,0) = d2x * l1y;
   dshape(4,1) = l2x * d1y;

   dshape(1,0) = d3x * l1y;
   dshape(1,1) = l3x * d1y;

   dshape(7,0) = d1x * l2y;
   dshape(7,1) = l1x * d2y;

   dshape(8,0) = d2x * l2y;
   dshape(8,1) = l2x * d2y;

   dshape(5,0) = d3x * l2y;
   dshape(5,1) = l3x * d2y;

   dshape(3,0) = d1x * l3y;
   dshape(3,1) = l1x * d3y;

   dshape(6,0) = d2x * l3y;
   dshape(6,1) = l2x * d3y;

   dshape(2,0) = d3x * l3y;
   dshape(2,1) = l3x * d3y;
}

void BiQuad2DFiniteElement::ProjectDelta(int vertex, Vector &dofs) const
{
#if 0
   dofs = 1.;
#else
   dofs = 0.;
   dofs(vertex) = 1.;
   switch (vertex)
   {
   case 0: dofs(4) = 0.25; dofs(7) = 0.25; break;
   case 1: dofs(4) = 0.25; dofs(5) = 0.25; break;
   case 2: dofs(5) = 0.25; dofs(6) = 0.25; break;
   case 3: dofs(6) = 0.25; dofs(7) = 0.25; break;
   }
   dofs(8) = 1./16.;
#endif
}

BiQuadPos2DFiniteElement::BiQuadPos2DFiniteElement()
   : FiniteElement(2, Geometry::SQUARE, 9, 2, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(5).x = 1.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(6).x = 0.5;
   Nodes.IntPoint(6).y = 1.0;
   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.5;
}

void BiQuadPos2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                         Vector &shape) const
{
   double x = ip.x, y = ip.y;
   double l1x, l2x, l3x, l1y, l2y, l3y;

   l1x = (1. - x) * (1. - x);
   l2x = 2. * x * (1. - x);
   l3x = x * x;
   l1y = (1. - y) * (1. - y);
   l2y = 2. * y * (1. - y);
   l3y = y * y;

   shape(0) = l1x * l1y;
   shape(4) = l2x * l1y;
   shape(1) = l3x * l1y;
   shape(7) = l1x * l2y;
   shape(8) = l2x * l2y;
   shape(5) = l3x * l2y;
   shape(3) = l1x * l3y;
   shape(6) = l2x * l3y;
   shape(2) = l3x * l3y;
}

void BiQuadPos2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                          DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y;
   double l1x, l2x, l3x, l1y, l2y, l3y;
   double d1x, d2x, d3x, d1y, d2y, d3y;

   l1x = (1. - x) * (1. - x);
   l2x = 2. * x * (1. - x);
   l3x = x * x;
   l1y = (1. - y) * (1. - y);
   l2y = 2. * y * (1. - y);
   l3y = y * y;

   d1x = 2. * x - 2.;
   d2x = 2. - 4. * x;
   d3x = 2. * x;
   d1y = 2. * y - 2.;
   d2y = 2. - 4. * y;
   d3y = 2. * y;

   dshape(0,0) = d1x * l1y;
   dshape(0,1) = l1x * d1y;

   dshape(4,0) = d2x * l1y;
   dshape(4,1) = l2x * d1y;

   dshape(1,0) = d3x * l1y;
   dshape(1,1) = l3x * d1y;

   dshape(7,0) = d1x * l2y;
   dshape(7,1) = l1x * d2y;

   dshape(8,0) = d2x * l2y;
   dshape(8,1) = l2x * d2y;

   dshape(5,0) = d3x * l2y;
   dshape(5,1) = l3x * d2y;

   dshape(3,0) = d1x * l3y;
   dshape(3,1) = l1x * d3y;

   dshape(6,0) = d2x * l3y;
   dshape(6,1) = l2x * d3y;

   dshape(2,0) = d3x * l3y;
   dshape(2,1) = l3x * d3y;
}

void BiQuadPos2DFiniteElement::Project(
   Coefficient &coeff, ElementTransformation &Trans, Vector &dofs) const
{
   double *d = dofs;

   for (int i = 0; i < 9; i++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(i);
      Trans.SetIntPoint(&ip);
      d[i] = coeff.Eval(Trans, ip);
   }
   d[4] = 2. * d[4] - 0.5 * (d[0] + d[1]);
   d[5] = 2. * d[5] - 0.5 * (d[1] + d[2]);
   d[6] = 2. * d[6] - 0.5 * (d[2] + d[3]);
   d[7] = 2. * d[7] - 0.5 * (d[3] + d[0]);
   d[8] = 4. * d[8] - 0.5 * (d[4] + d[5] + d[6] + d[7]) -
      0.25 * (d[0] + d[1] + d[2] + d[3]);
}

void BiQuadPos2DFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double v[3];
   Vector x (v, vc.GetVDim());

   for (int i = 0; i < 9; i++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(i);
      Trans.SetIntPoint(&ip);
      vc.Eval (x, Trans, ip);
      for (int j = 0; j < x.Size(); j++)
         dofs(9*j+i) = v[j];
   }
   for (int j = 0; j < x.Size(); j++)
   {
      double *d = &dofs(9*j);

      d[4] = 2. * d[4] - 0.5 * (d[0] + d[1]);
      d[5] = 2. * d[5] - 0.5 * (d[1] + d[2]);
      d[6] = 2. * d[6] - 0.5 * (d[2] + d[3]);
      d[7] = 2. * d[7] - 0.5 * (d[3] + d[0]);
      d[8] = 4. * d[8] - 0.5 * (d[4] + d[5] + d[6] + d[7]) -
         0.25 * (d[0] + d[1] + d[2] + d[3]);
   }
}


GaussBiQuad2DFiniteElement::GaussBiQuad2DFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE, 9, 2, FunctionSpace::Qk)
{
   const double p1 = 0.5*(1.-sqrt(3./5.));

   Nodes.IntPoint(0).x = p1;
   Nodes.IntPoint(0).y = p1;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = p1;
   Nodes.IntPoint(1).x = 1.-p1;
   Nodes.IntPoint(1).y = p1;
   Nodes.IntPoint(7).x = p1;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.5;
   Nodes.IntPoint(5).x = 1.-p1;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(3).x = p1;
   Nodes.IntPoint(3).y = 1.-p1;
   Nodes.IntPoint(6).x = 0.5;
   Nodes.IntPoint(6).y = 1.-p1;
   Nodes.IntPoint(2).x = 1.-p1;
   Nodes.IntPoint(2).y = 1.-p1;
}

void GaussBiQuad2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                           Vector &shape) const
{
   const double a = sqrt(5./3.);
   const double p1 = 0.5*(1.-sqrt(3./5.));

   double x = a*(ip.x-p1), y = a*(ip.y-p1);
   double l1x, l2x, l3x, l1y, l2y, l3y;

   l1x = (x - 1.) * (2. * x - 1);
   l2x = 4. * x * (1. - x);
   l3x = x * (2. * x - 1.);
   l1y = (y - 1.) * (2. * y - 1);
   l2y = 4. * y * (1. - y);
   l3y = y * (2. * y - 1.);

   shape(0) = l1x * l1y;
   shape(4) = l2x * l1y;
   shape(1) = l3x * l1y;
   shape(7) = l1x * l2y;
   shape(8) = l2x * l2y;
   shape(5) = l3x * l2y;
   shape(3) = l1x * l3y;
   shape(6) = l2x * l3y;
   shape(2) = l3x * l3y;
}

void GaussBiQuad2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                            DenseMatrix &dshape) const
{
   const double a = sqrt(5./3.);
   const double p1 = 0.5*(1.-sqrt(3./5.));

   double x = a*(ip.x-p1), y = a*(ip.y-p1);
   double l1x, l2x, l3x, l1y, l2y, l3y;
   double d1x, d2x, d3x, d1y, d2y, d3y;

   l1x = (x - 1.) * (2. * x - 1);
   l2x = 4. * x * (1. - x);
   l3x = x * (2. * x - 1.);
   l1y = (y - 1.) * (2. * y - 1);
   l2y = 4. * y * (1. - y);
   l3y = y * (2. * y - 1.);

   d1x = a * (4. * x - 3.);
   d2x = a * (4. - 8. * x);
   d3x = a * (4. * x - 1.);
   d1y = a * (4. * y - 3.);
   d2y = a * (4. - 8. * y);
   d3y = a * (4. * y - 1.);

   dshape(0,0) = d1x * l1y;
   dshape(0,1) = l1x * d1y;

   dshape(4,0) = d2x * l1y;
   dshape(4,1) = l2x * d1y;

   dshape(1,0) = d3x * l1y;
   dshape(1,1) = l3x * d1y;

   dshape(7,0) = d1x * l2y;
   dshape(7,1) = l1x * d2y;

   dshape(8,0) = d2x * l2y;
   dshape(8,1) = l2x * d2y;

   dshape(5,0) = d3x * l2y;
   dshape(5,1) = l3x * d2y;

   dshape(3,0) = d1x * l3y;
   dshape(3,1) = l1x * d3y;

   dshape(6,0) = d2x * l3y;
   dshape(6,1) = l2x * d3y;

   dshape(2,0) = d3x * l3y;
   dshape(2,1) = l3x * d3y;
}

BiCubic2DFiniteElement::BiCubic2DFiniteElement()
   : NodalFiniteElement (2, Geometry::SQUARE, 16, 3, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.;
   Nodes.IntPoint(0).y = 0.;
   Nodes.IntPoint(1).x = 1.;
   Nodes.IntPoint(1).y = 0.;
   Nodes.IntPoint(2).x = 1.;
   Nodes.IntPoint(2).y = 1.;
   Nodes.IntPoint(3).x = 0.;
   Nodes.IntPoint(3).y = 1.;
   Nodes.IntPoint(4).x = 1./3.;
   Nodes.IntPoint(4).y = 0.;
   Nodes.IntPoint(5).x = 2./3.;
   Nodes.IntPoint(5).y = 0.;
   Nodes.IntPoint(6).x = 1.;
   Nodes.IntPoint(6).y = 1./3.;
   Nodes.IntPoint(7).x = 1.;
   Nodes.IntPoint(7).y = 2./3.;
   Nodes.IntPoint(8).x = 2./3.;
   Nodes.IntPoint(8).y = 1.;
   Nodes.IntPoint(9).x = 1./3.;
   Nodes.IntPoint(9).y = 1.;
   Nodes.IntPoint(10).x = 0.;
   Nodes.IntPoint(10).y = 2./3.;
   Nodes.IntPoint(11).x = 0.;
   Nodes.IntPoint(11).y = 1./3.;
   Nodes.IntPoint(12).x = 1./3.;
   Nodes.IntPoint(12).y = 1./3.;
   Nodes.IntPoint(13).x = 2./3.;
   Nodes.IntPoint(13).y = 1./3.;
   Nodes.IntPoint(14).x = 1./3.;
   Nodes.IntPoint(14).y = 2./3.;
   Nodes.IntPoint(15).x = 2./3.;
   Nodes.IntPoint(15).y = 2./3.;
}

void BiCubic2DFiniteElement::CalcShape(
   const IntegrationPoint &ip, Vector &shape) const
{
   double x = ip.x, y = ip.y;

   double w1x, w2x, w3x, w1y, w2y, w3y;
   double l0x, l1x, l2x, l3x, l0y, l1y, l2y, l3y;

   w1x = x - 1./3.; w2x = x - 2./3.; w3x = x - 1.;
   w1y = y - 1./3.; w2y = y - 2./3.; w3y = y - 1.;

   l0x = (- 4.5) * w1x * w2x * w3x;
   l1x = ( 13.5) *   x * w2x * w3x;
   l2x = (-13.5) *   x * w1x * w3x;
   l3x = (  4.5) *   x * w1x * w2x;

   l0y = (- 4.5) * w1y * w2y * w3y;
   l1y = ( 13.5) *   y * w2y * w3y;
   l2y = (-13.5) *   y * w1y * w3y;
   l3y = (  4.5) *   y * w1y * w2y;

   shape(0)  = l0x * l0y;
   shape(1)  = l3x * l0y;
   shape(2)  = l3x * l3y;
   shape(3)  = l0x * l3y;
   shape(4)  = l1x * l0y;
   shape(5)  = l2x * l0y;
   shape(6)  = l3x * l1y;
   shape(7)  = l3x * l2y;
   shape(8)  = l2x * l3y;
   shape(9)  = l1x * l3y;
   shape(10) = l0x * l2y;
   shape(11) = l0x * l1y;
   shape(12) = l1x * l1y;
   shape(13) = l2x * l1y;
   shape(14) = l1x * l2y;
   shape(15) = l2x * l2y;
}

void BiCubic2DFiniteElement::CalcDShape(
   const IntegrationPoint &ip, DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y;

   double w1x, w2x, w3x, w1y, w2y, w3y;
   double l0x, l1x, l2x, l3x, l0y, l1y, l2y, l3y;
   double d0x, d1x, d2x, d3x, d0y, d1y, d2y, d3y;

   w1x = x - 1./3.; w2x = x - 2./3.; w3x = x - 1.;
   w1y = y - 1./3.; w2y = y - 2./3.; w3y = y - 1.;

   l0x = (- 4.5) * w1x * w2x * w3x;
   l1x = ( 13.5) *   x * w2x * w3x;
   l2x = (-13.5) *   x * w1x * w3x;
   l3x = (  4.5) *   x * w1x * w2x;

   l0y = (- 4.5) * w1y * w2y * w3y;
   l1y = ( 13.5) *   y * w2y * w3y;
   l2y = (-13.5) *   y * w1y * w3y;
   l3y = (  4.5) *   y * w1y * w2y;

   d0x = -5.5 + ( 18. - 13.5 * x) * x;
   d1x =  9.  + (-45. + 40.5 * x) * x;
   d2x = -4.5 + ( 36. - 40.5 * x) * x;
   d3x =  1.  + (- 9. + 13.5 * x) * x;

   d0y = -5.5 + ( 18. - 13.5 * y) * y;
   d1y =  9.  + (-45. + 40.5 * y) * y;
   d2y = -4.5 + ( 36. - 40.5 * y) * y;
   d3y =  1.  + (- 9. + 13.5 * y) * y;

   dshape( 0,0) = d0x * l0y;   dshape( 0,1) = l0x * d0y;
   dshape( 1,0) = d3x * l0y;   dshape( 1,1) = l3x * d0y;
   dshape( 2,0) = d3x * l3y;   dshape( 2,1) = l3x * d3y;
   dshape( 3,0) = d0x * l3y;   dshape( 3,1) = l0x * d3y;
   dshape( 4,0) = d1x * l0y;   dshape( 4,1) = l1x * d0y;
   dshape( 5,0) = d2x * l0y;   dshape( 5,1) = l2x * d0y;
   dshape( 6,0) = d3x * l1y;   dshape( 6,1) = l3x * d1y;
   dshape( 7,0) = d3x * l2y;   dshape( 7,1) = l3x * d2y;
   dshape( 8,0) = d2x * l3y;   dshape( 8,1) = l2x * d3y;
   dshape( 9,0) = d1x * l3y;   dshape( 9,1) = l1x * d3y;
   dshape(10,0) = d0x * l2y;   dshape(10,1) = l0x * d2y;
   dshape(11,0) = d0x * l1y;   dshape(11,1) = l0x * d1y;
   dshape(12,0) = d1x * l1y;   dshape(12,1) = l1x * d1y;
   dshape(13,0) = d2x * l1y;   dshape(13,1) = l2x * d1y;
   dshape(14,0) = d1x * l2y;   dshape(14,1) = l1x * d2y;
   dshape(15,0) = d2x * l2y;   dshape(15,1) = l2x * d2y;
}

void BiCubic2DFiniteElement::CalcHessian(
   const IntegrationPoint &ip, DenseMatrix &h) const
{
   double x = ip.x, y = ip.y;

   double w1x, w2x, w3x, w1y, w2y, w3y;
   double l0x, l1x, l2x, l3x, l0y, l1y, l2y, l3y;
   double d0x, d1x, d2x, d3x, d0y, d1y, d2y, d3y;
   double h0x, h1x, h2x, h3x, h0y, h1y, h2y, h3y;

   w1x = x - 1./3.; w2x = x - 2./3.; w3x = x - 1.;
   w1y = y - 1./3.; w2y = y - 2./3.; w3y = y - 1.;

   l0x = (- 4.5) * w1x * w2x * w3x;
   l1x = ( 13.5) *   x * w2x * w3x;
   l2x = (-13.5) *   x * w1x * w3x;
   l3x = (  4.5) *   x * w1x * w2x;

   l0y = (- 4.5) * w1y * w2y * w3y;
   l1y = ( 13.5) *   y * w2y * w3y;
   l2y = (-13.5) *   y * w1y * w3y;
   l3y = (  4.5) *   y * w1y * w2y;

   d0x = -5.5 + ( 18. - 13.5 * x) * x;
   d1x =  9.  + (-45. + 40.5 * x) * x;
   d2x = -4.5 + ( 36. - 40.5 * x) * x;
   d3x =  1.  + (- 9. + 13.5 * x) * x;

   d0y = -5.5 + ( 18. - 13.5 * y) * y;
   d1y =  9.  + (-45. + 40.5 * y) * y;
   d2y = -4.5 + ( 36. - 40.5 * y) * y;
   d3y =  1.  + (- 9. + 13.5 * y) * y;

   h0x = -27. * x + 18.;
   h1x =  81. * x - 45.;
   h2x = -81. * x + 36.;
   h3x =  27. * x -  9.;

   h0y = -27. * y + 18.;
   h1y =  81. * y - 45.;
   h2y = -81. * y + 36.;
   h3y =  27. * y -  9.;

   h( 0,0) = h0x * l0y;   h( 0,1) = d0x * d0y;   h( 0,2) = l0x * h0y;
   h( 1,0) = h3x * l0y;   h( 1,1) = d3x * d0y;   h( 1,2) = l3x * h0y;
   h( 2,0) = h3x * l3y;   h( 2,1) = d3x * d3y;   h( 2,2) = l3x * h3y;
   h( 3,0) = h0x * l3y;   h( 3,1) = d0x * d3y;   h( 3,2) = l0x * h3y;
   h( 4,0) = h1x * l0y;   h( 4,1) = d1x * d0y;   h( 4,2) = l1x * h0y;
   h( 5,0) = h2x * l0y;   h( 5,1) = d2x * d0y;   h( 5,2) = l2x * h0y;
   h( 6,0) = h3x * l1y;   h( 6,1) = d3x * d1y;   h( 6,2) = l3x * h1y;
   h( 7,0) = h3x * l2y;   h( 7,1) = d3x * d2y;   h( 7,2) = l3x * h2y;
   h( 8,0) = h2x * l3y;   h( 8,1) = d2x * d3y;   h( 8,2) = l2x * h3y;
   h( 9,0) = h1x * l3y;   h( 9,1) = d1x * d3y;   h( 9,2) = l1x * h3y;
   h(10,0) = h0x * l2y;   h(10,1) = d0x * d2y;   h(10,2) = l0x * h2y;
   h(11,0) = h0x * l1y;   h(11,1) = d0x * d1y;   h(11,2) = l0x * h1y;
   h(12,0) = h1x * l1y;   h(12,1) = d1x * d1y;   h(12,2) = l1x * h1y;
   h(13,0) = h2x * l1y;   h(13,1) = d2x * d1y;   h(13,2) = l2x * h1y;
   h(14,0) = h1x * l2y;   h(14,1) = d1x * d2y;   h(14,2) = l1x * h2y;
   h(15,0) = h2x * l2y;   h(15,1) = d2x * d2y;   h(15,2) = l2x * h2y;
}


Cubic1DFiniteElement::Cubic1DFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 4, 3)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(2).x = 0.33333333333333333333;
   Nodes.IntPoint(3).x = 0.66666666666666666667;
}

void Cubic1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   double x = ip.x;
   double l1 = x,
      l2 = (1.0-x),
      l3 = (0.33333333333333333333-x),
      l4 = (0.66666666666666666667-x);

   shape(0) =   4.5 * l2 * l3 * l4;
   shape(1) =   4.5 * l1 * l3 * l4;
   shape(2) =  13.5 * l1 * l2 * l4;
   shape(3) = -13.5 * l1 * l2 * l3;
}

void Cubic1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   double x = ip.x;

   dshape(0,0) = -5.5 + x * (18. - 13.5 * x);
   dshape(1,0) = 1. - x * (9. - 13.5 * x);
   dshape(2,0) = 9. - x * (45. - 40.5 * x);
   dshape(3,0) = -4.5 + x * (36. - 40.5 * x);
}


Cubic2DFiniteElement::Cubic2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 10, 3)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.33333333333333333333;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(4).x = 0.66666666666666666667;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(5).x = 0.66666666666666666667;
   Nodes.IntPoint(5).y = 0.33333333333333333333;
   Nodes.IntPoint(6).x = 0.33333333333333333333;
   Nodes.IntPoint(6).y = 0.66666666666666666667;
   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.66666666666666666667;
   Nodes.IntPoint(8).x = 0.0;
   Nodes.IntPoint(8).y = 0.33333333333333333333;
   Nodes.IntPoint(9).x = 0.33333333333333333333;
   Nodes.IntPoint(9).y = 0.33333333333333333333;
}

void Cubic2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   double x = ip.x, y = ip.y;
   double l1 = (-1. + x + y),
      lx = (-1. + 3.*x),
      ly = (-1. + 3.*y);

   shape(0) = -0.5*l1*(3.*l1 + 1.)*(3.*l1 + 2.);
   shape(1) =  0.5*x*(lx - 1.)*lx;
   shape(2) =  0.5*y*(-1. + ly)*ly;
   shape(3) =  4.5*x*l1*(3.*l1 + 1.);
   shape(4) = -4.5*x*lx*l1;
   shape(5) =  4.5*x*lx*y;
   shape(6) =  4.5*x*y*ly;
   shape(7) = -4.5*y*l1*ly;
   shape(8) =  4.5*y*l1*(1. + 3.*l1);
   shape(9) = -27.*x*y*l1;
}

void Cubic2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y;

   dshape(0,0) =  0.5*(-11. + 36.*y - 9.*(x*(-4. + 3.*x) + 6.*x*y + 3.*y*y));
   dshape(1,0) =  1. + 4.5*x*(-2. + 3.*x);
   dshape(2,0) =  0.;
   dshape(3,0) =  4.5*(2. + 9.*x*x - 5.*y + 3.*y*y + 2.*x*(-5. + 6.*y));
   dshape(4,0) = -4.5*(1. - 1.*y + x*(-8. + 9.*x + 6.*y));
   dshape(5,0) =  4.5*(-1. + 6.*x)*y;
   dshape(6,0) =  4.5*y*(-1. + 3.*y);
   dshape(7,0) =  4.5*(1. - 3.*y)*y;
   dshape(8,0) =  4.5*y*(-5. + 6.*x + 6.*y);
   dshape(9,0) =  -27.*y*(-1. + 2.*x + y);

   dshape(0,1) =  0.5*(-11. + 36.*y - 9.*(x*(-4. + 3.*x) + 6.*x*y + 3.*y*y));
   dshape(1,1) =  0.;
   dshape(2,1) =  1. + 4.5*y*(-2. + 3.*y);
   dshape(3,1) =  4.5*x*(-5. + 6.*x + 6.*y);
   dshape(4,1) =  4.5*(1. - 3.*x)*x;
   dshape(5,1) =  4.5*x*(-1. + 3.*x);
   dshape(6,1) =  4.5*x*(-1. + 6.*y);
   dshape(7,1) = -4.5*(1. + x*(-1. + 6.*y) + y*(-8. + 9.*y));
   dshape(8,1) =  4.5*(2. + 3.*x*x + y*(-10. + 9.*y) + x*(-5. + 12.*y));
   dshape(9,1) = -27.*x*(-1. + x + 2.*y);
}

void Cubic2DFiniteElement::CalcHessian (const IntegrationPoint &ip,
                                        DenseMatrix &h) const
{
   double x = ip.x, y = ip.y;

   h(0,0) = 18.-27.*(x+y);
   h(0,1) = 18.-27.*(x+y);
   h(0,2) = 18.-27.*(x+y);

   h(1,0) = -9.+27.*x;
   h(1,1) = 0.;
   h(1,2) = 0.;

   h(2,0) = 0.;
   h(2,1) = 0.;
   h(2,2) = -9.+27.*y;

   h(3,0) = -45.+81.*x+54.*y;
   h(3,1) = -22.5+54.*x+27.*y;
   h(3,2) = 27.*x;

   h(4,0) = 36.-81.*x-27.*y;
   h(4,1) = 4.5-27.*x;
   h(4,2) = 0.;

   h(5,0) = 27.*y;
   h(5,1) = -4.5+27.*x;
   h(5,2) = 0.;

   h(6,0) = 0.;
   h(6,1) = -4.5+27.*y;
   h(6,2) = 27.*x;

   h(7,0) = 0.;
   h(7,1) = 4.5-27.*y;
   h(7,2) = 36.-27.*x-81.*y;

   h(8,0) = 27.*y;
   h(8,1) = -22.5+27.*x+54.*y;
   h(8,2) = -45.+54.*x+81.*y;

   h(9,0) = -54.*y;
   h(9,1) = 27.-54.*(x+y);
   h(9,2) = -54.*x;
}


Cubic3DFiniteElement::Cubic3DFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, 20, 3)
{
   Nodes.IntPoint(0).x = 0;
   Nodes.IntPoint(0).y = 0;
   Nodes.IntPoint(0).z = 0;
   Nodes.IntPoint(1).x = 1.;
   Nodes.IntPoint(1).y = 0;
   Nodes.IntPoint(1).z = 0;
   Nodes.IntPoint(2).x = 0;
   Nodes.IntPoint(2).y = 1.;
   Nodes.IntPoint(2).z = 0;
   Nodes.IntPoint(3).x = 0;
   Nodes.IntPoint(3).y = 0;
   Nodes.IntPoint(3).z = 1.;
   Nodes.IntPoint(4).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(4).y = 0;
   Nodes.IntPoint(4).z = 0;
   Nodes.IntPoint(5).x = 0.6666666666666666666666666667;
   Nodes.IntPoint(5).y = 0;
   Nodes.IntPoint(5).z = 0;
   Nodes.IntPoint(6).x = 0;
   Nodes.IntPoint(6).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(6).z = 0;
   Nodes.IntPoint(7).x = 0;
   Nodes.IntPoint(7).y = 0.6666666666666666666666666667;
   Nodes.IntPoint(7).z = 0;
   Nodes.IntPoint(8).x = 0;
   Nodes.IntPoint(8).y = 0;
   Nodes.IntPoint(8).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(9).x = 0;
   Nodes.IntPoint(9).y = 0;
   Nodes.IntPoint(9).z = 0.6666666666666666666666666667;
   Nodes.IntPoint(10).x = 0.6666666666666666666666666667;
   Nodes.IntPoint(10).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(10).z = 0;
   Nodes.IntPoint(11).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(11).y = 0.6666666666666666666666666667;
   Nodes.IntPoint(11).z = 0;
   Nodes.IntPoint(12).x = 0.6666666666666666666666666667;
   Nodes.IntPoint(12).y = 0;
   Nodes.IntPoint(12).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(13).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(13).y = 0;
   Nodes.IntPoint(13).z = 0.6666666666666666666666666667;
   Nodes.IntPoint(14).x = 0;
   Nodes.IntPoint(14).y = 0.6666666666666666666666666667;
   Nodes.IntPoint(14).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(15).x = 0;
   Nodes.IntPoint(15).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(15).z = 0.6666666666666666666666666667;
   Nodes.IntPoint(16).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(16).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(16).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(17).x = 0;
   Nodes.IntPoint(17).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(17).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(18).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(18).y = 0;
   Nodes.IntPoint(18).z = 0.3333333333333333333333333333;
   Nodes.IntPoint(19).x = 0.3333333333333333333333333333;
   Nodes.IntPoint(19).y = 0.3333333333333333333333333333;
   Nodes.IntPoint(19).z = 0;
}

void Cubic3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;

   shape(0) = -((-1 + x + y + z)*(-2 + 3*x + 3*y + 3*z)*
                (-1 + 3*x + 3*y + 3*z))/2.;
   shape(4) = (9*x*(-1 + x + y + z)*(-2 + 3*x + 3*y + 3*z))/2.;
   shape(5) = (-9*x*(-1 + 3*x)*(-1 + x + y + z))/2.;
   shape(1) = (x*(2 + 9*(-1 + x)*x))/2.;
   shape(6) = (9*y*(-1 + x + y + z)*(-2 + 3*x + 3*y + 3*z))/2.;
   shape(19) = -27*x*y*(-1 + x + y + z);
   shape(10) = (9*x*(-1 + 3*x)*y)/2.;
   shape(7) = (-9*y*(-1 + 3*y)*(-1 + x + y + z))/2.;
   shape(11) = (9*x*y*(-1 + 3*y))/2.;
   shape(2) = (y*(2 + 9*(-1 + y)*y))/2.;
   shape(8) = (9*z*(-1 + x + y + z)*(-2 + 3*x + 3*y + 3*z))/2.;
   shape(18) = -27*x*z*(-1 + x + y + z);
   shape(12) = (9*x*(-1 + 3*x)*z)/2.;
   shape(17) = -27*y*z*(-1 + x + y + z);
   shape(16) = 27*x*y*z;
   shape(14) = (9*y*(-1 + 3*y)*z)/2.;
   shape(9) = (-9*z*(-1 + x + y + z)*(-1 + 3*z))/2.;
   shape(13) = (9*x*z*(-1 + 3*z))/2.;
   shape(15) = (9*y*z*(-1 + 3*z))/2.;
   shape(3) = (z*(2 + 9*(-1 + z)*z))/2.;
}

void Cubic3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y, z = ip.z;

   dshape(0,0) = (-11 + 36*y + 36*z - 9*(3*pow(x,2) + 3*pow(y + z,2) +
                                         x*(-4 + 6*y + 6*z)))/2.;
   dshape(0,1) = (-11 + 36*y + 36*z - 9*(3*pow(x,2) + 3*pow(y + z,2) +
                                         x*(-4 + 6*y + 6*z)))/2.;
   dshape(0,2) = (-11 + 36*y + 36*z - 9*(3*pow(x,2) + 3*pow(y + z,2) +
                                         x*(-4 + 6*y + 6*z)))/2.;
   dshape(4,0) = (9*(9*pow(x,2) + (-1 + y + z)*(-2 + 3*y + 3*z) +
                     2*x*(-5 + 6*y + 6*z)))/2.;
   dshape(4,1) = (9*x*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(4,2) = (9*x*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(5,0) = (-9*(1 - y - z + x*(-8 + 9*x + 6*y + 6*z)))/2.;
   dshape(5,1) = (9*(1 - 3*x)*x)/2.;
   dshape(5,2) = (9*(1 - 3*x)*x)/2.;
   dshape(1,0) = 1 + (9*x*(-2 + 3*x))/2.;
   dshape(1,1) = 0;
   dshape(1,2) = 0;
   dshape(6,0) = (9*y*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(6,1) = (9*(2 + 3*pow(x,2) - 10*y - 5*z + 3*(y + z)*(3*y + z) +
                     x*(-5 + 12*y + 6*z)))/2.;
   dshape(6,2) = (9*y*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(19,0) = -27*y*(-1 + 2*x + y + z);
   dshape(19,1) = -27*x*(-1 + x + 2*y + z);
   dshape(19,2) = -27*x*y;
   dshape(10,0) = (9*(-1 + 6*x)*y)/2.;
   dshape(10,1) = (9*x*(-1 + 3*x))/2.;
   dshape(10,2) = 0;
   dshape(7,0) = (9*(1 - 3*y)*y)/2.;
   dshape(7,1) = (-9*(1 + x*(-1 + 6*y) - z + y*(-8 + 9*y + 6*z)))/2.;
   dshape(7,2) = (9*(1 - 3*y)*y)/2.;
   dshape(11,0) = (9*y*(-1 + 3*y))/2.;
   dshape(11,1) = (9*x*(-1 + 6*y))/2.;
   dshape(11,2) = 0;
   dshape(2,0) = 0;
   dshape(2,1) = 1 + (9*y*(-2 + 3*y))/2.;
   dshape(2,2) = 0;
   dshape(8,0) = (9*z*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(8,1) = (9*z*(-5 + 6*x + 6*y + 6*z))/2.;
   dshape(8,2) = (9*(2 + 3*pow(x,2) - 5*y - 10*z + 3*(y + z)*(y + 3*z) +
                     x*(-5 + 6*y + 12*z)))/2.;
   dshape(18,0) = -27*z*(-1 + 2*x + y + z);
   dshape(18,1) = -27*x*z;
   dshape(18,2) = -27*x*(-1 + x + y + 2*z);
   dshape(12,0) = (9*(-1 + 6*x)*z)/2.;
   dshape(12,1) = 0;
   dshape(12,2) = (9*x*(-1 + 3*x))/2.;
   dshape(17,0) = -27*y*z;
   dshape(17,1) = -27*z*(-1 + x + 2*y + z);
   dshape(17,2) = -27*y*(-1 + x + y + 2*z);
   dshape(16,0) = 27*y*z;
   dshape(16,1) = 27*x*z;
   dshape(16,2) = 27*x*y;
   dshape(14,0) = 0;
   dshape(14,1) = (9*(-1 + 6*y)*z)/2.;
   dshape(14,2) = (9*y*(-1 + 3*y))/2.;
   dshape(9,0) = (9*(1 - 3*z)*z)/2.;
   dshape(9,1) = (9*(1 - 3*z)*z)/2.;
   dshape(9,2) = (9*(-1 + x + y + 8*z - 6*(x + y)*z - 9*pow(z,2)))/2.;
   dshape(13,0) = (9*z*(-1 + 3*z))/2.;
   dshape(13,1) = 0;
   dshape(13,2) = (9*x*(-1 + 6*z))/2.;
   dshape(15,0) = 0;
   dshape(15,1) = (9*z*(-1 + 3*z))/2.;
   dshape(15,2) = (9*y*(-1 + 6*z))/2.;
   dshape(3,0) = 0;
   dshape(3,1) = 0;
   dshape(3,2) = 1 + (9*z*(-2 + 3*z))/2.;
}


P0TriangleFiniteElement::P0TriangleFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE , 1, 0)
{
   Nodes.IntPoint(0).x = 0.333333333333333333;
   Nodes.IntPoint(0).y = 0.333333333333333333;
}

void P0TriangleFiniteElement::CalcShape(const IntegrationPoint &ip,
                                        Vector &shape) const
{
   shape(0) = 1.0;
}

void P0TriangleFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                         DenseMatrix &dshape) const
{
   dshape(0,0) = 0.0;
   dshape(0,1) = 0.0;
}


P0QuadFiniteElement::P0QuadFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE , 1, 0, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.5;
}

void P0QuadFiniteElement::CalcShape(const IntegrationPoint &ip,
                                    Vector &shape) const
{
   shape(0) = 1.0;
}

void P0QuadFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                     DenseMatrix &dshape) const
{
   dshape(0,0) = 0.0;
   dshape(0,1) = 0.0;
}


Linear3DFiniteElement::Linear3DFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, 4, 1)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(2).z = 0.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(3).z = 1.0;
}

void Linear3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   shape(0) = 1. - ip.x - ip.y - ip.z;
   shape(1) = ip.x;
   shape(2) = ip.y;
   shape(3) = ip.z;
}

void Linear3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   if (dshape.Height() == 4)
   {
      double *A = &dshape(0,0);
      A[0] = -1.; A[4] = -1.; A[8]  = -1.;
      A[1] =  1.; A[5] =  0.; A[9]  =  0.;
      A[2] =  0.; A[6] =  1.; A[10] =  0.;
      A[3] =  0.; A[7] =  0.; A[11] =  1.;
   }
   else
   {
      dshape(0,0) = -1.; dshape(0,1) = -1.; dshape(0,2) = -1.;
      dshape(1,0) =  1.; dshape(1,1) =  0.; dshape(1,2) =  0.;
      dshape(2,0) =  0.; dshape(2,1) =  1.; dshape(2,2) =  0.;
      dshape(3,0) =  0.; dshape(3,1) =  0.; dshape(3,2) =  1.;
   }
}

void Linear3DFiniteElement::GetFaceDofs (int face, int **dofs, int *ndofs)
   const
{
   static int face_dofs[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};

   *ndofs = 3;
   *dofs  = face_dofs[face];
}


Quadratic3DFiniteElement::Quadratic3DFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, 10, 2)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(2).z = 0.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(3).z = 1.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(4).z = 0.0;
   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 0.0;
   Nodes.IntPoint(6).x = 0.0;
   Nodes.IntPoint(6).y = 0.0;
   Nodes.IntPoint(6).z = 0.5;
   Nodes.IntPoint(7).x = 0.5;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(7).z = 0.0;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.0;
   Nodes.IntPoint(8).z = 0.5;
   Nodes.IntPoint(9).x = 0.0;
   Nodes.IntPoint(9).y = 0.5;
   Nodes.IntPoint(9).z = 0.5;
}

void Quadratic3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                         Vector &shape) const
{
   double L0, L1, L2, L3;

   L0 = 1. - ip.x - ip.y - ip.z;
   L1 = ip.x;
   L2 = ip.y;
   L3 = ip.z;

   shape(0) = L0 * ( 2.0 * L0 - 1.0 );
   shape(1) = L1 * ( 2.0 * L1 - 1.0 );
   shape(2) = L2 * ( 2.0 * L2 - 1.0 );
   shape(3) = L3 * ( 2.0 * L3 - 1.0 );
   shape(4) = 4.0 * L0 * L1;
   shape(5) = 4.0 * L0 * L2;
   shape(6) = 4.0 * L0 * L3;
   shape(7) = 4.0 * L1 * L2;
   shape(8) = 4.0 * L1 * L3;
   shape(9) = 4.0 * L2 * L3;
}

void Quadratic3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                          DenseMatrix &dshape) const
{
   double x, y, z, L0;

   x = ip.x;
   y = ip.y;
   z = ip.z;
   L0 = 1.0 - x - y - z;

   dshape(0,0) = dshape(0,1) = dshape(0,2) = 1.0 - 4.0 * L0;
   dshape(1,0) = -1.0 + 4.0 * x; dshape(1,1) = 0.0; dshape(1,2) = 0.0;
   dshape(2,0) = 0.0; dshape(2,1) = -1.0 + 4.0 * y; dshape(2,2) = 0.0;
   dshape(3,0) = dshape(3,1) = 0.0; dshape(3,2) = -1.0 + 4.0 * z;
   dshape(4,0) = 4.0 * (L0 - x); dshape(4,1) = dshape(4,2) = -4.0 * x;
   dshape(5,0) = dshape(5,2) = -4.0 * y; dshape(5,1) = 4.0 * (L0 - y);
   dshape(6,0) = dshape(6,1) = -4.0 * z; dshape(6,2) = 4.0 * (L0 - z);
   dshape(7,0) = 4.0 * y; dshape(7,1) = 4.0 * x; dshape(7,2) = 0.0;
   dshape(8,0) = 4.0 * z; dshape(8,1) = 0.0; dshape(8,2) = 4.0 * x;
   dshape(9,0) = 0.0; dshape(9,1) = 4.0 * z; dshape(9,2) = 4.0 * y;
}

TriLinear3DFiniteElement::TriLinear3DFiniteElement()
   : NodalFiniteElement(3, Geometry::CUBE, 8, 1, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;

   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.0;

   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(2).z = 0.0;

   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(3).z = 0.0;

   Nodes.IntPoint(4).x = 0.0;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(4).z = 1.0;

   Nodes.IntPoint(5).x = 1.0;
   Nodes.IntPoint(5).y = 0.0;
   Nodes.IntPoint(5).z = 1.0;

   Nodes.IntPoint(6).x = 1.0;
   Nodes.IntPoint(6).y = 1.0;
   Nodes.IntPoint(6).z = 1.0;

   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 1.0;
   Nodes.IntPoint(7).z = 1.0;
}

void TriLinear3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                         Vector &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;
   double ox = 1.-x, oy = 1.-y, oz = 1.-z;

   shape(0) = ox * oy * oz;
   shape(1) =  x * oy * oz;
   shape(2) =  x *  y * oz;
   shape(3) = ox *  y * oz;
   shape(4) = ox * oy *  z;
   shape(5) =  x * oy *  z;
   shape(6) =  x *  y *  z;
   shape(7) = ox *  y *  z;
}

void TriLinear3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                          DenseMatrix &dshape) const
{
   double x = ip.x, y = ip.y, z = ip.z;
   double ox = 1.-x, oy = 1.-y, oz = 1.-z;

   dshape(0,0) = - oy * oz;
   dshape(0,1) = - ox * oz;
   dshape(0,2) = - ox * oy;

   dshape(1,0) =   oy * oz;
   dshape(1,1) = -  x * oz;
   dshape(1,2) = -  x * oy;

   dshape(2,0) =    y * oz;
   dshape(2,1) =    x * oz;
   dshape(2,2) = -  x *  y;

   dshape(3,0) = -  y * oz;
   dshape(3,1) =   ox * oz;
   dshape(3,2) = - ox *  y;

   dshape(4,0) = - oy *  z;
   dshape(4,1) = - ox *  z;
   dshape(4,2) =   ox * oy;

   dshape(5,0) =   oy *  z;
   dshape(5,1) = -  x *  z;
   dshape(5,2) =    x * oy;

   dshape(6,0) =    y *  z;
   dshape(6,1) =    x *  z;
   dshape(6,2) =    x *  y;

   dshape(7,0) = -  y *  z;
   dshape(7,1) =   ox *  z;
   dshape(7,2) =   ox *  y;
}

P0SegmentFiniteElement::P0SegmentFiniteElement(int Ord)
   : NodalFiniteElement(1, Geometry::SEGMENT , 1, Ord)  // defaul Ord = 0
{
   Nodes.IntPoint(0).x = 0.5;
}

void P0SegmentFiniteElement::CalcShape(const IntegrationPoint &ip,
                                       Vector &shape) const
{
   shape(0) = 1.0;
}

void P0SegmentFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                        DenseMatrix &dshape) const
{
   dshape(0,0) = 0.0;
}

CrouzeixRaviartFiniteElement::CrouzeixRaviartFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE , 3, 1)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 0.5;
}

void CrouzeixRaviartFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   shape(0) =  1.0 - 2.0 * ip.y;
   shape(1) = -1.0 + 2.0 * ( ip.x + ip.y );
   shape(2) =  1.0 - 2.0 * ip.x;
}

void CrouzeixRaviartFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   dshape(0,0) =  0.0; dshape(0,1) = -2.0;
   dshape(1,0) =  2.0; dshape(1,1) =  2.0;
   dshape(2,0) = -2.0; dshape(2,1) =  0.0;
}

CrouzeixRaviartQuadFiniteElement::CrouzeixRaviartQuadFiniteElement()
// the FunctionSpace should be rotated (45 degrees) Q_1
// i.e. the span of { 1, x, y, x^2 - y^2 }
   : NodalFiniteElement(2, Geometry::SQUARE , 4, 2, FunctionSpace::Qk)

{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(2).x = 0.5;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.5;
}

void CrouzeixRaviartQuadFiniteElement::CalcShape(const IntegrationPoint &ip,
                                                 Vector &shape) const
{
   const double l1 = ip.x+ip.y-0.5, l2 = 1.-l1, l3 = ip.x-ip.y+0.5, l4 = 1.-l3;

   shape(0) = l2 * l3;
   shape(1) = l1 * l3;
   shape(2) = l1 * l4;
   shape(3) = l2 * l4;
}

void CrouzeixRaviartQuadFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                                  DenseMatrix &dshape) const
{
   const double x2 = 2.*ip.x, y2 = 2.*ip.y;

   dshape(0,0) =  1. - x2; dshape(0,1) = -2. + y2;
   dshape(1,0) =       x2; dshape(1,1) =  1. - y2;
   dshape(2,0) =  1. - x2; dshape(2,1) =       y2;
   dshape(3,0) = -2. + x2; dshape(3,1) =  1. - y2;
}


RT0TriangleFiniteElement::RT0TriangleFiniteElement()
   : VectorFiniteElement (2, Geometry::TRIANGLE, 3, 1)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 0.5;
}

void RT0TriangleFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                          DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   shape(0,0) = x;
   shape(0,1) = y - 1.;
   shape(1,0) = x;
   shape(1,1) = y;
   shape(2,0) = x - 1.;
   shape(2,1) = y;
}

void RT0TriangleFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                            Vector &divshape) const
{
   divshape(0) = 2.;
   divshape(1) = 2.;
   divshape(2) = 2.;
}

const double RT0TriangleFiniteElement::nk[3][2] =
{ {0, -1}, {1, 1}, {-1, 0} };

void RT0TriangleFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 3; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 3; j++)
      {
         double d = vshape(j,0)*nk[k][0]+vshape(j,1)*nk[k][1];
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT0TriangleFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[2];
   Vector xk (vk, 2);

   for (k = 0; k < 3; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1];
      for (j = 0; j < 3; j++)
         if (fabs (I(k,j) = vshape(j,0)*vk[0]+vshape(j,1)*vk[1]) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT0TriangleFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[2];
   Vector xk (vk, 2);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 3; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) = (vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1] ) +
                 vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1] ));
   }
}

RT0QuadFiniteElement::RT0QuadFiniteElement()
   : VectorFiniteElement (2, Geometry::SQUARE, 4, 1, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(2).x = 0.5;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.5;
}

void RT0QuadFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                      DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   shape(0,0) = 0;
   shape(0,1) = y - 1.;
   shape(1,0) = x;
   shape(1,1) = 0;
   shape(2,0) = 0;
   shape(2,1) = y;
   shape(3,0) = x - 1.;
   shape(3,1) = 0;
}

void RT0QuadFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                        Vector &divshape) const
{
   divshape(0) = 1.;
   divshape(1) = 1.;
   divshape(2) = 1.;
   divshape(3) = 1.;
}

const double RT0QuadFiniteElement::nk[4][2] =
{ {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

void RT0QuadFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 4; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 4; j++)
      {
         double d = vshape(j,0)*nk[k][0]+vshape(j,1)*nk[k][1];
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT0QuadFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear (more to have embedding?)
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[2];
   Vector xk (vk, 2);

   for (k = 0; k < 4; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1];
      for (j = 0; j < 4; j++)
         if (fabs (I(k,j) = vshape(j,0)*vk[0]+vshape(j,1)*vk[1]) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT0QuadFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[2];
   Vector xk (vk, 2);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 4; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) = (vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1] ) +
                 vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1] ));
   }
}

RT1TriangleFiniteElement::RT1TriangleFiniteElement()
   : VectorFiniteElement (2, Geometry::TRIANGLE, 8, 2)
{
   Nodes.IntPoint(0).x = 0.33333333333333333333;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 0.66666666666666666667;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.66666666666666666667;
   Nodes.IntPoint(2).y = 0.33333333333333333333;
   Nodes.IntPoint(3).x = 0.33333333333333333333;
   Nodes.IntPoint(3).y = 0.66666666666666666667;
   Nodes.IntPoint(4).x = 0.0;
   Nodes.IntPoint(4).y = 0.66666666666666666667;
   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.33333333333333333333;
   Nodes.IntPoint(6).x = 0.33333333333333333333;
   Nodes.IntPoint(6).y = 0.33333333333333333333;
   Nodes.IntPoint(7).x = 0.33333333333333333333;
   Nodes.IntPoint(7).y = 0.33333333333333333333;
}

void RT1TriangleFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                          DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   shape(0,0) = -2 * x * (-1 + x + 2 * y);
   shape(0,1) = -2 * (-1 + y) * (-1 + x + 2 * y);
   shape(1,0) =  2 * x * (x - y);
   shape(1,1) =  2 * (x - y) * (-1 + y);
   shape(2,0) =  2 * x * (-1 + 2 * x + y);
   shape(2,1) =  2 * y * (-1 + 2 * x + y);
   shape(3,0) =  2 * x * (-1 + x + 2 * y);
   shape(3,1) =  2 * y * (-1 + x + 2 * y);
   shape(4,0) = -2 * (-1 + x) * (x - y);
   shape(4,1) =  2 * y * (-x + y);
   shape(5,0) = -2 * (-1 + x) * (-1 + 2 * x + y);
   shape(5,1) = -2 * y * (-1 + 2 * x + y);
   shape(6,0) = -3 * x * (-2 + 2 * x + y);
   shape(6,1) = -3 * y * (-1 + 2 * x + y);
   shape(7,0) = -3 * x * (-1 + x + 2 * y);
   shape(7,1) = -3 * y * (-2 + x + 2 * y);
}

void RT1TriangleFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                            Vector &divshape) const
{
   double x = ip.x, y = ip.y;

   divshape(0) = -2 * (-4 + 3 * x + 6 * y);
   divshape(1) =  2 + 6 * x - 6 * y;
   divshape(2) = -4 + 12 * x + 6 * y;
   divshape(3) = -4 + 6 * x + 12 * y;
   divshape(4) =  2 - 6 * x + 6 * y;
   divshape(5) = -2 * (-4 + 6 * x + 3 * y);
   divshape(6) = -9 * (-1 + 2 * x + y);
   divshape(7) = -9 * (-1 + x + 2 * y);
}

const double RT1TriangleFiniteElement::nk[8][2] =
{
   { 0,-1}, { 0,-1},
   { 1, 1}, { 1, 1},
   {-1, 0}, {-1, 0},
   { 1, 0}, { 0, 1}
};

void RT1TriangleFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 8; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 8; j++)
      {
         double d = vshape(j,0)*nk[k][0]+vshape(j,1)*nk[k][1];
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT1QuadFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear (more to have embedding?)
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[2];
   Vector xk (vk, 2);

   for (k = 0; k < 8; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1];
      for (j = 0; j < 8; j++)
         if (fabs (I(k,j) = vshape(j,0)*vk[0]+vshape(j,1)*vk[1]) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT1TriangleFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   double vk[2];
   Vector xk (vk, 2);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 8; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) = (vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1] ) +
                 vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1] ));
      dofs(k) *= 0.5;
   }
}

RT1QuadFiniteElement::RT1QuadFiniteElement()
   : VectorFiniteElement (2, Geometry::SQUARE, 12, 2, FunctionSpace::Qk)
{
   // y = 0
   Nodes.IntPoint(0).x  = 1./3.;
   Nodes.IntPoint(0).y  = 0.0;
   Nodes.IntPoint(1).x  = 2./3.;
   Nodes.IntPoint(1).y  = 0.0;
   // x = 1
   Nodes.IntPoint(2).x  = 1.0;
   Nodes.IntPoint(2).y  = 1./3.;
   Nodes.IntPoint(3).x  = 1.0;
   Nodes.IntPoint(3).y  = 2./3.;
   // y = 1
   Nodes.IntPoint(4).x  = 2./3.;
   Nodes.IntPoint(4).y  = 1.0;
   Nodes.IntPoint(5).x  = 1./3.;
   Nodes.IntPoint(5).y  = 1.0;
   // x = 0
   Nodes.IntPoint(6).x  = 0.0;
   Nodes.IntPoint(6).y  = 2./3.;
   Nodes.IntPoint(7).x  = 0.0;
   Nodes.IntPoint(7).y  = 1./3.;
   // x = 0.5 (interior)
   Nodes.IntPoint(8).x  = 0.5;
   Nodes.IntPoint(8).y  = 1./3.;
   Nodes.IntPoint(9).x  = 0.5;
   Nodes.IntPoint(9).y  = 2./3.;
   // y = 0.5 (interior)
   Nodes.IntPoint(10).x = 1./3.;
   Nodes.IntPoint(10).y = 0.5;
   Nodes.IntPoint(11).x = 2./3.;
   Nodes.IntPoint(11).y = 0.5;
}

void RT1QuadFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                      DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   // y = 0
   shape(0,0)  = 0;
   shape(0,1)  = -( 1. - 3.*y + 2.*y*y)*( 2. - 3.*x);
   shape(1,0)  = 0;
   shape(1,1)  = -( 1. - 3.*y + 2.*y*y)*(-1. + 3.*x);
   // x = 1
   shape(2,0)  = (-x + 2.*x*x)*( 2. - 3.*y);
   shape(2,1)  = 0;
   shape(3,0)  = (-x + 2.*x*x)*(-1. + 3.*y);
   shape(3,1)  = 0;
   // y = 1
   shape(4,0)  = 0;
   shape(4,1)  = (-y + 2.*y*y)*(-1. + 3.*x);
   shape(5,0)  = 0;
   shape(5,1)  = (-y + 2.*y*y)*( 2. - 3.*x);
   // x = 0
   shape(6,0)  = -(1. - 3.*x + 2.*x*x)*(-1. + 3.*y);
   shape(6,1)  = 0;
   shape(7,0)  = -(1. - 3.*x + 2.*x*x)*( 2. - 3.*y);
   shape(7,1)  = 0;
   // x = 0.5 (interior)
   shape(8,0)  = (4.*x - 4.*x*x)*( 2. - 3.*y);
   shape(8,1)  = 0;
   shape(9,0)  = (4.*x - 4.*x*x)*(-1. + 3.*y);
   shape(9,1)  = 0;
   // y = 0.5 (interior)
   shape(10,0) = 0;
   shape(10,1) = (4.*y - 4.*y*y)*( 2. - 3.*x);
   shape(11,0) = 0;
   shape(11,1) = (4.*y - 4.*y*y)*(-1. + 3.*x);
}

void RT1QuadFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                        Vector &divshape) const
{
   double x = ip.x, y = ip.y;

   divshape(0)  = -(-3. + 4.*y)*( 2. - 3.*x);
   divshape(1)  = -(-3. + 4.*y)*(-1. + 3.*x);
   divshape(2)  = (-1. + 4.*x)*( 2. - 3.*y);
   divshape(3)  = (-1. + 4.*x)*(-1. + 3.*y);
   divshape(4)  = (-1. + 4.*y)*(-1. + 3.*x);
   divshape(5)  = (-1. + 4.*y)*( 2. - 3.*x);
   divshape(6)  = -(-3. + 4.*x)*(-1. + 3.*y);
   divshape(7)  = -(-3. + 4.*x)*( 2. - 3.*y);
   divshape(8)  = ( 4. - 8.*x)*( 2. - 3.*y);
   divshape(9)  = ( 4. - 8.*x)*(-1. + 3.*y);
   divshape(10) = ( 4. - 8.*y)*( 2. - 3.*x);
   divshape(11) = ( 4. - 8.*y)*(-1. + 3.*x);
}

const double RT1QuadFiniteElement::nk[12][2] =
{
   // y = 0
   {0,-1}, {0,-1},
   // X = 1
   {1, 0}, {1, 0},
   // y = 1
   {0, 1}, {0, 1},
   // x = 0
   {-1,0}, {-1,0},
   // x = 0.5 (interior)
   {1, 0}, {1, 0},
   // y = 0.5 (interior)
   {0, 1}, {0, 1}
};

void RT1QuadFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 12; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 12; j++)
      {
         double d = vshape(j,0)*nk[k][0]+vshape(j,1)*nk[k][1];
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT1QuadFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear (more to have embedding?)
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[2];
   Vector xk (vk, 2);

   for (k = 0; k < 12; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1];
      for (j = 0; j < 12; j++)
         if (fabs (I(k,j) = vshape(j,0)*vk[0]+vshape(j,1)*vk[1]) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT1QuadFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   double vk[2];
   Vector xk (vk, 2);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 12; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) = (vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1] ) +
                 vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1] ));
   }
}

const double RT2TriangleFiniteElement::M[15][15] =
{{ 0, -5.3237900077244501311, 5.3237900077244501311, 16.647580015448900262,
   0, 24.442740046346700787, -16.647580015448900262, -12.,
   -19.118950038622250656, -47.237900077244501311, 0, -34.414110069520051180,
   12., 30.590320061795601049, 15.295160030897800524},
 { 0, 1.5, -1.5, -15., 0, 2.625, 15., 15., -4.125, 30., 0, -14.625, -15.,
   -15., 10.5},
 { 0, -0.67620999227554986889, 0.67620999227554986889, 7.3524199845510997378,
   0, -3.4427400463467007866, -7.3524199845510997378, -12.,
   4.1189500386222506555, -0.76209992275549868892, 0, 7.4141100695200511800,
   12., -6.5903200617956010489, -3.2951600308978005244},
 { 0, 0, 1.5, 0, 0, 1.5, -11.471370023173350393, 0, 2.4713700231733503933,
   -11.471370023173350393, 0, 2.4713700231733503933, 15.295160030897800524,
   0, -3.2951600308978005244},
 { 0, 0, 4.875, 0, 0, 4.875, -16.875, 0, -16.875, -16.875, 0, -16.875, 10.5,
   36., 10.5},
 { 0, 0, 1.5, 0, 0, 1.5, 2.4713700231733503933, 0, -11.471370023173350393,
   2.4713700231733503933, 0, -11.471370023173350393, -3.2951600308978005244,
   0, 15.295160030897800524},
 { -0.67620999227554986889, 0, -3.4427400463467007866, 0,
   7.3524199845510997378, 0.67620999227554986889, 7.4141100695200511800, 0,
   -0.76209992275549868892, 4.1189500386222506555, -12.,
   -7.3524199845510997378, -3.2951600308978005244, -6.5903200617956010489,
   12.},
 { 1.5, 0, 2.625, 0, -15., -1.5, -14.625, 0, 30., -4.125, 15., 15., 10.5,
   -15., -15.},
 { -5.3237900077244501311, 0, 24.442740046346700787, 0, 16.647580015448900262,
   5.3237900077244501311, -34.414110069520051180, 0, -47.237900077244501311,
   -19.118950038622250656, -12., -16.647580015448900262, 15.295160030897800524,
   30.590320061795601049, 12.},
 { 0, 0, 18., 0, 0, 6., -42., 0, -30., -26., 0, -14., 24., 32., 8.},
 { 0, 0, 6., 0, 0, 18., -14., 0, -26., -30., 0, -42., 8., 32., 24.},
 { 0, 0, -6., 0, 0, -4., 30., 0, 4., 22., 0, 4., -24., -16., 0},
 { 0, 0, -4., 0, 0, -8., 20., 0, 8., 36., 0, 8., -16., -32., 0},
 { 0, 0, -8., 0, 0, -4., 8., 0, 36., 8., 0, 20., 0, -32., -16.},
 { 0, 0, -4., 0, 0, -6., 4., 0, 22., 4., 0, 30., 0, -16., -24.}
};

RT2TriangleFiniteElement::RT2TriangleFiniteElement()
   : VectorFiniteElement (2, Geometry::TRIANGLE, 15, 3)
{
   const double p = 0.11270166537925831148;

   Nodes.IntPoint(0).x = p;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 1.-p;
   Nodes.IntPoint(2).y = 0.0;
   Nodes.IntPoint(3).x = 1.-p;
   Nodes.IntPoint(3).y = p;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.5;
   Nodes.IntPoint(5).x = p;
   Nodes.IntPoint(5).y = 1.-p;
   Nodes.IntPoint(6).x = 0.0;
   Nodes.IntPoint(6).y = 1.-p;
   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(8).x = 0.0;
   Nodes.IntPoint(8).y = p;
   Nodes.IntPoint(9).x  = 0.25;
   Nodes.IntPoint(9).y  = 0.25;
   Nodes.IntPoint(10).x = 0.25;
   Nodes.IntPoint(10).y = 0.25;
   Nodes.IntPoint(11).x = 0.5;
   Nodes.IntPoint(11).y = 0.25;
   Nodes.IntPoint(12).x = 0.5;
   Nodes.IntPoint(12).y = 0.25;
   Nodes.IntPoint(13).x = 0.25;
   Nodes.IntPoint(13).y = 0.5;
   Nodes.IntPoint(14).x = 0.25;
   Nodes.IntPoint(14).y = 0.5;
}

void RT2TriangleFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                          DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   double Bx[15] = {1., 0., x, 0., y, 0., x*x, 0., x*y, 0., y*y, 0., x*x*x,
                    x*x*y, x*y*y};
   double By[15] = {0., 1., 0., x, 0., y, 0., x*x, 0., x*y, 0., y*y,
                    x*x*y, x*y*y, y*y*y};

   for (int i = 0; i < 15; i++)
   {
      double cx = 0.0, cy = 0.0;
      for (int j = 0; j < 15; j++)
      {
         cx += M[i][j] * Bx[j];
         cy += M[i][j] * By[j];
      }
      shape(i,0) = cx;
      shape(i,1) = cy;
   }
}

void RT2TriangleFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                            Vector &divshape) const
{
   double x = ip.x, y = ip.y;

   double DivB[15] = {0., 0., 1., 0., 0., 1., 2.*x, 0., y, x, 0., 2.*y,
                      4.*x*x, 4.*x*y, 4.*y*y};

   for (int i = 0; i < 15; i++)
   {
      double div = 0.0;
      for (int j = 0; j < 15; j++)
         div += M[i][j] * DivB[j];
      divshape(i) = div;
   }
}

const double RT2QuadFiniteElement::pt[4] = {0.,1./3.,2./3.,1.};

const double RT2QuadFiniteElement::dpt[3] = {0.25,0.5,0.75};

RT2QuadFiniteElement::RT2QuadFiniteElement()
   : VectorFiniteElement (2, Geometry::SQUARE, 24, 3, FunctionSpace::Qk)
{
   // y = 0 (pt[0])
   Nodes.IntPoint(0).x  = dpt[0];  Nodes.IntPoint(0).y  =  pt[0];
   Nodes.IntPoint(1).x  = dpt[1];  Nodes.IntPoint(1).y  =  pt[0];
   Nodes.IntPoint(2).x  = dpt[2];  Nodes.IntPoint(2).y  =  pt[0];
   // x = 1 (pt[3])
   Nodes.IntPoint(3).x  =  pt[3];  Nodes.IntPoint(3).y  = dpt[0];
   Nodes.IntPoint(4).x  =  pt[3];  Nodes.IntPoint(4).y  = dpt[1];
   Nodes.IntPoint(5).x  =  pt[3];  Nodes.IntPoint(5).y  = dpt[2];
   // y = 1 (pt[3])
   Nodes.IntPoint(6).x  = dpt[2];  Nodes.IntPoint(6).y  =  pt[3];
   Nodes.IntPoint(7).x  = dpt[1];  Nodes.IntPoint(7).y  =  pt[3];
   Nodes.IntPoint(8).x  = dpt[0];  Nodes.IntPoint(8).y  =  pt[3];
   // x = 0 (pt[0])
   Nodes.IntPoint(9).x  =  pt[0];  Nodes.IntPoint(9).y  = dpt[2];
   Nodes.IntPoint(10).x =  pt[0];  Nodes.IntPoint(10).y = dpt[1];
   Nodes.IntPoint(11).x =  pt[0];  Nodes.IntPoint(11).y = dpt[0];
   // x = pt[1] (interior)
   Nodes.IntPoint(12).x =  pt[1];  Nodes.IntPoint(12).y = dpt[0];
   Nodes.IntPoint(13).x =  pt[1];  Nodes.IntPoint(13).y = dpt[1];
   Nodes.IntPoint(14).x =  pt[1];  Nodes.IntPoint(14).y = dpt[2];
   // x = pt[2] (interior)
   Nodes.IntPoint(15).x =  pt[2];  Nodes.IntPoint(15).y = dpt[0];
   Nodes.IntPoint(16).x =  pt[2];  Nodes.IntPoint(16).y = dpt[1];
   Nodes.IntPoint(17).x =  pt[2];  Nodes.IntPoint(17).y = dpt[2];
   // y = pt[1] (interior)
   Nodes.IntPoint(18).x = dpt[0];  Nodes.IntPoint(18).y =  pt[1];
   Nodes.IntPoint(19).x = dpt[1];  Nodes.IntPoint(19).y =  pt[1];
   Nodes.IntPoint(20).x = dpt[2];  Nodes.IntPoint(20).y =  pt[1];
   // y = pt[2] (interior)
   Nodes.IntPoint(21).x = dpt[0];  Nodes.IntPoint(21).y =  pt[2];
   Nodes.IntPoint(22).x = dpt[1];  Nodes.IntPoint(22).y =  pt[2];
   Nodes.IntPoint(23).x = dpt[2];  Nodes.IntPoint(23).y =  pt[2];
}

void RT2QuadFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                      DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y;

   double ax0 =  pt[0] - x;
   double ax1 =  pt[1] - x;
   double ax2 =  pt[2] - x;
   double ax3 =  pt[3] - x;

   double by0 = dpt[0] - y;
   double by1 = dpt[1] - y;
   double by2 = dpt[2] - y;

   double ay0 =  pt[0] - y;
   double ay1 =  pt[1] - y;
   double ay2 =  pt[2] - y;
   double ay3 =  pt[3] - y;

   double bx0 = dpt[0] - x;
   double bx1 = dpt[1] - x;
   double bx2 = dpt[2] - x;

   double A01 =  pt[0] -  pt[1];
   double A02 =  pt[0] -  pt[2];
   double A12 =  pt[1] -  pt[2];
   double A03 =  pt[0] -  pt[3];
   double A13 =  pt[1] -  pt[3];
   double A23 =  pt[2] -  pt[3];

   double B01 = dpt[0] - dpt[1];
   double B02 = dpt[0] - dpt[2];
   double B12 = dpt[1] - dpt[2];

   double tx0 =  (bx1*bx2)/(B01*B02);
   double tx1 = -(bx0*bx2)/(B01*B12);
   double tx2 =  (bx0*bx1)/(B02*B12);

   double ty0 =  (by1*by2)/(B01*B02);
   double ty1 = -(by0*by2)/(B01*B12);
   double ty2 =  (by0*by1)/(B02*B12);

   // y = 0 (p[0])
   shape(0,  0) =  0;
   shape(0,  1) =  (ay1*ay2*ay3)/(A01*A02*A03)*tx0;
   shape(1,  0) =  0;
   shape(1,  1) =  (ay1*ay2*ay3)/(A01*A02*A03)*tx1;
   shape(2,  0) =  0;
   shape(2,  1) =  (ay1*ay2*ay3)/(A01*A02*A03)*tx2;
   // x = 1 (p[3])
   shape(3,  0) =  (ax0*ax1*ax2)/(A03*A13*A23)*ty0;
   shape(3,  1) =  0;
   shape(4,  0) =  (ax0*ax1*ax2)/(A03*A13*A23)*ty1;
   shape(4,  1) =  0;
   shape(5,  0) =  (ax0*ax1*ax2)/(A03*A13*A23)*ty2;
   shape(5,  1) =  0;
   // y = 1 (p[3])
   shape(6,  0) =  0;
   shape(6,  1) =  (ay0*ay1*ay2)/(A03*A13*A23)*tx2;
   shape(7,  0) =  0;
   shape(7,  1) =  (ay0*ay1*ay2)/(A03*A13*A23)*tx1;
   shape(8,  0) =  0;
   shape(8,  1) =  (ay0*ay1*ay2)/(A03*A13*A23)*tx0;
   // x = 0 (p[0])
   shape(9,  0) =  (ax1*ax2*ax3)/(A01*A02*A03)*ty2;
   shape(9,  1) =  0;
   shape(10, 0) =  (ax1*ax2*ax3)/(A01*A02*A03)*ty1;
   shape(10, 1) =  0;
   shape(11, 0) =  (ax1*ax2*ax3)/(A01*A02*A03)*ty0;
   shape(11, 1) =  0;
   // x = p[1] (interior)
   shape(12, 0) =  (ax0*ax2*ax3)/(A01*A12*A13)*ty0;
   shape(12, 1) =  0;
   shape(13, 0) =  (ax0*ax2*ax3)/(A01*A12*A13)*ty1;
   shape(13, 1) =  0;
   shape(14, 0) =  (ax0*ax2*ax3)/(A01*A12*A13)*ty2;
   shape(14, 1) =  0;
   // x = p[2] (interior)
   shape(15, 0) = -(ax0*ax1*ax3)/(A02*A12*A23)*ty0;
   shape(15, 1) =  0;
   shape(16, 0) = -(ax0*ax1*ax3)/(A02*A12*A23)*ty1;
   shape(16, 1) =  0;
   shape(17, 0) = -(ax0*ax1*ax3)/(A02*A12*A23)*ty2;
   shape(17, 1) =  0;
   // y = p[1] (interior)
   shape(18, 0) =  0;
   shape(18, 1) =  (ay0*ay2*ay3)/(A01*A12*A13)*tx0;
   shape(19, 0) =  0;
   shape(19, 1) =  (ay0*ay2*ay3)/(A01*A12*A13)*tx1;
   shape(20, 0) =  0;
   shape(20, 1) =  (ay0*ay2*ay3)/(A01*A12*A13)*tx2;
   // y = p[2] (interior)
   shape(21, 0) =  0;
   shape(21, 1) = -(ay0*ay1*ay3)/(A02*A12*A23)*tx0;
   shape(22, 0) =  0;
   shape(22, 1) = -(ay0*ay1*ay3)/(A02*A12*A23)*tx1;
   shape(23, 0) =  0;
   shape(23, 1) = -(ay0*ay1*ay3)/(A02*A12*A23)*tx2;
}

void RT2QuadFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                        Vector &divshape) const
{
   double x = ip.x, y = ip.y;

   double a01 =  pt[0]*pt[1];
   double a02 =  pt[0]*pt[2];
   double a12 =  pt[1]*pt[2];
   double a03 =  pt[0]*pt[3];
   double a13 =  pt[1]*pt[3];
   double a23 =  pt[2]*pt[3];

   double bx0 = dpt[0] - x;
   double bx1 = dpt[1] - x;
   double bx2 = dpt[2] - x;

   double by0 = dpt[0] - y;
   double by1 = dpt[1] - y;
   double by2 = dpt[2] - y;

   double A01 =  pt[0] -  pt[1];
   double A02 =  pt[0] -  pt[2];
   double A12 =  pt[1] -  pt[2];
   double A03 =  pt[0] -  pt[3];
   double A13 =  pt[1] -  pt[3];
   double A23 =  pt[2] -  pt[3];

   double A012 = pt[0] + pt[1] + pt[2];
   double A013 = pt[0] + pt[1] + pt[3];
   double A023 = pt[0] + pt[2] + pt[3];
   double A123 = pt[1] + pt[2] + pt[3];

   double B01 = dpt[0] - dpt[1];
   double B02 = dpt[0] - dpt[2];
   double B12 = dpt[1] - dpt[2];

   double tx0 =  (bx1*bx2)/(B01*B02);
   double tx1 = -(bx0*bx2)/(B01*B12);
   double tx2 =  (bx0*bx1)/(B02*B12);

   double ty0 =  (by1*by2)/(B01*B02);
   double ty1 = -(by0*by2)/(B01*B12);
   double ty2 =  (by0*by1)/(B02*B12);

   // y = 0 (p[0])
   divshape(0)  = -(a12 + a13 + a23 - 2.*A123*y + 3.*y*y)/(A01*A02*A03)*tx0;
   divshape(1)  = -(a12 + a13 + a23 - 2.*A123*y + 3.*y*y)/(A01*A02*A03)*tx1;
   divshape(2)  = -(a12 + a13 + a23 - 2.*A123*y + 3.*y*y)/(A01*A02*A03)*tx2;
   // x = 1 (p[3])
   divshape(3)  = -(a01 + a02 + a12 - 2.*A012*x + 3.*x*x)/(A03*A13*A23)*ty0;
   divshape(4)  = -(a01 + a02 + a12 - 2.*A012*x + 3.*x*x)/(A03*A13*A23)*ty1;
   divshape(5)  = -(a01 + a02 + a12 - 2.*A012*x + 3.*x*x)/(A03*A13*A23)*ty2;
   // y = 1 (p[3])
   divshape(6)  = -(a01 + a02 + a12 - 2.*A012*y + 3.*y*y)/(A03*A13*A23)*tx2;
   divshape(7)  = -(a01 + a02 + a12 - 2.*A012*y + 3.*y*y)/(A03*A13*A23)*tx1;
   divshape(8)  = -(a01 + a02 + a12 - 2.*A012*y + 3.*y*y)/(A03*A13*A23)*tx0;
   // x = 0 (p[0])
   divshape(9)  = -(a12 + a13 + a23 - 2.*A123*x + 3.*x*x)/(A01*A02*A03)*ty2;
   divshape(10) = -(a12 + a13 + a23 - 2.*A123*x + 3.*x*x)/(A01*A02*A03)*ty1;
   divshape(11) = -(a12 + a13 + a23 - 2.*A123*x + 3.*x*x)/(A01*A02*A03)*ty0;
   // x = p[1] (interior)
   divshape(12) = -(a02 + a03 + a23 - 2.*A023*x + 3.*x*x)/(A01*A12*A13)*ty0;
   divshape(13) = -(a02 + a03 + a23 - 2.*A023*x + 3.*x*x)/(A01*A12*A13)*ty1;
   divshape(14) = -(a02 + a03 + a23 - 2.*A023*x + 3.*x*x)/(A01*A12*A13)*ty2;
   // x = p[2] (interior)
   divshape(15) =  (a01 + a03 + a13 - 2.*A013*x + 3.*x*x)/(A02*A12*A23)*ty0;
   divshape(16) =  (a01 + a03 + a13 - 2.*A013*x + 3.*x*x)/(A02*A12*A23)*ty1;
   divshape(17) =  (a01 + a03 + a13 - 2.*A013*x + 3.*x*x)/(A02*A12*A23)*ty2;
   // y = p[1] (interior)
   divshape(18) = -(a02 + a03 + a23 - 2.*A023*y + 3.*y*y)/(A01*A12*A13)*tx0;
   divshape(19) = -(a02 + a03 + a23 - 2.*A023*y + 3.*y*y)/(A01*A12*A13)*tx1;
   divshape(20) = -(a02 + a03 + a23 - 2.*A023*y + 3.*y*y)/(A01*A12*A13)*tx2;
   // y = p[2] (interior)
   divshape(21) =  (a01 + a03 + a13 - 2.*A013*y + 3.*y*y)/(A02*A12*A23)*tx0;
   divshape(22) =  (a01 + a03 + a13 - 2.*A013*y + 3.*y*y)/(A02*A12*A23)*tx1;
   divshape(23) =  (a01 + a03 + a13 - 2.*A013*y + 3.*y*y)/(A02*A12*A23)*tx2;
}

const double RT2QuadFiniteElement::nk[24][2] =
{
   // y = 0
   {0,-1}, {0,-1}, {0,-1},
   // x = 1
   {1, 0}, {1, 0}, {1, 0},
   // y = 1
   {0, 1}, {0, 1}, {0, 1},
   // x = 0
   {-1,0}, {-1,0}, {-1,0},
   // x = p[1] (interior)
   {1, 0}, {1, 0}, {1, 0},
   // x = p[2] (interior)
   {1, 0}, {1, 0}, {1, 0},
   // y = p[1] (interior)
   {0, 1}, {0, 1}, {0, 1},
   // y = p[1] (interior)
   {0, 1}, {0, 1}, {0, 1}
};

void RT2QuadFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 24; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 24; j++)
      {
         double d = vshape(j,0)*nk[k][0]+vshape(j,1)*nk[k][1];
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT2QuadFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear (more to have embedding?)
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[2];
   Vector xk (vk, 2);

   for (k = 0; k < 24; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1];
      for (j = 0; j < 24; j++)
         if (fabs (I(k,j) = vshape(j,0)*vk[0]+vshape(j,1)*vk[1]) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT2QuadFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans, Vector &dofs) const
{
   double vk[2];
   Vector xk (vk, 2);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 24; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) = (vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1] ) +
                 vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1] ));
   }
}

P1SegmentFiniteElement::P1SegmentFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 2, 1)
{
   Nodes.IntPoint(0).x = 0.33333333333333333333;
   Nodes.IntPoint(1).x = 0.66666666666666666667;
}

void P1SegmentFiniteElement::CalcShape(const IntegrationPoint &ip,
                                       Vector &shape) const
{
   double x = ip.x;

   shape(0) = 2. - 3. * x;
   shape(1) = 3. * x - 1.;
}

void P1SegmentFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                        DenseMatrix &dshape) const
{
   dshape(0,0) = -3.;
   dshape(1,0) =  3.;
}


P2SegmentFiniteElement::P2SegmentFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 3, 2)
{
   const double p = 0.11270166537925831148;

   Nodes.IntPoint(0).x = p;
   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(2).x = 1.-p;
}

void P2SegmentFiniteElement::CalcShape(const IntegrationPoint &ip,
                                       Vector &shape) const
{
   const double p = 0.11270166537925831148;
   const double w = 1./((1-2*p)*(1-2*p));
   double x = ip.x;

   shape(0) = (2*x-1)*(x-1+p)*w;
   shape(1) = 4*(x-1+p)*(p-x)*w;
   shape(2) = (2*x-1)*(x-p)*w;
}

void P2SegmentFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                        DenseMatrix &dshape) const
{
   const double p = 0.11270166537925831148;
   const double w = 1./((1-2*p)*(1-2*p));
   double x = ip.x;

   dshape(0,0) = (-3+4*x+2*p)*w;
   dshape(1,0) = (4-8*x)*w;
   dshape(2,0) = (-1+4*x-2*p)*w;
}


Lagrange1DFiniteElement::Lagrange1DFiniteElement(int degree)
   : NodalFiniteElement(1, Geometry::SEGMENT, degree+1, degree)
{
   int i, m = degree;

   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   for (i = 1; i < m; i++)
      Nodes.IntPoint(i+1).x = double(i) / m;

   rwk.SetSize(degree+1);
#ifndef MFEM_USE_OPENMP
   rxxk.SetSize(degree+1);
#endif

   rwk(0) = 1.0;
   for (i = 1; i <= m; i++)
      rwk(i) = rwk(i-1) * ( (double)(m) / (double)(i) );
   for (i = 0; i < m/2+1; i++)
      rwk(m-i) = ( rwk(i) *= rwk(m-i) );
   for (i = m-1; i >= 0; i -= 2)
      rwk(i) = -rwk(i);
}

void Lagrange1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                        Vector &shape) const
{
   double w, wk, x = ip.x;
   int i, k, m = GetOrder();

#ifdef MFEM_USE_OPENMP
   Vector rxxk(m+1);
#endif

   k = (int) floor ( m * x + 0.5 );
   wk = 1.0;
   for (i = 0; i <= m; i++)
      if (i != k)
         wk *= ( rxxk(i) = x - (double)(i) / m );
   w = wk * ( rxxk(k) = x - (double)(k) / m );

   if (k != 0)
      shape(0) = w * rwk(0) / rxxk(0);
   else
      shape(0) = wk * rwk(0);
   if (k != m)
      shape(1) = w * rwk(m) / rxxk(m);
   else
      shape(1) = wk * rwk(k);
   for (i = 1; i < m; i++)
      if (i != k)
         shape(i+1) = w * rwk(i) / rxxk(i);
      else
         shape(k+1) = wk * rwk(k);
}

void Lagrange1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                         DenseMatrix &dshape) const
{
   double s, srx, w, wk, x = ip.x;
   int i, k, m = GetOrder();

#ifdef MFEM_USE_OPENMP
   Vector rxxk(m+1);
#endif

   k = (int) floor ( m * x + 0.5 );
   wk = 1.0;
   for (i = 0; i <= m; i++)
      if (i != k)
         wk *= ( rxxk(i) = x - (double)(i) / m );
   w = wk * ( rxxk(k) = x - (double)(k) / m );

   for (i = 0; i <= m; i++)
      rxxk(i) = 1.0 / rxxk(i);
   srx = 0.0;
   for (i = 0; i <= m; i++)
      if (i != k)
         srx += rxxk(i);
   s = w * srx + wk;

   if (k != 0)
      dshape(0,0) = (s - w * rxxk(0)) * rwk(0) * rxxk(0);
   else
      dshape(0,0) = wk * srx * rwk(0);
   if (k != m)
      dshape(1,0) = (s - w * rxxk(m)) * rwk(m) * rxxk(m);
   else
      dshape(1,0) = wk * srx * rwk(k);
   for (i = 1; i < m; i++)
      if (i != k)
         dshape(i+1,0) = (s - w * rxxk(i)) * rwk(i) * rxxk(i);
      else
         dshape(k+1,0) = wk * srx * rwk(k);
}


P1TetNonConfFiniteElement::P1TetNonConfFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, 4, 1)
{
   Nodes.IntPoint(0).x = 0.33333333333333333333;
   Nodes.IntPoint(0).y = 0.33333333333333333333;
   Nodes.IntPoint(0).z = 0.33333333333333333333;

   Nodes.IntPoint(1).x = 0.0;
   Nodes.IntPoint(1).y = 0.33333333333333333333;
   Nodes.IntPoint(1).z = 0.33333333333333333333;

   Nodes.IntPoint(2).x = 0.33333333333333333333;
   Nodes.IntPoint(2).y = 0.0;
   Nodes.IntPoint(2).z = 0.33333333333333333333;

   Nodes.IntPoint(3).x = 0.33333333333333333333;
   Nodes.IntPoint(3).y = 0.33333333333333333333;
   Nodes.IntPoint(3).z = 0.0;

}

void P1TetNonConfFiniteElement::CalcShape(const IntegrationPoint &ip,
                                          Vector &shape) const
{
   double L0, L1, L2, L3;

   L1 = ip.x;  L2 = ip.y;  L3 = ip.z;  L0 = 1.0 - L1 - L2 - L3;
   shape(0) = 1.0 - 3.0 * L0;
   shape(1) = 1.0 - 3.0 * L1;
   shape(2) = 1.0 - 3.0 * L2;
   shape(3) = 1.0 - 3.0 * L3;
}

void P1TetNonConfFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                           DenseMatrix &dshape) const
{
   dshape(0,0) =  3.0; dshape(0,1) =  3.0; dshape(0,2) =  3.0;
   dshape(1,0) = -3.0; dshape(1,1) =  0.0; dshape(1,2) =  0.0;
   dshape(2,0) =  0.0; dshape(2,1) = -3.0; dshape(2,2) =  0.0;
   dshape(3,0) =  0.0; dshape(3,1) =  0.0; dshape(3,2) = -3.0;
}


P0TetFiniteElement::P0TetFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON , 1, 0)
{
   Nodes.IntPoint(0).x = 0.25;
   Nodes.IntPoint(0).y = 0.25;
   Nodes.IntPoint(0).z = 0.25;
}

void P0TetFiniteElement::CalcShape(const IntegrationPoint &ip,
                                   Vector &shape) const
{
   shape(0) = 1.0;
}

void P0TetFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                    DenseMatrix &dshape) const
{
   dshape(0,0) =  0.0; dshape(0,1) =  0.0; dshape(0,2) = 0.0;
}


P0HexFiniteElement::P0HexFiniteElement()
   : NodalFiniteElement(3, Geometry::CUBE, 1, 0, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.5;
   Nodes.IntPoint(0).z = 0.5;
}

void P0HexFiniteElement::CalcShape(const IntegrationPoint &ip,
                                   Vector &shape) const
{
   shape(0) = 1.0;
}

void P0HexFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                    DenseMatrix &dshape) const
{
   dshape(0,0) =  0.0; dshape(0,1) =  0.0; dshape(0,2) = 0.0;
}


LagrangeHexFiniteElement::LagrangeHexFiniteElement (int degree)
   : NodalFiniteElement(3, Geometry::CUBE, (degree+1)*(degree+1)*(degree+1),
                        degree, FunctionSpace::Qk)
{
   if (degree == 2)
   {
      I = new int[Dof];
      J = new int[Dof];
      K = new int[Dof];
      // nodes
      I[ 0] = 0; J[ 0] = 0; K[ 0] = 0;
      I[ 1] = 1; J[ 1] = 0; K[ 1] = 0;
      I[ 2] = 1; J[ 2] = 1; K[ 2] = 0;
      I[ 3] = 0; J[ 3] = 1; K[ 3] = 0;
      I[ 4] = 0; J[ 4] = 0; K[ 4] = 1;
      I[ 5] = 1; J[ 5] = 0; K[ 5] = 1;
      I[ 6] = 1; J[ 6] = 1; K[ 6] = 1;
      I[ 7] = 0; J[ 7] = 1; K[ 7] = 1;
      // edges
      I[ 8] = 2; J[ 8] = 0; K[ 8] = 0;
      I[ 9] = 1; J[ 9] = 2; K[ 9] = 0;
      I[10] = 2; J[10] = 1; K[10] = 0;
      I[11] = 0; J[11] = 2; K[11] = 0;
      I[12] = 2; J[12] = 0; K[12] = 1;
      I[13] = 1; J[13] = 2; K[13] = 1;
      I[14] = 2; J[14] = 1; K[14] = 1;
      I[15] = 0; J[15] = 2; K[15] = 1;
      I[16] = 0; J[16] = 0; K[16] = 2;
      I[17] = 1; J[17] = 0; K[17] = 2;
      I[18] = 1; J[18] = 1; K[18] = 2;
      I[19] = 0; J[19] = 1; K[19] = 2;
      // faces
      I[20] = 2; J[20] = 2; K[20] = 0;
      I[21] = 2; J[21] = 0; K[21] = 2;
      I[22] = 1; J[22] = 2; K[22] = 2;
      I[23] = 2; J[23] = 1; K[23] = 2;
      I[24] = 0; J[24] = 2; K[24] = 2;
      I[25] = 2; J[25] = 2; K[25] = 1;
      // element
      I[26] = 2; J[26] = 2; K[26] = 2;
   }
   else if (degree == 3)
   {
      I = new int[Dof];
      J = new int[Dof];
      K = new int[Dof];
      // nodes
      I[ 0] = 0; J[ 0] = 0; K[ 0] = 0;
      I[ 1] = 1; J[ 1] = 0; K[ 1] = 0;
      I[ 2] = 1; J[ 2] = 1; K[ 2] = 0;
      I[ 3] = 0; J[ 3] = 1; K[ 3] = 0;
      I[ 4] = 0; J[ 4] = 0; K[ 4] = 1;
      I[ 5] = 1; J[ 5] = 0; K[ 5] = 1;
      I[ 6] = 1; J[ 6] = 1; K[ 6] = 1;
      I[ 7] = 0; J[ 7] = 1; K[ 7] = 1;
      // edges
      I[ 8] = 2; J[ 8] = 0; K[ 8] = 0;
      I[ 9] = 3; J[ 9] = 0; K[ 9] = 0;
      I[10] = 1; J[10] = 2; K[10] = 0;
      I[11] = 1; J[11] = 3; K[11] = 0;
      I[12] = 2; J[12] = 1; K[12] = 0;
      I[13] = 3; J[13] = 1; K[13] = 0;
      I[14] = 0; J[14] = 2; K[14] = 0;
      I[15] = 0; J[15] = 3; K[15] = 0;
      I[16] = 2; J[16] = 0; K[16] = 1;
      I[17] = 3; J[17] = 0; K[17] = 1;
      I[18] = 1; J[18] = 2; K[18] = 1;
      I[19] = 1; J[19] = 3; K[19] = 1;
      I[20] = 2; J[20] = 1; K[20] = 1;
      I[21] = 3; J[21] = 1; K[21] = 1;
      I[22] = 0; J[22] = 2; K[22] = 1;
      I[23] = 0; J[23] = 3; K[23] = 1;
      I[24] = 0; J[24] = 0; K[24] = 2;
      I[25] = 0; J[25] = 0; K[25] = 3;
      I[26] = 1; J[26] = 0; K[26] = 2;
      I[27] = 1; J[27] = 0; K[27] = 3;
      I[28] = 1; J[28] = 1; K[28] = 2;
      I[29] = 1; J[29] = 1; K[29] = 3;
      I[30] = 0; J[30] = 1; K[30] = 2;
      I[31] = 0; J[31] = 1; K[31] = 3;
      // faces
      I[32] = 2; J[32] = 3; K[32] = 0;
      I[33] = 3; J[33] = 3; K[33] = 0;
      I[34] = 2; J[34] = 2; K[34] = 0;
      I[35] = 3; J[35] = 2; K[35] = 0;
      I[36] = 2; J[36] = 0; K[36] = 2;
      I[37] = 3; J[37] = 0; K[37] = 2;
      I[38] = 2; J[38] = 0; K[38] = 3;
      I[39] = 3; J[39] = 0; K[39] = 3;
      I[40] = 1; J[40] = 2; K[40] = 2;
      I[41] = 1; J[41] = 3; K[41] = 2;
      I[42] = 1; J[42] = 2; K[42] = 3;
      I[43] = 1; J[43] = 3; K[43] = 3;
      I[44] = 3; J[44] = 1; K[44] = 2;
      I[45] = 2; J[45] = 1; K[45] = 2;
      I[46] = 3; J[46] = 1; K[46] = 3;
      I[47] = 2; J[47] = 1; K[47] = 3;
      I[48] = 0; J[48] = 3; K[48] = 2;
      I[49] = 0; J[49] = 2; K[49] = 2;
      I[50] = 0; J[50] = 3; K[50] = 3;
      I[51] = 0; J[51] = 2; K[51] = 3;
      I[52] = 2; J[52] = 2; K[52] = 1;
      I[53] = 3; J[53] = 2; K[53] = 1;
      I[54] = 2; J[54] = 3; K[54] = 1;
      I[55] = 3; J[55] = 3; K[55] = 1;
      // element
      I[56] = 2; J[56] = 2; K[56] = 2;
      I[57] = 3; J[57] = 2; K[57] = 2;
      I[58] = 3; J[58] = 3; K[58] = 2;
      I[59] = 2; J[59] = 3; K[59] = 2;
      I[60] = 2; J[60] = 2; K[60] = 3;
      I[61] = 3; J[61] = 2; K[61] = 3;
      I[62] = 3; J[62] = 3; K[62] = 3;
      I[63] = 2; J[63] = 3; K[63] = 3;
   }
   else
   {
      mfem_error ("LagrangeHexFiniteElement::LagrangeHexFiniteElement");
   }

   fe1d = new Lagrange1DFiniteElement(degree);
   dof1d = fe1d -> GetDof();

#ifndef MFEM_USE_OPENMP
   shape1dx.SetSize(dof1d);
   shape1dy.SetSize(dof1d);
   shape1dz.SetSize(dof1d);

   dshape1dx.SetSize(dof1d,1);
   dshape1dy.SetSize(dof1d,1);
   dshape1dz.SetSize(dof1d,1);
#endif

   for (int n = 0; n < Dof; n++)
   {
      Nodes.IntPoint(n).x = fe1d -> GetNodes().IntPoint(I[n]).x;
      Nodes.IntPoint(n).y = fe1d -> GetNodes().IntPoint(J[n]).x;
      Nodes.IntPoint(n).z = fe1d -> GetNodes().IntPoint(K[n]).x;
   }
}

void LagrangeHexFiniteElement::CalcShape(const IntegrationPoint &ip,
                                         Vector &shape) const
{
   IntegrationPoint ipy, ipz;
   ipy.x = ip.y;
   ipz.x = ip.z;

#ifdef MFEM_USE_OPENMP
   Vector shape1dx(dof1d), shape1dy(dof1d), shape1dz(dof1d);
#endif

   fe1d -> CalcShape(ip,  shape1dx);
   fe1d -> CalcShape(ipy, shape1dy);
   fe1d -> CalcShape(ipz, shape1dz);

   for (int n = 0; n < Dof; n++)
      shape(n) = shape1dx(I[n]) *  shape1dy(J[n]) * shape1dz(K[n]);
}

void LagrangeHexFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                          DenseMatrix &dshape) const
{
   IntegrationPoint ipy, ipz;
   ipy.x = ip.y;
   ipz.x = ip.z;

#ifdef MFEM_USE_OPENMP
   Vector shape1dx(dof1d), shape1dy(dof1d), shape1dz(dof1d);
   DenseMatrix dshape1dx(dof1d,1), dshape1dy(dof1d,1), dshape1dz(dof1d,1);
#endif

   fe1d -> CalcShape(ip,  shape1dx);
   fe1d -> CalcShape(ipy, shape1dy);
   fe1d -> CalcShape(ipz, shape1dz);

   fe1d -> CalcDShape(ip,  dshape1dx);
   fe1d -> CalcDShape(ipy, dshape1dy);
   fe1d -> CalcDShape(ipz, dshape1dz);

   for (int n = 0; n < Dof; n++) {
      dshape(n,0) = dshape1dx(I[n],0) * shape1dy(J[n])    * shape1dz(K[n]);
      dshape(n,1) = shape1dx(I[n])    * dshape1dy(J[n],0) * shape1dz(K[n]);
      dshape(n,2) = shape1dx(I[n])    * shape1dy(J[n])    * dshape1dz(K[n],0);
   }
}

LagrangeHexFiniteElement::~LagrangeHexFiniteElement ()
{
   delete fe1d;

   delete [] I;
   delete [] J;
   delete [] K;
}


RefinedLinear1DFiniteElement::RefinedLinear1DFiniteElement()
   : NodalFiniteElement(1, Geometry::SEGMENT, 3, 4)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(2).x = 0.5;
}

void RefinedLinear1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   double x = ip.x;

   if (x <= 0.5) {
      shape(0) = 1.0 - 2.0 * x;
      shape(1) = 0.0;
      shape(2) = 2.0 * x;
   } else {
      shape(0) = 0.0;
      shape(1) = 2.0 * x - 1.0;
      shape(2) = 2.0 - 2.0 * x;
   }
}

void RefinedLinear1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   double x = ip.x;

   if (x <= 0.5) {
      dshape(0,0) = - 2.0;
      dshape(1,0) =   0.0;
      dshape(2,0) =   2.0;
   } else {
      dshape(0,0) =   0.0;
      dshape(1,0) =   2.0;
      dshape(2,0) = - 2.0;
   }
}

RefinedLinear2DFiniteElement::RefinedLinear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::TRIANGLE, 6, 5)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.5;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.5;
   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.5;
}

void RefinedLinear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   int i;

   double L0, L1, L2;
   L0 = 2.0 * ( 1. - ip.x - ip.y );
   L1 = 2.0 * ( ip.x );
   L2 = 2.0 * ( ip.y );

   // The reference triangle is split in 4 triangles as follows:
   //
   // T0 - 0,3,5
   // T1 - 1,3,4
   // T2 - 2,4,5
   // T3 - 3,4,5

   for (i = 0; i < 6; i++)
      shape(i) = 0.0;

   if (L0 >= 1.0) { // T0
      shape(0) = L0 - 1.0;
      shape(3) =       L1;
      shape(5) =       L2;
   }
   else if (L1 >= 1.0) { // T1
      shape(3) =       L0;
      shape(1) = L1 - 1.0;
      shape(4) =       L2;
   }
   else if (L2 >= 1.0) { // T2
      shape(5) =       L0;
      shape(4) =       L1;
      shape(2) = L2 - 1.0;
   }
   else { // T3
      shape(3) = 1.0 - L2;
      shape(4) = 1.0 - L0;
      shape(5) = 1.0 - L1;
   }
}

void RefinedLinear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   int i,j;

   double L0, L1, L2;
   L0 = 2.0 * ( 1. - ip.x - ip.y );
   L1 = 2.0 * ( ip.x );
   L2 = 2.0 * ( ip.y );

   double DL0[2], DL1[2], DL2[2];
   DL0[0] = -2.0; DL0[1] = -2.0;
   DL1[0] =  2.0; DL1[1] =  0.0;
   DL2[0] =  0.0; DL2[1] =  2.0;

   for (i = 0; i < 6; i++)
      for (j = 0; j < 2; j++)
         dshape(i,j) = 0.0;

   if (L0 >= 1.0) { // T0
      for (j = 0; j < 2; j++) {
         dshape(0,j) = DL0[j];
         dshape(3,j) = DL1[j];
         dshape(5,j) = DL2[j];
      }
   }
   else if (L1 >= 1.0) { // T1
      for (j = 0; j < 2; j++) {
         dshape(3,j) = DL0[j];
         dshape(1,j) = DL1[j];
         dshape(4,j) = DL2[j];
      }
   }
   else if (L2 >= 1.0) { // T2
      for (j = 0; j < 2; j++) {
         dshape(5,j) = DL0[j];
         dshape(4,j) = DL1[j];
         dshape(2,j) = DL2[j];
      }
   }
   else { // T3
      for (j = 0; j < 2; j++) {
         dshape(3,j) = - DL2[j];
         dshape(4,j) = - DL0[j];
         dshape(5,j) = - DL1[j];
      }
   }
}

RefinedLinear3DFiniteElement::RefinedLinear3DFiniteElement()
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, 10, 4)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.0;
   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(2).z = 0.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.0;
   Nodes.IntPoint(3).z = 1.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(4).z = 0.0;
   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 0.0;
   Nodes.IntPoint(6).x = 0.0;
   Nodes.IntPoint(6).y = 0.0;
   Nodes.IntPoint(6).z = 0.5;
   Nodes.IntPoint(7).x = 0.5;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(7).z = 0.0;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.0;
   Nodes.IntPoint(8).z = 0.5;
   Nodes.IntPoint(9).x = 0.0;
   Nodes.IntPoint(9).y = 0.5;
   Nodes.IntPoint(9).z = 0.5;
}

void RefinedLinear3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   int i;

   double L0, L1, L2, L3, L4, L5;
   L0 = 2.0 * ( 1. - ip.x - ip.y - ip.z );
   L1 = 2.0 * ( ip.x );
   L2 = 2.0 * ( ip.y );
   L3 = 2.0 * ( ip.z );
   L4 = 2.0 * ( ip.x + ip.y );
   L5 = 2.0 * ( ip.y + ip.z );

   // The reference tetrahedron is split in 8 tetrahedra as follows:
   //
   // T0 - 0,4,5,6
   // T1 - 1,4,7,8
   // T2 - 2,5,7,9
   // T3 - 3,6,8,9
   // T4 - 4,5,6,8
   // T5 - 4,5,7,8
   // T6 - 5,6,8,9
   // T7 - 5,7,8,9

   for (i = 0; i < 10; i++)
      shape(i) = 0.0;

   if (L0 >= 1.0) { // T0
      shape(0) = L0 - 1.0;
      shape(4) =       L1;
      shape(5) =       L2;
      shape(6) =       L3;
   }
   else if (L1 >= 1.0) { // T1
      shape(4) =       L0;
      shape(1) = L1 - 1.0;
      shape(7) =       L2;
      shape(8) =       L3;
   }
   else if (L2 >= 1.0) { // T2
      shape(5) =       L0;
      shape(7) =       L1;
      shape(2) = L2 - 1.0;
      shape(9) =       L3;
   }
   else if (L3 >= 1.0) { // T3
      shape(6) =       L0;
      shape(8) =       L1;
      shape(9) =       L2;
      shape(3) = L3 - 1.0;
   }
   else if ((L4 <= 1.0) && (L5 <= 1.0)) { // T4
      shape(4) = 1.0 - L5;
      shape(5) =       L2;
      shape(6) = 1.0 - L4;
      shape(8) = 1.0 - L0;
   }
   else if ((L4 >= 1.0) && (L5 <= 1.0)) { // T5
      shape(4) = 1.0 - L5;
      shape(5) = 1.0 - L1;
      shape(7) = L4 - 1.0;
      shape(8) =       L3;
   }
   else if ((L4 <= 1.0) && (L5 >= 1.0)) { // T6
      shape(5) = 1.0 - L3;
      shape(6) = 1.0 - L4;
      shape(8) =       L1;
      shape(9) = L5 - 1.0;
   }
   else if ((L4 >= 1.0) && (L5 >= 1.0)) { // T7
      shape(5) =       L0;
      shape(7) = L4 - 1.0;
      shape(8) = 1.0 - L2;
      shape(9) = L5 - 1.0;
   }
}

void RefinedLinear3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   int i,j;

   double L0, L1, L2, L3, L4, L5;
   L0 = 2.0 * ( 1. - ip.x - ip.y - ip.z );
   L1 = 2.0 * ( ip.x );
   L2 = 2.0 * ( ip.y );
   L3 = 2.0 * ( ip.z );
   L4 = 2.0 * ( ip.x + ip.y );
   L5 = 2.0 * ( ip.y + ip.z );

   double DL0[3], DL1[3], DL2[3], DL3[3], DL4[3], DL5[3];
   DL0[0] = -2.0; DL0[1] = -2.0; DL0[2] = -2.0;
   DL1[0] =  2.0; DL1[1] =  0.0; DL1[2] =  0.0;
   DL2[0] =  0.0; DL2[1] =  2.0; DL2[2] =  0.0;
   DL3[0] =  0.0; DL3[1] =  0.0; DL3[2] =  2.0;
   DL4[0] =  2.0; DL4[1] =  2.0; DL4[2] =  0.0;
   DL5[0] =  0.0; DL5[1] =  2.0; DL5[2] =  2.0;

   for (i = 0; i < 10; i++)
      for (j = 0; j < 3; j++)
         dshape(i,j) = 0.0;

   if (L0 >= 1.0) { // T0
      for (j = 0; j < 3; j++) {
         dshape(0,j) = DL0[j];
         dshape(4,j) = DL1[j];
         dshape(5,j) = DL2[j];
         dshape(6,j) = DL3[j];
      }
   }
   else if (L1 >= 1.0) { // T1
      for (j = 0; j < 3; j++) {
         dshape(4,j) = DL0[j];
         dshape(1,j) = DL1[j];
         dshape(7,j) = DL2[j];
         dshape(8,j) = DL3[j];
      }
   }
   else if (L2 >= 1.0) { // T2
      for (j = 0; j < 3; j++) {
         dshape(5,j) = DL0[j];
         dshape(7,j) = DL1[j];
         dshape(2,j) = DL2[j];
         dshape(9,j) = DL3[j];
      }
   }
   else if (L3 >= 1.0) { // T3
      for (j = 0; j < 3; j++) {
         dshape(6,j) = DL0[j];
         dshape(8,j) = DL1[j];
         dshape(9,j) = DL2[j];
         dshape(3,j) = DL3[j];
      }
   }
   else if ((L4 <= 1.0) && (L5 <= 1.0)) { // T4
      for (j = 0; j < 3; j++) {
         dshape(4,j) = - DL5[j];
         dshape(5,j) =   DL2[j];
         dshape(6,j) = - DL4[j];
         dshape(8,j) = - DL0[j];
      }
   }
   else if ((L4 >= 1.0) && (L5 <= 1.0)) { // T5
      for (j = 0; j < 3; j++) {
         dshape(4,j) = - DL5[j];
         dshape(5,j) = - DL1[j];
         dshape(7,j) =   DL4[j];
         dshape(8,j) =   DL3[j];
      }
   }
   else if ((L4 <= 1.0) && (L5 >= 1.0)) { // T6
      for (j = 0; j < 3; j++) {
         dshape(5,j) = - DL3[j];
         dshape(6,j) = - DL4[j];
         dshape(8,j) =   DL1[j];
         dshape(9,j) =   DL5[j];
      }
   }
   else if ((L4 >= 1.0) && (L5 >= 1.0)) { // T7
      for (j = 0; j < 3; j++) {
         dshape(5,j) =   DL0[j];
         dshape(7,j) =   DL4[j];
         dshape(8,j) = - DL2[j];
         dshape(9,j) =   DL5[j];
      }
   }
}


RefinedBiLinear2DFiniteElement::RefinedBiLinear2DFiniteElement()
   : NodalFiniteElement(2, Geometry::SQUARE , 9, 1, FunctionSpace::rQk)
{
   Nodes.IntPoint(0).x = 0.0;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(5).x = 1.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(6).x = 0.5;
   Nodes.IntPoint(6).y = 1.0;
   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(8).x = 0.5;
   Nodes.IntPoint(8).y = 0.5;
}

void RefinedBiLinear2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                               Vector &shape) const
{
   int i;
   double x = ip.x, y = ip.y;
   double Lx, Ly;
   Lx = 2.0 * ( 1. - x );
   Ly = 2.0 * ( 1. - y );

   // The reference square is split in 4 squares as follows:
   //
   // T0 - 0,4,7,8
   // T1 - 1,4,5,8
   // T2 - 2,5,6,8
   // T3 - 3,6,7,8

   for (i = 0; i < 9; i++)
      shape(i) = 0.0;

   if ((x <= 0.5) && (y <= 0.5)) { // T0
      shape(0) = (Lx - 1.0) * (Ly - 1.0);
      shape(4) = (2.0 - Lx) * (Ly - 1.0);
      shape(8) = (2.0 - Lx) * (2.0 - Ly);
      shape(7) = (Lx - 1.0) * (2.0 - Ly);
   }
   else if ((x >= 0.5) && (y <= 0.5)) { // T1
      shape(4) =        Lx  * (Ly - 1.0);
      shape(1) = (1.0 - Lx) * (Ly - 1.0);
      shape(5) = (1.0 - Lx) * (2.0 - Ly);
      shape(8) =        Lx  * (2.0 - Ly);
   }
   else if ((x >= 0.5) && (y >= 0.5)) { // T2
      shape(8) =        Lx  *        Ly ;
      shape(5) = (1.0 - Lx) *        Ly ;
      shape(2) = (1.0 - Lx) * (1.0 - Ly);
      shape(6) =        Lx  * (1.0 - Ly);
   }
   else if ((x <= 0.5) && (y >= 0.5)) { // T3
      shape(7) = (Lx - 1.0) *        Ly ;
      shape(8) = (2.0 - Lx) *        Ly ;
      shape(6) = (2.0 - Lx) * (1.0 - Ly);
      shape(3) = (Lx - 1.0) * (1.0 - Ly);
   }
}

void RefinedBiLinear2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                                DenseMatrix &dshape) const
{
   int i,j;
   double x = ip.x, y = ip.y;
   double Lx, Ly;
   Lx = 2.0 * ( 1. - x );
   Ly = 2.0 * ( 1. - y );

   for (i = 0; i < 9; i++)
      for (j = 0; j < 2; j++)
         dshape(i,j) = 0.0;

   if ((x <= 0.5) && (y <= 0.5)) { // T0
      dshape(0,0) =  2.0 * (1.0 - Ly);
      dshape(0,1) =  2.0 * (1.0 - Lx);

      dshape(4,0) =  2.0 * (Ly - 1.0);
      dshape(4,1) = -2.0 * (2.0 - Lx);

      dshape(8,0) =  2.0 * (2.0 - Ly);
      dshape(8,1) =  2.0 * (2.0 - Lx);

      dshape(7,0) = -2.0 * (2.0 - Ly);
      dshape(7,0) =  2.0 * (Lx - 1.0);
   }
   else if ((x >= 0.5) && (y <= 0.5)) { // T1
      dshape(4,0) = -2.0 * (Ly - 1.0);
      dshape(4,1) = -2.0 * Lx;

      dshape(1,0) =  2.0 * (Ly - 1.0);
      dshape(1,1) = -2.0 * (1.0 - Lx);

      dshape(5,0) =  2.0 * (2.0 - Ly);
      dshape(5,1) =  2.0 * (1.0 - Lx);

      dshape(8,0) = -2.0 * (2.0 - Ly);
      dshape(8,1) =  2.0 * Lx;
   }
   else if ((x >= 0.5) && (y >= 0.5)) { // T2
      dshape(8,0) = -2.0 * Ly;
      dshape(8,1) = -2.0 * Lx;

      dshape(5,0) =  2.0 * Ly;
      dshape(5,1) = -2.0 * (1.0 - Lx);

      dshape(2,0) =  2.0 * (1.0 - Ly);
      dshape(2,1) =  2.0 * (1.0 - Lx);

      dshape(6,0) = -2.0 * (1.0 - Ly);
      dshape(6,1) =  2.0 * Lx;
   }
   else if ((x <= 0.5) && (y >= 0.5)) { // T3
      dshape(7,0) = -2.0 * Ly;
      dshape(7,1) = -2.0 * (Lx - 1.0);

      dshape(8,0) =  2.0 * Ly ;
      dshape(8,1) = -2.0 * (2.0 - Lx);

      dshape(6,0) = 2.0 * (1.0 - Ly);
      dshape(6,1) = 2.0 * (2.0 - Lx);

      dshape(3,0) = -2.0 * (1.0 - Ly);
      dshape(3,1) =  2.0 * (Lx - 1.0);
   }
}

RefinedTriLinear3DFiniteElement::RefinedTriLinear3DFiniteElement()
   : NodalFiniteElement(3, Geometry::CUBE, 27, 2, FunctionSpace::rQk)
{
   double I[27];
   double J[27];
   double K[27];
   // nodes
   I[ 0] = 0.0; J[ 0] = 0.0; K[ 0] = 0.0;
   I[ 1] = 1.0; J[ 1] = 0.0; K[ 1] = 0.0;
   I[ 2] = 1.0; J[ 2] = 1.0; K[ 2] = 0.0;
   I[ 3] = 0.0; J[ 3] = 1.0; K[ 3] = 0.0;
   I[ 4] = 0.0; J[ 4] = 0.0; K[ 4] = 1.0;
   I[ 5] = 1.0; J[ 5] = 0.0; K[ 5] = 1.0;
   I[ 6] = 1.0; J[ 6] = 1.0; K[ 6] = 1.0;
   I[ 7] = 0.0; J[ 7] = 1.0; K[ 7] = 1.0;
   // edges
   I[ 8] = 0.5; J[ 8] = 0.0; K[ 8] = 0.0;
   I[ 9] = 1.0; J[ 9] = 0.5; K[ 9] = 0.0;
   I[10] = 0.5; J[10] = 1.0; K[10] = 0.0;
   I[11] = 0.0; J[11] = 0.5; K[11] = 0.0;
   I[12] = 0.5; J[12] = 0.0; K[12] = 1.0;
   I[13] = 1.0; J[13] = 0.5; K[13] = 1.0;
   I[14] = 0.5; J[14] = 1.0; K[14] = 1.0;
   I[15] = 0.0; J[15] = 0.5; K[15] = 1.0;
   I[16] = 0.0; J[16] = 0.0; K[16] = 0.5;
   I[17] = 1.0; J[17] = 0.0; K[17] = 0.5;
   I[18] = 1.0; J[18] = 1.0; K[18] = 0.5;
   I[19] = 0.0; J[19] = 1.0; K[19] = 0.5;
   // faces
   I[20] = 0.5; J[20] = 0.5; K[20] = 0.0;
   I[21] = 0.5; J[21] = 0.0; K[21] = 0.5;
   I[22] = 1.0; J[22] = 0.5; K[22] = 0.5;
   I[23] = 0.5; J[23] = 1.0; K[23] = 0.5;
   I[24] = 0.0; J[24] = 0.5; K[24] = 0.5;
   I[25] = 0.5; J[25] = 0.5; K[25] = 1.0;
   // element
   I[26] = 0.5; J[26] = 0.5; K[26] = 0.5;

   for (int n = 0; n < 27; n++) {
      Nodes.IntPoint(n).x = I[n];
      Nodes.IntPoint(n).y = J[n];
      Nodes.IntPoint(n).z = K[n];
   }
}

void RefinedTriLinear3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                                Vector &shape) const
{
   int i, N[8];
   double Lx, Ly, Lz;
   double x = ip.x, y = ip.y, z = ip.z;

   for (i = 0; i < 27; i++)
      shape(i) = 0.0;

   if ((x <= 0.5) && (y <= 0.5) && (z <= 0.5)) { // T0
      Lx = 1.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] =  0;
      N[1] =  8;
      N[2] = 20;
      N[3] = 11;
      N[4] = 16;
      N[5] = 21;
      N[6] = 26;
      N[7] = 24;
   }
   else if ((x >= 0.5) && (y <= 0.5) && (z <= 0.5)) { // T1
      Lx = 2.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] =  8;
      N[1] =  1;
      N[2] =  9;
      N[3] = 20;
      N[4] = 21;
      N[5] = 17;
      N[6] = 22;
      N[7] = 26;
   }
   else if ((x <= 0.5) && (y >= 0.5) && (z <= 0.5)) { // T2
      Lx = 2.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] = 20;
      N[1] =  9;
      N[2] =  2;
      N[3] = 10;
      N[4] = 26;
      N[5] = 22;
      N[6] = 18;
      N[7] = 23;
   }
   else if ((x >= 0.5) && (y >= 0.5) && (z <= 0.5)) { // T3
      Lx = 1.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] = 11;
      N[1] = 20;
      N[2] = 10;
      N[3] =  3;
      N[4] = 24;
      N[5] = 26;
      N[6] = 23;
      N[7] = 19;
   }
   else if ((x <= 0.5) && (y <= 0.5) && (z >= 0.5)) { // T4
      Lx = 1.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 16;
      N[1] = 21;
      N[2] = 26;
      N[3] = 24;
      N[4] =  4;
      N[5] = 12;
      N[6] = 25;
      N[7] = 15;
   }
   else if ((x >= 0.5) && (y <= 0.5) && (z >= 0.5)) { // T5
      Lx = 2.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 21;
      N[1] = 17;
      N[2] = 22;
      N[3] = 26;
      N[4] = 12;
      N[5] =  5;
      N[6] = 13;
      N[7] = 25;
   }
   else if ((x <= 0.5) && (y >= 0.5) && (z >= 0.5)) { // T6
      Lx = 2.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 26;
      N[1] = 22;
      N[2] = 18;
      N[3] = 23;
      N[4] = 25;
      N[5] = 13;
      N[6] =  6;
      N[7] = 14;
   }
   else if ((x >= 0.5) && (y >= 0.5) && (z >= 0.5)) { // T7
      Lx = 1.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 24;
      N[1] = 26;
      N[2] = 23;
      N[3] = 19;
      N[4] = 15;
      N[5] = 25;
      N[6] = 14;
      N[7] =  7;
   }

   shape(N[0]) = Lx       * Ly       * Lz;
   shape(N[1]) = (1 - Lx) * Ly       * Lz;
   shape(N[2]) = (1 - Lx) * (1 - Ly) * Lz;
   shape(N[3]) = Lx       * (1 - Ly) * Lz;
   shape(N[4]) = Lx       * Ly       * (1 - Lz);
   shape(N[5]) = (1 - Lx) * Ly       * (1 - Lz);
   shape(N[6]) = (1 - Lx) * (1 - Ly) * (1 - Lz);
   shape(N[7]) = Lx       * (1 - Ly) * (1 - Lz);
}

void RefinedTriLinear3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                                 DenseMatrix &dshape) const
{
   int i, j, N[8];
   double Lx, Ly, Lz;
   double x = ip.x, y = ip.y, z = ip.z;

   for (i = 0; i < 27; i++)
      for (j = 0; j < 3; j++)
         dshape(i,j) = 0.0;

   if ((x <= 0.5) && (y <= 0.5) && (z <= 0.5)) { // T0
      Lx = 1.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] =  0;
      N[1] =  8;
      N[2] = 20;
      N[3] = 11;
      N[4] = 16;
      N[5] = 21;
      N[6] = 26;
      N[7] = 24;
   }
   else if ((x >= 0.5) && (y <= 0.5) && (z <= 0.5)) { // T1
      Lx = 2.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] =  8;
      N[1] =  1;
      N[2] =  9;
      N[3] = 20;
      N[4] = 21;
      N[5] = 17;
      N[6] = 22;
      N[7] = 26;
   }
   else if ((x <= 0.5) && (y >= 0.5) && (z <= 0.5)) { // T2
      Lx = 2.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] = 20;
      N[1] =  9;
      N[2] =  2;
      N[3] = 10;
      N[4] = 26;
      N[5] = 22;
      N[6] = 18;
      N[7] = 23;
   }
   else if ((x >= 0.5) && (y >= 0.5) && (z <= 0.5)) { // T3
      Lx = 1.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 1.0 - 2.0 * z;

      N[0] = 11;
      N[1] = 20;
      N[2] = 10;
      N[3] =  3;
      N[4] = 24;
      N[5] = 26;
      N[6] = 23;
      N[7] = 19;
   }
   else if ((x <= 0.5) && (y <= 0.5) && (z >= 0.5)) { // T4
      Lx = 1.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 16;
      N[1] = 21;
      N[2] = 26;
      N[3] = 24;
      N[4] =  4;
      N[5] = 12;
      N[6] = 25;
      N[7] = 15;
   }
   else if ((x >= 0.5) && (y <= 0.5) && (z >= 0.5)) { // T5
      Lx = 2.0 - 2.0 * x;
      Ly = 1.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 21;
      N[1] = 17;
      N[2] = 22;
      N[3] = 26;
      N[4] = 12;
      N[5] =  5;
      N[6] = 13;
      N[7] = 25;
   }
   else if ((x <= 0.5) && (y >= 0.5) && (z >= 0.5)) { // T6
      Lx = 2.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 26;
      N[1] = 22;
      N[2] = 18;
      N[3] = 23;
      N[4] = 25;
      N[5] = 13;
      N[6] =  6;
      N[7] = 14;
   }
   else if ((x >= 0.5) && (y >= 0.5) && (z >= 0.5)) { // T7
      Lx = 1.0 - 2.0 * x;
      Ly = 2.0 - 2.0 * y;
      Lz = 2.0 - 2.0 * z;

      N[0] = 24;
      N[1] = 26;
      N[2] = 23;
      N[3] = 19;
      N[4] = 15;
      N[5] = 25;
      N[6] = 14;
      N[7] =  7;
   }

   dshape(N[0],0) = -2.0 * Ly       * Lz      ;
   dshape(N[0],1) = -2.0 * Lx       * Lz      ;
   dshape(N[0],2) = -2.0 * Lx       * Ly      ;

   dshape(N[1],0) =  2.0 * Ly       * Lz      ;
   dshape(N[1],1) = -2.0 * (1 - Lx) * Lz      ;
   dshape(N[1],2) = -2.0 * (1 - Lx) * Ly      ;

   dshape(N[2],0) =  2.0 * (1 - Ly) * Lz      ;
   dshape(N[2],1) =  2.0 * (1 - Lx) * Lz      ;
   dshape(N[2],2) = -2.0 * (1 - Lx) * (1 - Ly);

   dshape(N[3],0) = -2.0 * (1 - Ly) * Lz      ;
   dshape(N[3],1) =  2.0 * Lx       * Lz      ;
   dshape(N[3],2) = -2.0 * Lx       * (1 - Ly);

   dshape(N[4],0) = -2.0 * Ly       * (1 - Lz);
   dshape(N[4],1) = -2.0 * Lx       * (1 - Lz);
   dshape(N[4],2) =  2.0 * Lx       * Ly      ;

   dshape(N[5],0) =  2.0 * Ly       * (1 - Lz);
   dshape(N[5],1) = -2.0 * (1 - Lx) * (1 - Lz);
   dshape(N[5],2) =  2.0 * (1 - Lx) * Ly      ;

   dshape(N[6],0) =  2.0 * (1 - Ly) * (1 - Lz);
   dshape(N[6],1) =  2.0 * (1 - Lx) * (1 - Lz);
   dshape(N[6],2) =  2.0 * (1 - Lx) * (1 - Ly);

   dshape(N[7],0) = -2.0 * (1 - Ly) * (1 - Lz);
   dshape(N[7],1) =  2.0 * Lx       * (1 - Lz);
   dshape(N[7],2) =  2.0 * Lx       * (1 - Ly);
}


Nedelec1HexFiniteElement::Nedelec1HexFiniteElement()
   : VectorFiniteElement (3, Geometry::CUBE, 12, 1, FunctionSpace::Qk)
{
   // not real nodes ...
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;

   Nodes.IntPoint(1).x = 1.0;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(1).z = 0.0;

   Nodes.IntPoint(2).x = 0.5;
   Nodes.IntPoint(2).y = 1.0;
   Nodes.IntPoint(2).z = 0.0;

   Nodes.IntPoint(3).x = 0.0;
   Nodes.IntPoint(3).y = 0.5;
   Nodes.IntPoint(3).z = 0.0;

   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(4).z = 1.0;

   Nodes.IntPoint(5).x = 1.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 1.0;

   Nodes.IntPoint(6).x = 0.5;
   Nodes.IntPoint(6).y = 1.0;
   Nodes.IntPoint(6).z = 1.0;

   Nodes.IntPoint(7).x = 0.0;
   Nodes.IntPoint(7).y = 0.5;
   Nodes.IntPoint(7).z = 1.0;

   Nodes.IntPoint(8).x = 0.0;
   Nodes.IntPoint(8).y = 0.0;
   Nodes.IntPoint(8).z = 0.5;

   Nodes.IntPoint(9).x = 1.0;
   Nodes.IntPoint(9).y = 0.0;
   Nodes.IntPoint(9).z = 0.5;

   Nodes.IntPoint(10).x= 1.0;
   Nodes.IntPoint(10).y= 1.0;
   Nodes.IntPoint(10).z= 0.5;

   Nodes.IntPoint(11).x= 0.0;
   Nodes.IntPoint(11).y= 1.0;
   Nodes.IntPoint(11).z= 0.5;
}

void Nedelec1HexFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                          DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;

   shape(0,0) = (1. - y) * (1. - z);
   shape(0,1) = 0.;
   shape(0,2) = 0.;

   shape(2,0) = y * (1. - z);
   shape(2,1) = 0.;
   shape(2,2) = 0.;

   shape(4,0) = z * (1. - y);
   shape(4,1) = 0.;
   shape(4,2) = 0.;

   shape(6,0) = y * z;
   shape(6,1) = 0.;
   shape(6,2) = 0.;

   shape(1,0) = 0.;
   shape(1,1) = x * (1. - z);
   shape(1,2) = 0.;

   shape(3,0) = 0.;
   shape(3,1) = (1. - x) * (1. - z);
   shape(3,2) = 0.;

   shape(5,0) = 0.;
   shape(5,1) = x * z;
   shape(5,2) = 0.;

   shape(7,0) = 0.;
   shape(7,1) = (1. - x) * z;
   shape(7,2) = 0.;

   shape(8,0) = 0.;
   shape(8,1) = 0.;
   shape(8,2) = (1. - x) * (1. - y);

   shape(9,0) = 0.;
   shape(9,1) = 0.;
   shape(9,2) = x * (1. - y);

   shape(10,0) = 0.;
   shape(10,1) = 0.;
   shape(10,2) = x * y;

   shape(11,0) = 0.;
   shape(11,1) = 0.;
   shape(11,2) = y * (1. - x);

}

void Nedelec1HexFiniteElement::CalcCurlShape(const IntegrationPoint &ip,
                                             DenseMatrix &curl_shape)
   const
{
   double x = ip.x, y = ip.y, z = ip.z;

   curl_shape(0,0) = 0.;
   curl_shape(0,1) = y - 1.;
   curl_shape(0,2) = 1. - z;

   curl_shape(2,0) = 0.;
   curl_shape(2,1) = -y;
   curl_shape(2,2) = z - 1.;

   curl_shape(4,0) = 0;
   curl_shape(4,1) = 1. - y;
   curl_shape(4,2) = z;

   curl_shape(6,0) = 0.;
   curl_shape(6,1) = y;
   curl_shape(6,2) = -z;

   curl_shape(1,0) = x;
   curl_shape(1,1) = 0.;
   curl_shape(1,2) = 1. - z;

   curl_shape(3,0) = 1. - x;
   curl_shape(3,1) = 0.;
   curl_shape(3,2) = z - 1.;

   curl_shape(5,0) = -x;
   curl_shape(5,1) = 0.;
   curl_shape(5,2) = z;

   curl_shape(7,0) = x - 1.;
   curl_shape(7,1) = 0.;
   curl_shape(7,2) = -z;

   curl_shape(8,0) = x - 1.;
   curl_shape(8,1) = 1. - y;
   curl_shape(8,2) = 0.;

   curl_shape(9,0) = -x;
   curl_shape(9,1) = y - 1.;
   curl_shape(9,2) = 0;

   curl_shape(10,0) = x;
   curl_shape(10,1) = -y;
   curl_shape(10,2) = 0.;

   curl_shape(11,0) = 1. - x;
   curl_shape(11,1) = y;
   curl_shape(11,2) = 0.;
}

const double Nedelec1HexFiniteElement::tk[12][3] =
{{1,0,0}, {0,1,0}, {1,0,0}, {0,1,0},
 {1,0,0}, {0,1,0}, {1,0,0}, {0,1,0},
 {0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}};

void Nedelec1HexFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 12; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 12; j++)
      {
         double d = ( vshape(j,0)*tk[k][0] + vshape(j,1)*tk[k][1] +
                      vshape(j,2)*tk[k][2] );
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "Nedelec1HexFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = ip.z = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear (more to have embedding?)
   const DenseMatrix &J = Trans.Jacobian();
   double vk[3];
   Vector xk (vk, 3);

   for (k = 0; k < 12; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1]; ip.z = vk[2];
      CalcVShape (ip, vshape);
      //  vk = J tk
      vk[0] = J(0,0)*tk[k][0]+J(0,1)*tk[k][1]+J(0,2)*tk[k][2];
      vk[1] = J(1,0)*tk[k][0]+J(1,1)*tk[k][1]+J(1,2)*tk[k][2];
      vk[2] = J(2,0)*tk[k][0]+J(2,1)*tk[k][1]+J(2,2)*tk[k][2];
      for (j = 0; j < 12; j++)
         if (fabs (I(k,j) = (vshape(j,0)*vk[0]+vshape(j,1)*vk[1]+
                             vshape(j,2)*vk[2])) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void Nedelec1HexFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[3];
   Vector xk (vk, 3);

   for (int k = 0; k < 12; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      const DenseMatrix &J = Trans.Jacobian();

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t J tk
      dofs(k) =
         vk[0] * ( J(0,0)*tk[k][0]+J(0,1)*tk[k][1]+J(0,2)*tk[k][2] ) +
         vk[1] * ( J(1,0)*tk[k][0]+J(1,1)*tk[k][1]+J(1,2)*tk[k][2] ) +
         vk[2] * ( J(2,0)*tk[k][0]+J(2,1)*tk[k][1]+J(2,2)*tk[k][2] );
   }
}


Nedelec1TetFiniteElement::Nedelec1TetFiniteElement()
   : VectorFiniteElement (3, Geometry::TETRAHEDRON, 6, 1)
{
   // not real nodes ...
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.0;
   Nodes.IntPoint(0).z = 0.0;

   Nodes.IntPoint(1).x = 0.0;
   Nodes.IntPoint(1).y = 0.5;
   Nodes.IntPoint(1).z = 0.0;

   Nodes.IntPoint(2).x = 0.0;
   Nodes.IntPoint(2).y = 0.0;
   Nodes.IntPoint(2).z = 0.5;

   Nodes.IntPoint(3).x = 0.5;
   Nodes.IntPoint(3).y = 0.5;
   Nodes.IntPoint(3).z = 0.0;

   Nodes.IntPoint(4).x = 0.5;
   Nodes.IntPoint(4).y = 0.0;
   Nodes.IntPoint(4).z = 0.5;

   Nodes.IntPoint(5).x = 0.0;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 0.5;
}

void Nedelec1TetFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                          DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;

   shape(0,0) = 1. - y - z;
   shape(0,1) = x;
   shape(0,2) = x;

   shape(1,0) = y;
   shape(1,1) = 1. - x - z;
   shape(1,2) = y;

   shape(2,0) = z;
   shape(2,1) = z;
   shape(2,2) = 1. - x - y;

   shape(3,0) = -y;
   shape(3,1) = x;
   shape(3,2) = 0.;

   shape(4,0) = -z;
   shape(4,1) = 0.;
   shape(4,2) = x;

   shape(5,0) = 0.;
   shape(5,1) = -z;
   shape(5,2) = y;
}

void Nedelec1TetFiniteElement::CalcCurlShape(const IntegrationPoint &ip,
                                             DenseMatrix &curl_shape)
   const
{
   curl_shape(0,0) =  0.;
   curl_shape(0,1) = -2.;
   curl_shape(0,2) =  2.;

   curl_shape(1,0) =  2.;
   curl_shape(1,1) =  0.;
   curl_shape(1,2) = -2.;

   curl_shape(2,0) = -2.;
   curl_shape(2,1) =  2.;
   curl_shape(2,2) =  0.;

   curl_shape(3,0) = 0.;
   curl_shape(3,1) = 0.;
   curl_shape(3,2) = 2.;

   curl_shape(4,0) =  0.;
   curl_shape(4,1) = -2.;
   curl_shape(4,2) =  0.;

   curl_shape(5,0) = 2.;
   curl_shape(5,1) = 0.;
   curl_shape(5,2) = 0.;
}

const double Nedelec1TetFiniteElement::tk[6][3] =
{{1,0,0}, {0,1,0}, {0,0,1}, {-1,1,0}, {-1,0,1}, {0,-1,1}};

void Nedelec1TetFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 6; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 6; j++)
      {
         double d = ( vshape(j,0)*tk[k][0] + vshape(j,1)*tk[k][1] +
                      vshape(j,2)*tk[k][2] );
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "Nedelec1TetFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = ip.z = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear
   const DenseMatrix &J = Trans.Jacobian();
   double vk[3];
   Vector xk (vk, 3);

   for (k = 0; k < 6; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1]; ip.z = vk[2];
      CalcVShape (ip, vshape);
      //  vk = J tk
      vk[0] = J(0,0)*tk[k][0]+J(0,1)*tk[k][1]+J(0,2)*tk[k][2];
      vk[1] = J(1,0)*tk[k][0]+J(1,1)*tk[k][1]+J(1,2)*tk[k][2];
      vk[2] = J(2,0)*tk[k][0]+J(2,1)*tk[k][1]+J(2,2)*tk[k][2];
      for (j = 0; j < 6; j++)
         if (fabs (I(k,j) = (vshape(j,0)*vk[0]+vshape(j,1)*vk[1]+
                             vshape(j,2)*vk[2])) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void Nedelec1TetFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[3];
   Vector xk (vk, 3);

   for (int k = 0; k < 6; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      const DenseMatrix &J = Trans.Jacobian();

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t J tk
      dofs(k) =
         vk[0] * ( J(0,0)*tk[k][0]+J(0,1)*tk[k][1]+J(0,2)*tk[k][2] ) +
         vk[1] * ( J(1,0)*tk[k][0]+J(1,1)*tk[k][1]+J(1,2)*tk[k][2] ) +
         vk[2] * ( J(2,0)*tk[k][0]+J(2,1)*tk[k][1]+J(2,2)*tk[k][2] );
   }
}

RT0HexFiniteElement::RT0HexFiniteElement()
   : VectorFiniteElement (3, Geometry::CUBE, 6, 1, FunctionSpace::Qk)
{
   // not real nodes ...
   // z = 0, y = 0, x = 1, y = 1, x = 0, z = 1
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.5;
   Nodes.IntPoint(0).z = 0.0;

   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.5;

   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 0.5;
   Nodes.IntPoint(2).z = 0.5;

   Nodes.IntPoint(3).x = 0.5;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(3).z = 0.5;

   Nodes.IntPoint(4).x = 0.0;
   Nodes.IntPoint(4).y = 0.5;
   Nodes.IntPoint(4).z = 0.5;

   Nodes.IntPoint(5).x = 0.5;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 1.0;
}

void RT0HexFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                     DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;
   // z = 0
   shape(0,0) = 0.;
   shape(0,1) = 0.;
   shape(0,2) = z - 1.;
   // y = 0
   shape(1,0) = 0.;
   shape(1,1) = y - 1.;
   shape(1,2) = 0.;
   // x = 1
   shape(2,0) = x;
   shape(2,1) = 0.;
   shape(2,2) = 0.;
   // y = 1
   shape(3,0) = 0.;
   shape(3,1) = y;
   shape(3,2) = 0.;
   // x = 0
   shape(4,0) = x - 1.;
   shape(4,1) = 0.;
   shape(4,2) = 0.;
   // z = 1
   shape(5,0) = 0.;
   shape(5,1) = 0.;
   shape(5,2) = z;
}

void RT0HexFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                       Vector &divshape) const
{
   divshape(0) = 1.;
   divshape(1) = 1.;
   divshape(2) = 1.;
   divshape(3) = 1.;
   divshape(4) = 1.;
   divshape(5) = 1.;
}

const double RT0HexFiniteElement::nk[6][3] =
{{0,0,-1}, {0,-1,0}, {1,0,0}, {0,1,0}, {-1,0,0}, {0,0,1}};

void RT0HexFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 6; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 6; j++)
      {
         double d = ( vshape(j,0)*nk[k][0] + vshape(j,1)*nk[k][1] +
                      vshape(j,2)*nk[k][2] );
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT0HexFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = ip.z = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[3];
   Vector xk (vk, 3);

   for (k = 0; k < 6; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1]; ip.z = vk[2];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2];
      vk[2] = Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2];
      for (j = 0; j < 6; j++)
         if (fabs (I(k,j) = (vshape(j,0)*vk[0]+vshape(j,1)*vk[1]+
                             vshape(j,2)*vk[2])) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT0HexFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[3];
   Vector xk (vk, 3);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 6; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) =
         vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2] ) +
         vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2] ) +
         vk[2] * ( Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2] );
   }
}

RT1HexFiniteElement::RT1HexFiniteElement()
   : VectorFiniteElement (3, Geometry::CUBE, 36, 2, FunctionSpace::Qk)
{
   // z = 0
   Nodes.IntPoint(2).x  = 1./3.;
   Nodes.IntPoint(2).y  = 1./3.;
   Nodes.IntPoint(2).z  = 0.0;
   Nodes.IntPoint(3).x  = 2./3.;
   Nodes.IntPoint(3).y  = 1./3.;
   Nodes.IntPoint(3).z  = 0.0;
   Nodes.IntPoint(0).x  = 1./3.;
   Nodes.IntPoint(0).y  = 2./3.;
   Nodes.IntPoint(0).z  = 0.0;
   Nodes.IntPoint(1).x  = 2./3.;
   Nodes.IntPoint(1).y  = 2./3.;
   Nodes.IntPoint(1).z  = 0.0;
   // y = 0
   Nodes.IntPoint(4).x  = 1./3.;
   Nodes.IntPoint(4).y  = 0.0;
   Nodes.IntPoint(4).z  = 1./3.;
   Nodes.IntPoint(5).x  = 2./3.;
   Nodes.IntPoint(5).y  = 0.0;
   Nodes.IntPoint(5).z  = 1./3.;
   Nodes.IntPoint(6).x  = 1./3.;
   Nodes.IntPoint(6).y  = 0.0;
   Nodes.IntPoint(6).z  = 2./3.;
   Nodes.IntPoint(7).x  = 2./3.;
   Nodes.IntPoint(7).y  = 0.0;
   Nodes.IntPoint(7).z  = 2./3.;
   // x = 1
   Nodes.IntPoint(8).x  = 1.0;
   Nodes.IntPoint(8).y  = 1./3.;
   Nodes.IntPoint(8).z  = 1./3.;
   Nodes.IntPoint(9).x  = 1.0;
   Nodes.IntPoint(9).y  = 2./3.;
   Nodes.IntPoint(9).z  = 1./3.;
   Nodes.IntPoint(10).x = 1.0;
   Nodes.IntPoint(10).y = 1./3.;
   Nodes.IntPoint(10).z = 2./3.;
   Nodes.IntPoint(11).x = 1.0;
   Nodes.IntPoint(11).y = 2./3.;
   Nodes.IntPoint(11).z = 2./3.;
   // y = 1
   Nodes.IntPoint(13).x = 1./3.;
   Nodes.IntPoint(13).y = 1.0;
   Nodes.IntPoint(13).z = 1./3.;
   Nodes.IntPoint(12).x = 2./3.;
   Nodes.IntPoint(12).y = 1.0;
   Nodes.IntPoint(12).z = 1./3.;
   Nodes.IntPoint(15).x = 1./3.;
   Nodes.IntPoint(15).y = 1.0;
   Nodes.IntPoint(15).z = 2./3.;
   Nodes.IntPoint(14).x = 2./3.;
   Nodes.IntPoint(14).y = 1.0;
   Nodes.IntPoint(14).z = 2./3.;
   // x = 0
   Nodes.IntPoint(17).x = 0.0;
   Nodes.IntPoint(17).y = 1./3.;
   Nodes.IntPoint(17).z = 1./3.;
   Nodes.IntPoint(16).x = 0.0;
   Nodes.IntPoint(16).y = 2./3.;
   Nodes.IntPoint(16).z = 1./3.;
   Nodes.IntPoint(19).x = 0.0;
   Nodes.IntPoint(19).y = 1./3.;
   Nodes.IntPoint(19).z = 2./3.;
   Nodes.IntPoint(18).x = 0.0;
   Nodes.IntPoint(18).y = 2./3.;
   Nodes.IntPoint(18).z = 2./3.;
   // z = 1
   Nodes.IntPoint(20).x = 1./3.;
   Nodes.IntPoint(20).y = 1./3.;
   Nodes.IntPoint(20).z = 1.0;
   Nodes.IntPoint(21).x = 2./3.;
   Nodes.IntPoint(21).y = 1./3.;
   Nodes.IntPoint(21).z = 1.0;
   Nodes.IntPoint(22).x = 1./3.;
   Nodes.IntPoint(22).y = 2./3.;
   Nodes.IntPoint(22).z = 1.0;
   Nodes.IntPoint(23).x = 2./3.;
   Nodes.IntPoint(23).y = 2./3.;
   Nodes.IntPoint(23).z = 1.0;
   // x = 0.5 (interior)
   Nodes.IntPoint(24).x = 0.5;
   Nodes.IntPoint(24).y = 1./3.;
   Nodes.IntPoint(24).z = 1./3.;
   Nodes.IntPoint(25).x = 0.5;
   Nodes.IntPoint(25).y = 1./3.;
   Nodes.IntPoint(25).z = 2./3.;
   Nodes.IntPoint(26).x = 0.5;
   Nodes.IntPoint(26).y = 2./3.;
   Nodes.IntPoint(26).z = 1./3.;
   Nodes.IntPoint(27).x = 0.5;
   Nodes.IntPoint(27).y = 2./3.;
   Nodes.IntPoint(27).z = 2./3.;
   // y = 0.5 (interior)
   Nodes.IntPoint(28).x = 1./3.;
   Nodes.IntPoint(28).y = 0.5;
   Nodes.IntPoint(28).z = 1./3.;
   Nodes.IntPoint(29).x = 1./3.;
   Nodes.IntPoint(29).y = 0.5;
   Nodes.IntPoint(29).z = 2./3.;
   Nodes.IntPoint(30).x = 2./3.;
   Nodes.IntPoint(30).y = 0.5;
   Nodes.IntPoint(30).z = 1./3.;
   Nodes.IntPoint(31).x = 2./3.;
   Nodes.IntPoint(31).y = 0.5;
   Nodes.IntPoint(31).z = 2./3.;
   // z = 0.5 (interior)
   Nodes.IntPoint(32).x = 1./3.;
   Nodes.IntPoint(32).y = 1./3.;
   Nodes.IntPoint(32).z = 0.5;
   Nodes.IntPoint(33).x = 1./3.;
   Nodes.IntPoint(33).y = 2./3.;
   Nodes.IntPoint(33).z = 0.5;
   Nodes.IntPoint(34).x = 2./3.;
   Nodes.IntPoint(34).y = 1./3.;
   Nodes.IntPoint(34).z = 0.5;
   Nodes.IntPoint(35).x = 2./3.;
   Nodes.IntPoint(35).y = 2./3.;
   Nodes.IntPoint(35).z = 0.5;
}

void RT1HexFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                     DenseMatrix &shape) const
{
   double x = ip.x, y = ip.y, z = ip.z;
   // z = 0
   shape(2,0)  = 0.;
   shape(2,1)  = 0.;
   shape(2,2)  = -(1. - 3.*z + 2.*z*z)*( 2. - 3.*x)*( 2. - 3.*y);
   shape(3,0)  = 0.;
   shape(3,1)  = 0.;
   shape(3,2)  = -(1. - 3.*z + 2.*z*z)*(-1. + 3.*x)*( 2. - 3.*y);
   shape(0,0)  = 0.;
   shape(0,1)  = 0.;
   shape(0,2)  = -(1. - 3.*z + 2.*z*z)*( 2. - 3.*x)*(-1. + 3.*y);
   shape(1,0)  = 0.;
   shape(1,1)  = 0.;
   shape(1,2)  = -(1. - 3.*z + 2.*z*z)*(-1. + 3.*x)*(-1. + 3.*y);
   // y = 0
   shape(4,0)  = 0.;
   shape(4,1)  = -(1. - 3.*y + 2.*y*y)*( 2. - 3.*x)*( 2. - 3.*z);
   shape(4,2)  = 0.;
   shape(5,0)  = 0.;
   shape(5,1)  = -(1. - 3.*y + 2.*y*y)*(-1. + 3.*x)*( 2. - 3.*z);
   shape(5,2)  = 0.;
   shape(6,0)  = 0.;
   shape(6,1)  = -(1. - 3.*y + 2.*y*y)*( 2. - 3.*x)*(-1. + 3.*z);
   shape(6,2)  = 0.;
   shape(7,0)  = 0.;
   shape(7,1)  = -(1. - 3.*y + 2.*y*y)*(-1. + 3.*x)*(-1. + 3.*z);
   shape(7,2)  = 0.;
   // x = 1
   shape(8,0)  = (-x + 2.*x*x)*( 2. - 3.*y)*( 2. - 3.*z);
   shape(8,1)  = 0.;
   shape(8,2)  = 0.;
   shape(9,0)  = (-x + 2.*x*x)*(-1. + 3.*y)*( 2. - 3.*z);
   shape(9,1)  = 0.;
   shape(9,2)  = 0.;
   shape(10,0) = (-x + 2.*x*x)*( 2. - 3.*y)*(-1. + 3.*z);
   shape(10,1) = 0.;
   shape(10,2) = 0.;
   shape(11,0) = (-x + 2.*x*x)*(-1. + 3.*y)*(-1. + 3.*z);
   shape(11,1) = 0.;
   shape(11,2) = 0.;
   // y = 1
   shape(13,0) = 0.;
   shape(13,1) = (-y + 2.*y*y)*( 2. - 3.*x)*( 2. - 3.*z);
   shape(13,2) = 0.;
   shape(12,0) = 0.;
   shape(12,1) = (-y + 2.*y*y)*(-1. + 3.*x)*( 2. - 3.*z);
   shape(12,2) = 0.;
   shape(15,0) = 0.;
   shape(15,1) = (-y + 2.*y*y)*( 2. - 3.*x)*(-1. + 3.*z);
   shape(15,2) = 0.;
   shape(14,0) = 0.;
   shape(14,1) = (-y + 2.*y*y)*(-1. + 3.*x)*(-1. + 3.*z);
   shape(14,2) = 0.;
   // x = 0
   shape(17,0) = -(1. - 3.*x + 2.*x*x)*( 2. - 3.*y)*( 2. - 3.*z);
   shape(17,1) = 0.;
   shape(17,2) = 0.;
   shape(16,0) = -(1. - 3.*x + 2.*x*x)*(-1. + 3.*y)*( 2. - 3.*z);
   shape(16,1) = 0.;
   shape(16,2) = 0.;
   shape(19,0) = -(1. - 3.*x + 2.*x*x)*( 2. - 3.*y)*(-1. + 3.*z);
   shape(19,1) = 0.;
   shape(19,2) = 0.;
   shape(18,0) = -(1. - 3.*x + 2.*x*x)*(-1. + 3.*y)*(-1. + 3.*z);
   shape(18,1) = 0.;
   shape(18,2) = 0.;
   // z = 1
   shape(20,0) = 0.;
   shape(20,1) = 0.;
   shape(20,2) = (-z + 2.*z*z)*( 2. - 3.*x)*( 2. - 3.*y);
   shape(21,0) = 0.;
   shape(21,1) = 0.;
   shape(21,2) = (-z + 2.*z*z)*(-1. + 3.*x)*( 2. - 3.*y);
   shape(22,0) = 0.;
   shape(22,1) = 0.;
   shape(22,2) = (-z + 2.*z*z)*( 2. - 3.*x)*(-1. + 3.*y);
   shape(23,0) = 0.;
   shape(23,1) = 0.;
   shape(23,2) = (-z + 2.*z*z)*(-1. + 3.*x)*(-1. + 3.*y);
   // x = 0.5 (interior)
   shape(24,0) = (4.*x - 4.*x*x)*( 2. - 3.*y)*( 2. - 3.*z);
   shape(24,1) = 0.;
   shape(24,2) = 0.;
   shape(25,0) = (4.*x - 4.*x*x)*( 2. - 3.*y)*(-1. + 3.*z);
   shape(25,1) = 0.;
   shape(25,2) = 0.;
   shape(26,0) = (4.*x - 4.*x*x)*(-1. + 3.*y)*( 2. - 3.*z);
   shape(26,1) = 0.;
   shape(26,2) = 0.;
   shape(27,0) = (4.*x - 4.*x*x)*(-1. + 3.*y)*(-1. + 3.*z);
   shape(27,1) = 0.;
   shape(27,2) = 0.;
   // y = 0.5 (interior)
   shape(28,0) = 0.;
   shape(28,1) = (4.*y - 4.*y*y)*( 2. - 3.*x)*( 2. - 3.*z);
   shape(28,2) = 0.;
   shape(29,0) = 0.;
   shape(29,1) = (4.*y - 4.*y*y)*( 2. - 3.*x)*(-1. + 3.*z);
   shape(29,2) = 0.;
   shape(30,0) = 0.;
   shape(30,1) = (4.*y - 4.*y*y)*(-1. + 3.*x)*( 2. - 3.*z);
   shape(30,2) = 0.;
   shape(31,0) = 0.;
   shape(31,1) = (4.*y - 4.*y*y)*(-1. + 3.*x)*(-1. + 3.*z);
   shape(31,2) = 0.;
   // z = 0.5 (interior)
   shape(32,0) = 0.;
   shape(32,1) = 0.;
   shape(32,2) = (4.*z - 4.*z*z)*( 2. - 3.*x)*( 2. - 3.*y);
   shape(33,0) = 0.;
   shape(33,1) = 0.;
   shape(33,2) = (4.*z - 4.*z*z)*( 2. - 3.*x)*(-1. + 3.*y);
   shape(34,0) = 0.;
   shape(34,1) = 0.;
   shape(34,2) = (4.*z - 4.*z*z)*(-1. + 3.*x)*( 2. - 3.*y);
   shape(35,0) = 0.;
   shape(35,1) = 0.;
   shape(35,2) = (4.*z - 4.*z*z)*(-1. + 3.*x)*(-1. + 3.*y);
}

void RT1HexFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                       Vector &divshape) const
{
   double x = ip.x, y = ip.y, z = ip.z;
   // z = 0
   divshape(2)  = -(-3. + 4.*z)*( 2. - 3.*x)*( 2. - 3.*y);
   divshape(3)  = -(-3. + 4.*z)*(-1. + 3.*x)*( 2. - 3.*y);
   divshape(0)  = -(-3. + 4.*z)*( 2. - 3.*x)*(-1. + 3.*y);
   divshape(1)  = -(-3. + 4.*z)*(-1. + 3.*x)*(-1. + 3.*y);
   // y = 0
   divshape(4)  = -(-3. + 4.*y)*( 2. - 3.*x)*( 2. - 3.*z);
   divshape(5)  = -(-3. + 4.*y)*(-1. + 3.*x)*( 2. - 3.*z);
   divshape(6)  = -(-3. + 4.*y)*( 2. - 3.*x)*(-1. + 3.*z);
   divshape(7)  = -(-3. + 4.*y)*(-1. + 3.*x)*(-1. + 3.*z);
   // x = 1
   divshape(8)  = (-1. + 4.*x)*( 2. - 3.*y)*( 2. - 3.*z);
   divshape(9)  = (-1. + 4.*x)*(-1. + 3.*y)*( 2. - 3.*z);
   divshape(10) = (-1. + 4.*x)*( 2. - 3.*y)*(-1. + 3.*z);
   divshape(11) = (-1. + 4.*x)*(-1. + 3.*y)*(-1. + 3.*z);
   // y = 1
   divshape(13) = (-1. + 4.*y)*( 2. - 3.*x)*( 2. - 3.*z);
   divshape(12) = (-1. + 4.*y)*(-1. + 3.*x)*( 2. - 3.*z);
   divshape(15) = (-1. + 4.*y)*( 2. - 3.*x)*(-1. + 3.*z);
   divshape(14) = (-1. + 4.*y)*(-1. + 3.*x)*(-1. + 3.*z);
   // x = 0
   divshape(17) = -(-3. + 4.*x)*( 2. - 3.*y)*( 2. - 3.*z);
   divshape(16) = -(-3. + 4.*x)*(-1. + 3.*y)*( 2. - 3.*z);
   divshape(19) = -(-3. + 4.*x)*( 2. - 3.*y)*(-1. + 3.*z);
   divshape(18) = -(-3. + 4.*x)*(-1. + 3.*y)*(-1. + 3.*z);
   // z = 1
   divshape(20) = (-1. + 4.*z)*( 2. - 3.*x)*( 2. - 3.*y);
   divshape(21) = (-1. + 4.*z)*(-1. + 3.*x)*( 2. - 3.*y);
   divshape(22) = (-1. + 4.*z)*( 2. - 3.*x)*(-1. + 3.*y);
   divshape(23) = (-1. + 4.*z)*(-1. + 3.*x)*(-1. + 3.*y);
   // x = 0.5 (interior)
   divshape(24) = ( 4. - 8.*x)*( 2. - 3.*y)*( 2. - 3.*z);
   divshape(25) = ( 4. - 8.*x)*( 2. - 3.*y)*(-1. + 3.*z);
   divshape(26) = ( 4. - 8.*x)*(-1. + 3.*y)*( 2. - 3.*z);
   divshape(27) = ( 4. - 8.*x)*(-1. + 3.*y)*(-1. + 3.*z);
   // y = 0.5 (interior)
   divshape(28) = ( 4. - 8.*y)*( 2. - 3.*x)*( 2. - 3.*z);
   divshape(29) = ( 4. - 8.*y)*( 2. - 3.*x)*(-1. + 3.*z);
   divshape(30) = ( 4. - 8.*y)*(-1. + 3.*x)*( 2. - 3.*z);
   divshape(31) = ( 4. - 8.*y)*(-1. + 3.*x)*(-1. + 3.*z);
   // z = 0.5 (interior)
   divshape(32) = ( 4. - 8.*z)*( 2. - 3.*x)*( 2. - 3.*y);
   divshape(33) = ( 4. - 8.*z)*( 2. - 3.*x)*(-1. + 3.*y);
   divshape(34) = ( 4. - 8.*z)*(-1. + 3.*x)*( 2. - 3.*y);
   divshape(35) = ( 4. - 8.*z)*(-1. + 3.*x)*(-1. + 3.*y);
}

const double RT1HexFiniteElement::nk[36][3] =
{
   {0, 0,-1}, {0, 0,-1}, {0, 0,-1}, {0, 0,-1},
   {0,-1, 0}, {0,-1, 0}, {0,-1, 0}, {0,-1, 0},
   {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
   {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
   {-1,0, 0}, {-1,0, 0}, {-1,0, 0}, {-1,0, 0},
   {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
   {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
   {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
   {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}
};

void RT1HexFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 36; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 36; j++)
      {
         double d = ( vshape(j,0)*nk[k][0] + vshape(j,1)*nk[k][1] +
                      vshape(j,2)*nk[k][2] );
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT0HexFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = ip.z = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[3];
   Vector xk (vk, 3);

   for (k = 0; k < 36; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1]; ip.z = vk[2];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2];
      vk[2] = Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2];
      for (j = 0; j < 36; j++)
         if (fabs (I(k,j) = (vshape(j,0)*vk[0]+vshape(j,1)*vk[1]+
                             vshape(j,2)*vk[2])) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT1HexFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[3];
   Vector xk (vk, 3);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 36; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) =
         vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2] ) +
         vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2] ) +
         vk[2] * ( Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2] );
   }
}

RT0TetFiniteElement::RT0TetFiniteElement()
   : VectorFiniteElement (3, Geometry::TETRAHEDRON, 4, 1)
{
   // not real nodes ...
   Nodes.IntPoint(0).x = 0.33333333333333333333;
   Nodes.IntPoint(0).y = 0.33333333333333333333;
   Nodes.IntPoint(0).z = 0.33333333333333333333;

   Nodes.IntPoint(1).x = 0.0;
   Nodes.IntPoint(1).y = 0.33333333333333333333;
   Nodes.IntPoint(1).z = 0.33333333333333333333;

   Nodes.IntPoint(2).x = 0.33333333333333333333;
   Nodes.IntPoint(2).y = 0.0;
   Nodes.IntPoint(2).z = 0.33333333333333333333;

   Nodes.IntPoint(3).x = 0.33333333333333333333;
   Nodes.IntPoint(3).y = 0.33333333333333333333;
   Nodes.IntPoint(3).z = 0.0;
}

void RT0TetFiniteElement::CalcVShape(const IntegrationPoint &ip,
                                     DenseMatrix &shape) const
{
   double x2 = 2.0*ip.x, y2 = 2.0*ip.y, z2 = 2.0*ip.z;

   shape(0,0) = x2;
   shape(0,1) = y2;
   shape(0,2) = z2;

   shape(1,0) = x2 - 2.0;
   shape(1,1) = y2;
   shape(1,2) = z2;

   shape(2,0) = x2;
   shape(2,1) = y2 - 2.0;
   shape(2,2) = z2;

   shape(3,0) = x2;
   shape(3,1) = y2;
   shape(3,2) = z2 - 2.0;
}

void RT0TetFiniteElement::CalcDivShape(const IntegrationPoint &ip,
                                       Vector &divshape) const
{
   divshape(0) = 6.0;
   divshape(1) = 6.0;
   divshape(2) = 6.0;
   divshape(3) = 6.0;
}

const double RT0TetFiniteElement::nk[4][3] =
{{.5,.5,.5}, {-.5,0,0}, {0,-.5,0}, {0,0,-.5}};

void RT0TetFiniteElement::GetLocalInterpolation (
   ElementTransformation &Trans, DenseMatrix &I) const
{
   int k, j;
#ifdef MFEM_USE_OPENMP
   DenseMatrix vshape(Dof, Dim);
   DenseMatrix Jinv(Dim);
#endif

#ifdef MFEM_DEBUG
   for (k = 0; k < 4; k++)
   {
      CalcVShape (Nodes.IntPoint(k), vshape);
      for (j = 0; j < 4; j++)
      {
         double d = ( vshape(j,0)*nk[k][0] + vshape(j,1)*nk[k][1] +
                      vshape(j,2)*nk[k][2] );
         if (j == k) d -= 1.0;
         if (fabs(d) > 1.0e-12)
         {
            cerr << "RT0TetFiniteElement::GetLocalInterpolation (...)\n"
               " k = " << k << ", j = " << j << ", d = " << d << endl;
            mfem_error();
         }
      }
   }
#endif

   IntegrationPoint ip;
   ip.x = ip.y = ip.z = 0.0;
   Trans.SetIntPoint (&ip);
   // Trans must be linear
   // set Jinv = |J| J^{-t} = adj(J)^t
   CalcAdjugateTranspose (Trans.Jacobian(), Jinv);
   double vk[3];
   Vector xk (vk, 3);

   for (k = 0; k < 4; k++)
   {
      Trans.Transform (Nodes.IntPoint (k), xk);
      ip.x = vk[0]; ip.y = vk[1]; ip.z = vk[2];
      CalcVShape (ip, vshape);
      //  vk = |J| J^{-t} nk
      vk[0] = Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2];
      vk[1] = Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2];
      vk[2] = Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2];
      for (j = 0; j < 4; j++)
         if (fabs (I(k,j) = (vshape(j,0)*vk[0]+vshape(j,1)*vk[1]+
                             vshape(j,2)*vk[2])) < 1.0e-12)
            I(k,j) = 0.0;
   }
}

void RT0TetFiniteElement::Project (
   VectorCoefficient &vc, ElementTransformation &Trans,
   Vector &dofs) const
{
   double vk[3];
   Vector xk (vk, 3);
#ifdef MFEM_USE_OPENMP
   DenseMatrix Jinv(Dim);
#endif

   for (int k = 0; k < 4; k++)
   {
      Trans.SetIntPoint (&Nodes.IntPoint (k));
      // set Jinv = |J| J^{-t} = adj(J)^t
      CalcAdjugateTranspose (Trans.Jacobian(), Jinv);

      vc.Eval (xk, Trans, Nodes.IntPoint (k));
      //  xk^t |J| J^{-t} nk
      dofs(k) =
         vk[0] * ( Jinv(0,0)*nk[k][0]+Jinv(0,1)*nk[k][1]+Jinv(0,2)*nk[k][2] ) +
         vk[1] * ( Jinv(1,0)*nk[k][0]+Jinv(1,1)*nk[k][1]+Jinv(1,2)*nk[k][2] ) +
         vk[2] * ( Jinv(2,0)*nk[k][0]+Jinv(2,1)*nk[k][1]+Jinv(2,2)*nk[k][2] );
   }
}

RotTriLinearHexFiniteElement::RotTriLinearHexFiniteElement()
   : NodalFiniteElement(3, Geometry::CUBE, 6, 2, FunctionSpace::Qk)
{
   Nodes.IntPoint(0).x = 0.5;
   Nodes.IntPoint(0).y = 0.5;
   Nodes.IntPoint(0).z = 0.0;

   Nodes.IntPoint(1).x = 0.5;
   Nodes.IntPoint(1).y = 0.0;
   Nodes.IntPoint(1).z = 0.5;

   Nodes.IntPoint(2).x = 1.0;
   Nodes.IntPoint(2).y = 0.5;
   Nodes.IntPoint(2).z = 0.5;

   Nodes.IntPoint(3).x = 0.5;
   Nodes.IntPoint(3).y = 1.0;
   Nodes.IntPoint(3).z = 0.5;

   Nodes.IntPoint(4).x = 0.0;
   Nodes.IntPoint(4).y = 0.5;
   Nodes.IntPoint(4).z = 0.5;

   Nodes.IntPoint(5).x = 0.5;
   Nodes.IntPoint(5).y = 0.5;
   Nodes.IntPoint(5).z = 1.0;
}

void RotTriLinearHexFiniteElement::CalcShape(const IntegrationPoint &ip,
                                             Vector &shape) const
{
   double x = 2. * ip.x - 1.;
   double y = 2. * ip.y - 1.;
   double z = 2. * ip.z - 1.;
   double f5 = x * x - y * y;
   double f6 = y * y - z * z;

   shape(0) = (1./6.) * (1. - 3. * z -      f5 - 2. * f6);
   shape(1) = (1./6.) * (1. - 3. * y -      f5 +      f6);
   shape(2) = (1./6.) * (1. + 3. * x + 2. * f5 +      f6);
   shape(3) = (1./6.) * (1. + 3. * y -      f5 +      f6);
   shape(4) = (1./6.) * (1. - 3. * x + 2. * f5 +      f6);
   shape(5) = (1./6.) * (1. + 3. * z -      f5 - 2. * f6);
}

void RotTriLinearHexFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                              DenseMatrix &dshape) const
{
   const double a = 2./3.;

   double xt = a * (1. - 2. * ip.x);
   double yt = a * (1. - 2. * ip.y);
   double zt = a * (1. - 2. * ip.z);

   dshape(0,0) = xt;
   dshape(0,1) = yt;
   dshape(0,2) = -1. - 2. * zt;

   dshape(1,0) = xt;
   dshape(1,1) = -1. - 2. * yt;
   dshape(1,2) = zt;

   dshape(2,0) = 1. - 2. * xt;
   dshape(2,1) = yt;
   dshape(2,2) = zt;

   dshape(3,0) = xt;
   dshape(3,1) = 1. - 2. * yt;
   dshape(3,2) = zt;

   dshape(4,0) = -1. - 2. * xt;
   dshape(4,1) = yt;
   dshape(4,2) = zt;

   dshape(5,0) = xt;
   dshape(5,1) = yt;
   dshape(5,2) = 1. - 2. * zt;
}


Poly_1D::Basis::Basis(const int p, const double *nodes)
   : A(p + 1)
{
#ifdef MFEM_USE_OPENMP
   Vector a(p+1);
#else
   a.SetSize(p+1);
   b.SetSize(p+1);
#endif
   for (int i = 0; i <= p; i++)
   {
      CalcBasis(p, nodes[i], a);
      for (int j = 0; j <= p; j++)
         A(j, i) = a(j);
   }

   A.Invert();
   // dbg << "Poly_1D::Basis(" << p << ",...) : "; A.TestInversion();
}

void Poly_1D::Basis::Eval(const double x, Vector &u) const
{
#ifdef MFEM_USE_OPENMP
   Vector a(A.Size());
#endif
   const int p = A.Size() - 1;
   CalcBasis(p, x, a);
   A.Mult(a, u);
}

void Poly_1D::Basis::Eval(const double x, Vector &u, Vector &d) const
{
#ifdef MFEM_USE_OPENMP
   Vector a(A.Size()), b(A.Size());
#endif
   const int p = A.Size() - 1;
   CalcBasis(p, x, a, b);
   A.Mult(a, u);
   A.Mult(b, d);
}

void Poly_1D::UniformPoints(const int p, double *x)
{
   if (p == 0)
   {
      x[0] = 0.5;
   }
   else
   {
      for (int i = 0; i <= p; i++)
         x[i] = double(i)/p;
   }
}

void Poly_1D::GaussPoints(const int p, double *x)
{
   int m = (p+1)/2, odd_p = p%2;

   if (!odd_p)
      x[m] = 0.5;

   for (int i = 0; i < m; i++)
   {
      double z, y, p0, d0;
      z = cos(M_PI*(i + 0.75)/(p + 1.5));

      for (int k = 0; true; )
      {
         // compute p0, d0 -- P_{p+1}(z), P'_{p+1}(z) using
         // (n+1)*P_{n+1}(z) = (2*n+1)*z*P_n(z)-n*P_{n-1}(z)
         // P'_{n+1}(z) = (2*n+1)*P_n(z)+P'_{n-1}(z)
         {
            double p1, p2;
            p2 = 1.;
            p1 = z;
            d0 = 1 - odd_p;
            for (int n = 1; true; n++)
            {
               p0 = ((2*n+1)*z*p1 - n*p2)/(n + 1);
               if (n%2 == odd_p)
                  d0 += (2*n+1)*p1;
               if (n == p) break;
               p2 = p1;
               p1 = p0;
            }
            // d0 = (p + 1)*(z*p0 - p1)/(z*z - 1); // alternative formula
         }

         if (fabs(p0/d0) < 2e-16) break;

         if (++k == 5)
         {
            dbg << "Poly_1D::GaussPoints : No convergence!"
               " p = " << p << ", i = " << i << ", p0/d0 = " << p0/d0
                      << std::endl;
            break;
         }

         z = z - p0/d0;
      }

      // z = z - p0/d0; y = (1 - z)/2; // bad roundoff !!!
      y = ((1 - z) + p0/d0)/2;

      x[i]   = y;
      x[p-i] = 1 - y;
      // the weight is: 1./(4*y*(1 - y)*d0*d0)
   }
}

void Poly_1D::GaussLobattoPoints(const int p, double *x)
{
   if (p == 0)
   {
      x[0] = 0.5;
   }
   else
   {
      x[0] = 0.;
      x[p] = 1.;
      if (p == 1) return;

      // x[1],...,x[p-1] are the (shifted) roots of P'_p
      int m = (p - 1)/2, odd_p = p%2;

      if (!odd_p)
         x[m+1] = 0.5;
      for (int i = 0; i < m; )
      {
         double y, z, d0, s0;
         z = cos(M_PI*(i + 1)/p);

         for (int k = 0; true; )
         {
            // compute d0, s0 -- P'_p(z), P"_p(z)
            // (n+1)*P_{n+1}(z) = (2*n+1)*z*P_n(z)-n*P_{n-1}(z)
            // P'_{n+1}(z) = (2*n+1)* P_n(z)+P'_{n-1}(z)
            // P"_{n+1}(z) = (2*n+1)*P'_n(z)+P"_{n-1}(z)
            {
               double p0, p1, p2, d1;
               p2 = 1.;
               p1 = z;
               d0 = odd_p;
               d1 = 1 - odd_p;
               s0 = 0;
               for (int n = 1; true; n++)
               {
                  p0 = ((2*n+1)*z*p1 - n*p2)/(n + 1);
                  if (n%2 != odd_p)
                  {
                     d0 += (2*n+1)*p1;
                     s0 += (2*n+1)*d1;
                  }
                  else
                  {
                     d1 += (2*n+1)*p1;
                  }
                  if (n == p - 1) break;
                  p2 = p1;
                  p1 = p0;
               }
            }

            if (fabs(d0/s0) < 2e-16) break;

            if (++k == 6)
            {
               dbg << "Poly_1D::GaussLobattoPoints : No convergence!"
                  " p = " << p << ", i = " << i << ", d0/s0 = " << d0/s0
                         << std::endl;
               break;
            }

            z = z - d0/s0;
         }

         y = ((1 - z) + d0/s0)/2;

         x[++i] = y;
         x[p-i] = 1 - y;
      }
   }
}

void Poly_1D::ChebyshevPoints(const int p, double *x)
{
   for (int i = 0; i <= p; i++)
   {
      // x[i] = 0.5*(1. + cos(M_PI*(p - i + 0.5)/(p + 1)));
      double s = sin(M_PI_2*(i + 0.5)/(p + 1));
      x[i] = s*s;
   }
}

void Poly_1D::CalcMono(const int p, const double x, double *u)
{
   double xn;
   u[0] = xn = 1.;
   for (int n = 1; n <= p; n++)
      u[n] = (xn *= x);
}

void Poly_1D::CalcMono(const int p, const double x, double *u, double *d)
{
   double xn;
   u[0] = xn = 1.;
   d[0] = 0.;
   for (int n = 1; n <= p; n++)
   {
      d[n] = n * xn;
      u[n] = (xn *= x);
   }
}

void Poly_1D::CalcBernstein(const int p, const double x, double *u)
{
   // not the best way to evaluate; just for testing
   u[0] = 1.;
   for (int n = 1; n <= p; n++)
   {
      u[n] = x*u[n-1];
      for (int k = n - 1; k > 0; k--)
         u[k] = x*u[k-1] + (1. - x)*u[k];
      u[0] = (1. - x)*u[0];
   }
}

void Poly_1D::CalcBernstein(const int p, const double x, double *u, double *d)
{
   // not the best way to evaluate; just for testing
   u[0] = 1.;
   d[0] = 0.;
   for (int n = 1; n <= p; n++)
   {
      d[n] = u[n-1] + x*d[n-1];
      u[n] = x*u[n-1];
      for (int k = n - 1; k > 0; k--)
      {
         d[k] = u[k-1] + x*d[k-1] - u[k] + (1. - x)*d[k];
         u[k] = x*u[k-1] + (1. - x)*u[k];
      }
      d[0] = -u[0] + (1. - x)*d[0];
      u[0] = (1. - x)*u[0];
   }
}

void Poly_1D::CalcLegendre(const int p, const double x, double *u)
{
   // use the recursive definition for [-1,1]:
   // (n+1)*P_{n+1}(z) = (2*n+1)*z*P_n(z)-n*P_{n-1}(z)
   double z;
   u[0] = 1.;
   if (p == 0) return;
   u[1] = z = 2.*x - 1.;
   for (int n = 1; n < p; n++)
   {
      u[n+1] = ((2*n + 1)*z*u[n] - n*u[n-1])/(n + 1);
   }
}

void Poly_1D::CalcLegendre(const int p, const double x, double *u, double *d)
{
   // use the recursive definition for [-1,1]:
   // (n+1)*P_{n+1}(z) = (2*n+1)*z*P_n(z)-n*P_{n-1}(z)
   // for the derivative use, z in [-1,1]:
   // P'_{n+1}(z) = (2*n+1)*P_n(z)+P'_{n-1}(z)
   double z;
   u[0] = 1.;
   d[0] = 0.;
   if (p == 0) return;
   u[1] = z = 2.*x - 1.;
   d[1] = 2.;
   for (int n = 1; n < p; n++)
   {
      u[n+1] = ((2*n + 1)*z*u[n] - n*u[n-1])/(n + 1);
      d[n+1] = (4*n + 2)*u[n] + d[n-1];
   }
}

void Poly_1D::CalcChebyshev(const int p, const double x, double *u)
{
   // recursive definition, z in [-1,1]
   // T_0(z) = 1,  T_1(z) = z
   // T_{n+1}(z) = 2*z*T_n(z) - T_{n-1}(z)
   double z;
   u[0] = 1.;
   if (p == 0) return;
   u[1] = z = 2.*x - 1.;
   for (int n = 1; n < p; n++)
   {
      u[n+1] = 2*z*u[n] - u[n-1];
   }
}

void Poly_1D::CalcChebyshev(const int p, const double x, double *u, double *d)
{
   // recursive definition, z in [-1,1]
   // T_0(z) = 1,  T_1(z) = z
   // T_{n+1}(z) = 2*z*T_n(z) - T_{n-1}(z)
   // T'_n(z) = n*U_{n-1}(z)
   // U_0(z) = 1  U_1(z) = 2*z
   // U_{n+1}(z) = 2*z*U_n(z) - U_{n-1}(z)
   // U_n(z) = z*U_{n-1}(z) + T_n(z) = z*T'_n(z)/n + T_n(z)
   // T'_{n+1}(z) = (n + 1)*(z*T'_n(z)/n + T_n(z))
   double z;
   u[0] = 1.;
   d[0] = 0.;
   if (p == 0) return;
   u[1] = z = 2.*x - 1.;
   d[1] = 2.;
   for (int n = 1; n < p; n++)
   {
      u[n+1] = 2*z*u[n] - u[n-1];
      d[n+1] = (n + 1)*(z*d[n]/n + 2*u[n]);
   }
}

const double *Poly_1D::OpenPoints(const int p)
{
   double *op;

   if (open_pts.Size() <= p)
   {
      int i = open_pts.Size();
      open_pts.SetSize(p + 1);
      for ( ; i < p; i++)
         open_pts[i] = NULL;
      goto alloc_open;
   }
   if ((op = open_pts[p]) != NULL)
      return op;
alloc_open:
   open_pts[p] = op = new double[p + 1];
   GaussPoints(p, op);
   // ChebyshevPoints(p, op);
   return op;
}

const double *Poly_1D::ClosedPoints(const int p)
{
   double *cp;

   if (closed_pts.Size() <= p)
   {
      int i = closed_pts.Size();
      closed_pts.SetSize(p + 1);
      for ( ; i < p; i++)
         closed_pts[i] = NULL;
      goto alloc_closed;
   }
   if ((cp = closed_pts[p]) != NULL)
      return cp;
alloc_closed:
   closed_pts[p] = cp = new double[p + 1];
   GaussLobattoPoints(p, cp);
   // UniformPoints(p, cp);
   return cp;
}

Poly_1D::Basis &Poly_1D::OpenBasis(const int p)
{
   Basis *ob;

   if (open_basis.Size() <= p)
   {
      int i = open_basis.Size();
      open_basis.SetSize(p + 1);
      for ( ; i < p; i++)
         open_basis[i] = NULL;
      goto alloc_obasis;
   }
   if ((ob = open_basis[p]) != NULL)
      return *ob;
alloc_obasis:
   open_basis[p] = ob = new Basis(p, OpenPoints(p));
   return *ob;
}

Poly_1D::Basis &Poly_1D::ClosedBasis(const int p)
{
   Basis *cb;

   if (closed_basis.Size() <= p)
   {
      int i = closed_basis.Size();
      closed_basis.SetSize(p + 1);
      for ( ; i < p; i++)
         closed_basis[i] = NULL;
      goto alloc_cbasis;
   }
   if ((cb = closed_basis[p]) != NULL)
      return *cb;
alloc_cbasis:
   closed_basis[p] = cb = new Basis(p, ClosedPoints(p));
   return *cb;
}

Poly_1D::~Poly_1D()
{
   for (int i = 0; i < open_pts.Size(); i++)
      delete [] open_pts[i];
   for (int i = 0; i < closed_pts.Size(); i++)
      delete [] closed_pts[i];
   for (int i = 0; i < open_basis.Size(); i++)
      delete open_basis[i];
   for (int i = 0; i < closed_basis.Size(); i++)
      delete closed_basis[i];
}

Poly_1D poly1d;


H1_SegmentElement::H1_SegmentElement(const int p)
   : NodalFiniteElement(1, Geometry::SEGMENT, p + 1, p, FunctionSpace::Pk),
     basis1d(poly1d.ClosedBasis(p))
{
   const double *cp = poly1d.ClosedPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p+1);
   dshape_x.SetSize(p+1);
#endif

   Nodes.IntPoint(0).x = cp[0];
   Nodes.IntPoint(1).x = cp[p];
   for (int i = 1; i < p; i++)
      Nodes.IntPoint(i+1).x = cp[i];
}

void H1_SegmentElement::CalcShape(const IntegrationPoint &ip,
                                  Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1);
#endif

   basis1d.Eval(ip.x, shape_x);

   shape(0) = shape_x(0);
   shape(1) = shape_x(p);
   for (int i = 1; i < p; i++)
      shape(i+1) = shape_x(i);
}

void H1_SegmentElement::CalcDShape(const IntegrationPoint &ip,
                                   DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), dshape_x(p+1);
#endif

   basis1d.Eval(ip.x, shape_x, dshape_x);

   dshape(0,0) = dshape_x(0);
   dshape(1,0) = dshape_x(p);
   for (int i = 1; i < p; i++)
      dshape(i+1,0) = dshape_x(i);
}


H1_QuadrilateralElement::H1_QuadrilateralElement(const int p)
   : NodalFiniteElement(2, Geometry::SQUARE, (p + 1)*(p + 1), p,
                        FunctionSpace::Qk),
     basis1d(poly1d.ClosedBasis(p)), dof_map((p + 1)*(p + 1))
{
   const double *cp = poly1d.ClosedPoints(p);

   const int p1 = p + 1;

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p1);
   shape_y.SetSize(p1);
   dshape_x.SetSize(p1);
   dshape_y.SetSize(p1);
#endif

   // vertices
   dof_map[0 + 0*p1] = 0;
   dof_map[p + 0*p1] = 1;
   dof_map[p + p*p1] = 2;
   dof_map[0 + p*p1] = 3;

   // edges
   int o = 4;
   for (int i = 1; i < p; i++)
      dof_map[i + 0*p1] = o++;
   for (int i = 1; i < p; i++)
      dof_map[p + i*p1] = o++;
   for (int i = 1; i < p; i++)
      dof_map[(p-i) + p*p1] = o++;
   for (int i = 1; i < p; i++)
      dof_map[0 + (p-i)*p1] = o++;

   // interior
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[i + j*p1] = o++;

   o = 0;
   for (int j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
         Nodes.IntPoint(dof_map[o++]).Set2(cp[i], cp[j]);
}

void H1_QuadrilateralElement::CalcShape(const IntegrationPoint &ip,
                                        Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1);
#endif

   basis1d.Eval(ip.x, shape_x);
   basis1d.Eval(ip.y, shape_y);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
         shape(dof_map[o++]) = shape_x(i)*shape_y(j);
}

void H1_QuadrilateralElement::CalcDShape(const IntegrationPoint &ip,
                                         DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1), dshape_x(p+1), dshape_y(p+1);
#endif

   basis1d.Eval(ip.x, shape_x, dshape_x);
   basis1d.Eval(ip.y, shape_y, dshape_y);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
      {
         dshape(dof_map[o],0) = dshape_x(i)* shape_y(j);
         dshape(dof_map[o],1) =  shape_x(i)*dshape_y(j);  o++;
      }
}

H1_HexahedronElement::H1_HexahedronElement(const int p)
   : NodalFiniteElement(3, Geometry::CUBE, (p + 1)*(p + 1)*(p + 1), p,
                        FunctionSpace::Qk),
     basis1d(poly1d.ClosedBasis(p)), dof_map((p + 1)*(p + 1)*(p + 1))
{
   const double *cp = poly1d.ClosedPoints(p);

   const int p1 = p + 1;

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p1);
   shape_y.SetSize(p1);
   shape_z.SetSize(p1);
   dshape_x.SetSize(p1);
   dshape_y.SetSize(p1);
   dshape_z.SetSize(p1);
#endif

   // vertices
   dof_map[0 + (0 + 0*p1)*p1] = 0;
   dof_map[p + (0 + 0*p1)*p1] = 1;
   dof_map[p + (p + 0*p1)*p1] = 2;
   dof_map[0 + (p + 0*p1)*p1] = 3;
   dof_map[0 + (0 + p*p1)*p1] = 4;
   dof_map[p + (0 + p*p1)*p1] = 5;
   dof_map[p + (p + p*p1)*p1] = 6;
   dof_map[0 + (p + p*p1)*p1] = 7;

   // edges (see Hexahedron::edges in mesh/hexahedron.cpp)
   int o = 8;
   for (int i = 1; i < p; i++)
      dof_map[i + (0 + 0*p1)*p1] = o++;  // (0,1)
   for (int i = 1; i < p; i++)
      dof_map[p + (i + 0*p1)*p1] = o++;  // (1,2)
   for (int i = 1; i < p; i++)
      dof_map[i + (p + 0*p1)*p1] = o++;  // (3,2)
   for (int i = 1; i < p; i++)
      dof_map[0 + (i + 0*p1)*p1] = o++;  // (0,3)
   for (int i = 1; i < p; i++)
      dof_map[i + (0 + p*p1)*p1] = o++;  // (4,5)
   for (int i = 1; i < p; i++)
      dof_map[p + (i + p*p1)*p1] = o++;  // (5,6)
   for (int i = 1; i < p; i++)
      dof_map[i + (p + p*p1)*p1] = o++;  // (7,6)
   for (int i = 1; i < p; i++)
      dof_map[0 + (i + p*p1)*p1] = o++;  // (4,7)
   for (int i = 1; i < p; i++)
      dof_map[0 + (0 + i*p1)*p1] = o++;  // (0,4)
   for (int i = 1; i < p; i++)
      dof_map[p + (0 + i*p1)*p1] = o++;  // (1,5)
   for (int i = 1; i < p; i++)
      dof_map[p + (p + i*p1)*p1] = o++;  // (2,6)
   for (int i = 1; i < p; i++)
      dof_map[0 + (p + i*p1)*p1] = o++;  // (3,7)

   // faces (see Mesh::GenerateFaces in mesh/mesh.cpp)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[i + ((p-j) + 0*p1)*p1] = o++;  // (3,2,1,0)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[i + (0 + j*p1)*p1] = o++;  // (0,1,5,4)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[p + (i + j*p1)*p1] = o++;  // (1,2,6,5)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[(p-i) + (p + j*p1)*p1] = o++;  // (2,3,7,6)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[0 + ((p-i) + j*p1)*p1] = o++;  // (3,0,4,7)
   for (int j = 1; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[i + (j + p*p1)*p1] = o++;  // (4,5,6,7)

   // interior
   for (int k = 1; k < p; k++)
      for (int j = 1; j < p; j++)
         for (int i = 1; i < p; i++)
            dof_map[i + (j + k*p1)*p1] = o++;

   o = 0;
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            Nodes.IntPoint(dof_map[o++]).Set3(cp[i], cp[j], cp[k]);
}

void H1_HexahedronElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1), shape_z(p+1);
#endif

   basis1d.Eval(ip.x, shape_x);
   basis1d.Eval(ip.y, shape_y);
   basis1d.Eval(ip.z, shape_z);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            shape(dof_map[o++]) = shape_x(i)*shape_y(j)*shape_z(k);
}

void H1_HexahedronElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1),  shape_y(p+1),  shape_z(p+1);
   Vector dshape_x(p+1), dshape_y(p+1), dshape_z(p+1);
#endif

   basis1d.Eval(ip.x, shape_x, dshape_x);
   basis1d.Eval(ip.y, shape_y, dshape_y);
   basis1d.Eval(ip.z, shape_z, dshape_z);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            dshape(dof_map[o],0) = dshape_x(i)* shape_y(j)* shape_z(k);
            dshape(dof_map[o],1) =  shape_x(i)*dshape_y(j)* shape_z(k);
            dshape(dof_map[o],2) =  shape_x(i)* shape_y(j)*dshape_z(k);  o++;
         }
}


H1_TriangleElement::H1_TriangleElement(const int p)
   : NodalFiniteElement(2, Geometry::TRIANGLE, ((p + 1)*(p + 2))/2, p,
                        FunctionSpace::Pk), T(Dof)
{
   const double *cp = poly1d.ClosedPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof);
   du.SetSize(Dof, Dim);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1);
#endif

   // vertices
   Nodes.IntPoint(0).Set2(cp[0], cp[0]);
   Nodes.IntPoint(1).Set2(cp[p], cp[0]);
   Nodes.IntPoint(2).Set2(cp[0], cp[p]);

   // edges
   int o = 3;
   for (int i = 1; i < p; i++)
      Nodes.IntPoint(o++).Set2(cp[i], cp[0]);
   for (int i = 1; i < p; i++)
      Nodes.IntPoint(o++).Set2(cp[p-i], cp[i]);
   for (int i = 1; i < p; i++)
      Nodes.IntPoint(o++).Set2(cp[0], cp[p-i]);

   // interior
   for (int j = 1; j < p; j++)
      for (int i = 1; i + j < p; i++)
      {
         const double w = cp[i] + cp[j] + cp[p-i-j];
         Nodes.IntPoint(o++).Set2(cp[i]/w, cp[j]/w);
      }

   for (int k = 0; k < Dof; k++)
   {
      IntegrationPoint &ip = Nodes.IntPoint(k);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);

      o = 0;
      for (int j = 0; j <= p; j++)
         for (int i = 0; i + j <= p; i++)
            T(o++, k) = shape_x(i)*shape_y(j)*shape_l(p-i-j);
   }

   T.Invert();
   // dbg << "H1_TriangleElement(" << p << ") : "; T.TestInversion();
}

void H1_TriangleElement::CalcShape(const IntegrationPoint &ip,
                                   Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1), u(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
         u(o++) = shape_x(i)*shape_y(j)*shape_l(p-i-j);

   T.Mult(u, shape);
}

void H1_TriangleElement::CalcDShape(const IntegrationPoint &ip,
                                    DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector  shape_x(p + 1),  shape_y(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_l(p + 1);
   DenseMatrix du(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l, dshape_l);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         int k = p - i - j;
         du(o,0) = ((dshape_x(i)* shape_l(k)) -
                    ( shape_x(i)*dshape_l(k)))*shape_y(j);
         du(o,1) = ((dshape_y(j)* shape_l(k)) -
                    ( shape_y(j)*dshape_l(k)))*shape_x(i);
         o++;
      }

   Mult(T, du, dshape);
}


H1_TetrahedronElement::H1_TetrahedronElement(const int p)
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, ((p + 1)*(p + 2)*(p + 3))/6,
                        p, FunctionSpace::Pk), T(Dof)
{
   const double *cp = poly1d.ClosedPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_z.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_z.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof);
   du.SetSize(Dof, Dim);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
#endif

   // vertices
   Nodes.IntPoint(0).Set3(cp[0], cp[0], cp[0]);
   Nodes.IntPoint(1).Set3(cp[p], cp[0], cp[0]);
   Nodes.IntPoint(2).Set3(cp[0], cp[p], cp[0]);
   Nodes.IntPoint(3).Set3(cp[0], cp[0], cp[p]);

   // edges (see Tetrahedron::edges in mesh/tetrahedron.cpp)
   int o = 4;
   for (int i = 1; i < p; i++)  // (0,1)
      Nodes.IntPoint(o++).Set3(cp[i], cp[0], cp[0]);
   for (int i = 1; i < p; i++)  // (0,2)
      Nodes.IntPoint(o++).Set3(cp[0], cp[i], cp[0]);
   for (int i = 1; i < p; i++)  // (0,3)
      Nodes.IntPoint(o++).Set3(cp[0], cp[0], cp[i]);
   for (int i = 1; i < p; i++)  // (1,2)
      Nodes.IntPoint(o++).Set3(cp[p-i], cp[i], cp[0]);
   for (int i = 1; i < p; i++)  // (1,3)
      Nodes.IntPoint(o++).Set3(cp[p-i], cp[0], cp[i]);
   for (int i = 1; i < p; i++)  // (2,3)
      Nodes.IntPoint(o++).Set3(cp[0], cp[p-i], cp[i]);

   // faces (see Mesh::GenerateFaces in mesh/mesh.cpp)
   for (int j = 1; j < p; j++)
      for (int i = 1; i + j < p; i++)  // (1,2,3)
      {
         double w = cp[i] + cp[j] + cp[p-i-j];
         Nodes.IntPoint(o++).Set3(cp[p-i-j]/w, cp[i]/w, cp[j]/w);
      }
   for (int j = 1; j < p; j++)
      for (int i = 1; i + j < p; i++)  // (0,3,2)
      {
         double w = cp[i] + cp[j] + cp[p-i-j];
         Nodes.IntPoint(o++).Set3(cp[0], cp[j]/w, cp[i]/w);
      }
   for (int j = 1; j < p; j++)
      for (int i = 1; i + j < p; i++)  // (0,1,3)
      {
         double w = cp[i] + cp[j] + cp[p-i-j];
         Nodes.IntPoint(o++).Set3(cp[i]/w, cp[0], cp[j]/w);
      }
   for (int j = 1; j < p; j++)
      for (int i = 1; i + j < p; i++)  // (0,2,1)
      {
         double w = cp[i] + cp[j] + cp[p-i-j];
         Nodes.IntPoint(o++).Set3(cp[j]/w, cp[i]/w, cp[0]);
      }

   // interior
   for (int k = 1; k < p; k++)
      for (int j = 1; j + k < p; j++)
         for (int i = 1; i + j + k < p; i++)
         {
            double w = cp[i] + cp[j] + cp[k] + cp[p-i-j-k];
            Nodes.IntPoint(o++).Set3(cp[i]/w, cp[j]/w, cp[k]/w);
         }

   for (int m = 0; m < Dof; m++)
   {
      IntegrationPoint &ip = Nodes.IntPoint(m);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, ip.z, shape_z);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);

      o = 0;
      for (int k = 0; k <= p; k++)
         for (int j = 0; j + k <= p; j++)
            for (int i = 0; i + j + k <= p; i++)
               T(o++, m) = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);
   }

   T.Invert();
   // dbg << "H1_TetrahedronElement(" << p << ") : "; T.TestInversion();
}

void H1_TetrahedronElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
   Vector u(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, ip.z, shape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
            u(o++) = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);

   T.Mult(u, shape);
}

void H1_TetrahedronElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector  shape_x(p + 1),  shape_y(p + 1),  shape_z(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_z(p + 1), dshape_l(p + 1);
   DenseMatrix du(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, ip.z, shape_z, dshape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l, dshape_l);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
         {
            int l = p - i - j - k;
            du(o,0) = ((dshape_x(i)* shape_l(l)) -
                       ( shape_x(i)*dshape_l(l)))*shape_y(j)*shape_z(k);
            du(o,1) = ((dshape_y(j)* shape_l(l)) -
                       ( shape_y(j)*dshape_l(l)))*shape_x(i)*shape_z(k);
            du(o,2) = ((dshape_z(k)* shape_l(l)) -
                       ( shape_z(k)*dshape_l(l)))*shape_x(i)*shape_y(j);
            o++;
         }

   Mult(T, du, dshape);
}


L2_SegmentElement::L2_SegmentElement(const int p)
   : NodalFiniteElement(1, Geometry::SEGMENT, p + 1, p, FunctionSpace::Pk),
     basis1d(poly1d.OpenBasis(p))
{
   const double *op = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   dshape_x.SetDataAndSize(NULL, p + 1);
#endif

   for (int i = 0; i <= p; i++)
      Nodes.IntPoint(i).x = op[i];
}

void L2_SegmentElement::CalcShape(const IntegrationPoint &ip,
                                  Vector &shape) const
{
   basis1d.Eval(ip.x, shape);
}

void L2_SegmentElement::CalcDShape(const IntegrationPoint &ip,
                                   DenseMatrix &dshape) const
{
#ifdef MFEM_USE_OPENMP
   Vector shape_x(Dof), dshape_x(dshape.Data(), Dof);
#else
   dshape_x.SetData(dshape.Data());
#endif
   basis1d.Eval(ip.x, shape_x, dshape_x);
}

void L2_SegmentElement::ProjectDelta(int vertex,
                                     Vector &dofs) const
{
   const int p = Order;
   const double *op = poly1d.OpenPoints(p);
   switch (vertex)
   {
   case 0:
      for (int i = 0; i <= p; i++)
         dofs(i) = poly1d.CalcDelta(p,(1.0 - op[i]));
      break;

   case 1:
      for (int i = 0; i <= p; i++)
         dofs(i) = poly1d.CalcDelta(p,op[i]);
      break;
   }
}


L2_QuadrilateralElement::L2_QuadrilateralElement(const int p)
   : NodalFiniteElement(2, Geometry::SQUARE, (p + 1)*(p + 1), p,
                        FunctionSpace::Qk), basis1d(poly1d.OpenBasis(p))
{
   const double *op = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
#endif

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
         Nodes.IntPoint(o++).Set2(op[i], op[j]);
}

void L2_QuadrilateralElement::CalcShape(const IntegrationPoint &ip,
                                        Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1);
#endif

   basis1d.Eval(ip.x, shape_x);
   basis1d.Eval(ip.y, shape_y);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
         shape(o++) = shape_x(i)*shape_y(j);
}

void L2_QuadrilateralElement::CalcDShape(const IntegrationPoint &ip,
                                         DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1), dshape_x(p+1), dshape_y(p+1);
#endif

   basis1d.Eval(ip.x, shape_x, dshape_x);
   basis1d.Eval(ip.y, shape_y, dshape_y);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i <= p; i++)
      {
         dshape(o,0) = dshape_x(i)* shape_y(j);
         dshape(o,1) =  shape_x(i)*dshape_y(j);  o++;
      }
}

void L2_QuadrilateralElement::ProjectDelta(int vertex,
                                           Vector &dofs) const
{
   const int p = Order;
   const double *op = poly1d.OpenPoints(p);

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1);
#endif

   for (int i = 0; i <= p; i++)
   {
      shape_x(i) = poly1d.CalcDelta(p,(1.0 - op[i]));
      shape_y(i) = poly1d.CalcDelta(p,op[i]);
   }

   switch (vertex)
   {
   case 0:
      for (int o = 0, j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dofs[o++] = shape_x(i)*shape_x(j);
      break;
   case 1:
      for (int o = 0, j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dofs[o++] = shape_y(i)*shape_x(j);
      break;
   case 2:
      for (int o = 0, j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dofs[o++] = shape_y(i)*shape_y(j);
      break;
   case 3:
      for (int o = 0, j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dofs[o++] = shape_x(i)*shape_y(j);
      break;
   }
}


L2_HexahedronElement::L2_HexahedronElement(const int p)
   : NodalFiniteElement(3, Geometry::CUBE, (p + 1)*(p + 1)*(p + 1), p,
                        FunctionSpace::Qk), basis1d(poly1d.OpenBasis(p))
{
   const double *op = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_z.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_z.SetSize(p + 1);
#endif

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            Nodes.IntPoint(o++).Set3(op[i], op[j], op[k]);
}

void L2_HexahedronElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1), shape_z(p+1);
#endif

   basis1d.Eval(ip.x, shape_x);
   basis1d.Eval(ip.y, shape_y);
   basis1d.Eval(ip.z, shape_z);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            shape(o++) = shape_x(i)*shape_y(j)*shape_z(k);
}

void L2_HexahedronElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1),  shape_y(p+1),  shape_z(p+1);
   Vector dshape_x(p+1), dshape_y(p+1), dshape_z(p+1);
#endif

   basis1d.Eval(ip.x, shape_x, dshape_x);
   basis1d.Eval(ip.y, shape_y, dshape_y);
   basis1d.Eval(ip.z, shape_z, dshape_z);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            dshape(o,0) = dshape_x(i)* shape_y(j)* shape_z(k);
            dshape(o,1) =  shape_x(i)*dshape_y(j)* shape_z(k);
            dshape(o,2) =  shape_x(i)* shape_y(j)*dshape_z(k);  o++;
         }
}

void L2_HexahedronElement::ProjectDelta(int vertex,
                                        Vector &dofs) const
{
   const int p = Order;
   const double *op = poly1d.OpenPoints(p);

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p+1), shape_y(p+1);
#endif

   for (int i = 0; i <= p; i++)
   {
      shape_x(i) = poly1d.CalcDelta(p,(1.0 - op[i]));
      shape_y(i) = poly1d.CalcDelta(p,op[i]);
   }

   switch (vertex)
   {
   case 0:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_x(i)*shape_x(j)*shape_x(k);
      break;
   case 1:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_y(i)*shape_x(j)*shape_x(k);
      break;
   case 2:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_y(i)*shape_y(j)*shape_x(k);
      break;
   case 3:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_x(i)*shape_y(j)*shape_x(k);
      break;
   case 4:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_x(i)*shape_x(j)*shape_y(k);
      break;
   case 5:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_y(i)*shape_x(j)*shape_y(k);
      break;
   case 6:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_y(i)*shape_y(j)*shape_y(k);
      break;
   case 7:
      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j <= p; j++)
            for (int i = 0; i <= p; i++)
               dofs[o++] = shape_x(i)*shape_y(j)*shape_y(k);
      break;
   }
}


L2_TriangleElement::L2_TriangleElement(const int p)
   : NodalFiniteElement(2, Geometry::TRIANGLE, ((p + 1)*(p + 2))/2, p,
                        FunctionSpace::Pk), T(Dof)
{
   const double *op = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof);
   du.SetSize(Dof, Dim);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1);
#endif

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         double w = op[i] + op[j] + op[p-i-j];
         Nodes.IntPoint(o++).Set2(op[i]/w, op[j]/w);
      }

   for (int k = 0; k < Dof; k++)
   {
      IntegrationPoint &ip = Nodes.IntPoint(k);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);

      for (int o = 0, j = 0; j <= p; j++)
         for (int i = 0; i + j <= p; i++)
            T(o++, k) = shape_x(i)*shape_y(j)*shape_l(p-i-j);
   }

   T.Invert();
   // dbg << "L2_TriangleElement(" << p << ") : "; T.TestInversion();
}

void L2_TriangleElement::CalcShape(const IntegrationPoint &ip,
                                   Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1), u(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
         u(o++) = shape_x(i)*shape_y(j)*shape_l(p-i-j);

   T.Mult(u, shape);
}

void L2_TriangleElement::CalcDShape(const IntegrationPoint &ip,
                                    DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector  shape_x(p + 1),  shape_y(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_l(p + 1);
   DenseMatrix du(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l, dshape_l);

   for (int o = 0, j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         int k = p - i - j;
         du(o,0) = ((dshape_x(i)* shape_l(k)) -
                    ( shape_x(i)*dshape_l(k)))*shape_y(j);
         du(o,1) = ((dshape_y(j)* shape_l(k)) -
                    ( shape_y(j)*dshape_l(k)))*shape_x(i);
         o++;
      }

   Mult(T, du, dshape);
}


L2_TetrahedronElement::L2_TetrahedronElement(const int p)
   : NodalFiniteElement(3, Geometry::TETRAHEDRON, ((p + 1)*(p + 2)*(p + 3))/6,
                        p, FunctionSpace::Pk), T(Dof)
{
   const double *op = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_z.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_z.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof);
   du.SetSize(Dof, Dim);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
#endif

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
         {
            double w = op[i] + op[j] + op[k] + op[p-i-j-k];
            Nodes.IntPoint(o++).Set3(op[i]/w, op[j]/w, op[k]/w);
         }

   for (int m = 0; m < Dof; m++)
   {
      IntegrationPoint &ip = Nodes.IntPoint(m);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, ip.z, shape_z);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);

      for (int o = 0, k = 0; k <= p; k++)
         for (int j = 0; j + k <= p; j++)
            for (int i = 0; i + j + k <= p; i++)
               T(o++, m) = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);
   }

   T.Invert();
   // dbg << "L2_TetrahedronElement(" << p << ") : "; T.TestInversion();
}

void L2_TetrahedronElement::CalcShape(const IntegrationPoint &ip,
                                      Vector &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
   Vector u(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, ip.z, shape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
            u(o++) = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);

   T.Mult(u, shape);
}

void L2_TetrahedronElement::CalcDShape(const IntegrationPoint &ip,
                                       DenseMatrix &dshape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector  shape_x(p + 1),  shape_y(p + 1),  shape_z(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_z(p + 1), dshape_l(p + 1);
   DenseMatrix du(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, ip.z, shape_z, dshape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l, dshape_l);

   for (int o = 0, k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
         {
            int l = p - i - j - k;
            du(o,0) = ((dshape_x(i)* shape_l(l)) -
                       ( shape_x(i)*dshape_l(l)))*shape_y(j)*shape_z(k);
            du(o,1) = ((dshape_y(j)* shape_l(l)) -
                       ( shape_y(j)*dshape_l(l)))*shape_x(i)*shape_z(k);
            du(o,2) = ((dshape_z(k)* shape_l(l)) -
                       ( shape_z(k)*dshape_l(l)))*shape_x(i)*shape_y(j);
            o++;
         }

   Mult(T, du, dshape);
}


const double RT_QuadrilateralElement::nk[8] =
{ 0., -1.,  1., 0.,  0., 1.,  -1., 0. };

RT_QuadrilateralElement::RT_QuadrilateralElement(const int p)
   : VectorFiniteElement(2, Geometry::SQUARE, 2*(p + 1)*(p + 2), p + 1,
                         FunctionSpace::Qk),
     cbasis1d(poly1d.ClosedBasis(p + 1)), obasis1d(poly1d.OpenBasis(p)),
     dof_map(Dof), dof2nk(Dof)
{
   const double *cp = poly1d.ClosedPoints(p + 1);
   const double *op = poly1d.OpenPoints(p);
   const int dof2 = Dof/2;

#ifndef MFEM_USE_OPENMP
   shape_cx.SetSize(p + 2);
   shape_ox.SetSize(p + 1);
   shape_cy.SetSize(p + 2);
   shape_oy.SetSize(p + 1);
   dshape_cx.SetSize(p + 2);
   dshape_cy.SetSize(p + 2);
#endif

   // edges
   int o = 0;
   for (int i = 0; i <= p; i++)  // (0,1)
      dof_map[1*dof2 + i + 0*(p + 1)] = o++;
   for (int i = 0; i <= p; i++)  // (1,2)
      dof_map[0*dof2 + (p + 1) + i*(p + 2)] = o++;
   for (int i = 0; i <= p; i++)  // (2,3)
      dof_map[1*dof2 + (p - i) + (p + 1)*(p + 1)] = o++;
   for (int i = 0; i <= p; i++)  // (3,0)
      dof_map[0*dof2 + 0 + (p - i)*(p + 2)] = o++;

   // interior
   for (int j = 0; j <= p; j++)  // x-components
      for (int i = 1; i <= p; i++)
         dof_map[0*dof2 + i + j*(p + 2)] = o++;
   for (int j = 1; j <= p; j++)  // y-components
      for (int i = 0; i <= p; i++)
         dof_map[1*dof2 + i + j*(p + 1)] = o++;

   // dof orientations
   // x-components
   for (int j = 0; j <= p; j++)
      for (int i = 0; i <= p/2; i++)
      {
         int idx = 0*dof2 + i + j*(p + 2);
         dof_map[idx] = -1 - dof_map[idx];
      }
   if (p%2 == 1)
      for (int j = p/2 + 1; j <= p; j++)
      {
         int idx = 0*dof2 + (p/2 + 1) + j*(p + 2);
         dof_map[idx] = -1 - dof_map[idx];
      }
   // y-components
   for (int j = 0; j <= p/2; j++)
      for (int i = 0; i <= p; i++)
      {
         int idx = 1*dof2 + i + j*(p + 1);
         dof_map[idx] = -1 - dof_map[idx];
      }
   if (p%2 == 1)
      for (int i = p/2 + 1; i <= p; i++)
      {
         int idx = 1*dof2 + i + (p/2 + 1)*(p + 1);
         dof_map[idx] = -1 - dof_map[idx];
      }

   o = 0;
   for (int j = 0; j <= p; j++)
      for (int i = 0; i <= p + 1; i++)
      {
         int idx;
         if ((idx = dof_map[o++]) < 0)
         {
            idx = -1 - idx;
            dof2nk[idx] = 3;
         }
         else
         {
            dof2nk[idx] = 1;
         }
         Nodes.IntPoint(idx).Set2(cp[i], op[j]);
      }
   for (int j = 0; j <= p + 1; j++)
      for (int i = 0; i <= p; i++)
      {
         int idx;
         if ((idx = dof_map[o++]) < 0)
         {
            idx = -1 - idx;
            dof2nk[idx] = 0;
         }
         else
         {
            dof2nk[idx] = 2;
         }
         Nodes.IntPoint(idx).Set2(op[i], cp[j]);
      }
}

void RT_QuadrilateralElement::CalcVShape(const IntegrationPoint &ip,
                                         DenseMatrix &shape) const
{
   const int pp1 = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(pp1 + 1), shape_ox(pp1), shape_cy(pp1 + 1), shape_oy(pp1);
#endif

   cbasis1d.Eval(ip.x, shape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy);
   obasis1d.Eval(ip.y, shape_oy);

   int o = 0;
   for (int j = 0; j < pp1; j++)
      for (int i = 0; i <= pp1; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         shape(idx,0) = s*shape_cx(i)*shape_oy(j);
         shape(idx,1) = 0.;
      }
   for (int j = 0; j <= pp1; j++)
      for (int i = 0; i < pp1; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         shape(idx,0) = 0.;
         shape(idx,1) = s*shape_ox(i)*shape_cy(j);
      }
}

void RT_QuadrilateralElement::CalcDivShape(const IntegrationPoint &ip,
                                           Vector &divshape) const
{
   const int pp1 = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(pp1 + 1), shape_ox(pp1), shape_cy(pp1 + 1), shape_oy(pp1);
   Vector dshape_cx(pp1 + 1), dshape_cy(pp1 + 1);
#endif

   cbasis1d.Eval(ip.x, shape_cx, dshape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy, dshape_cy);
   obasis1d.Eval(ip.y, shape_oy);

   int o = 0;
   for (int j = 0; j < pp1; j++)
      for (int i = 0; i <= pp1; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         divshape(idx) = s*dshape_cx(i)*shape_oy(j);
      }
   for (int j = 0; j <= pp1; j++)
      for (int i = 0; i < pp1; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         divshape(idx) = s*shape_ox(i)*dshape_cy(j);
      }
}


const double RT_HexahedronElement::nk[18] =
{ 0.,0.,-1.,  0.,-1.,0.,  1.,0.,0.,  0.,1.,0.,  -1.,0.,0.,  0.,0.,1. };

RT_HexahedronElement::RT_HexahedronElement(const int p)
   : VectorFiniteElement(3, Geometry::CUBE, 3*(p + 1)*(p + 1)*(p + 2), p + 1,
                         FunctionSpace::Qk),
     cbasis1d(poly1d.ClosedBasis(p + 1)), obasis1d(poly1d.OpenBasis(p)),
     dof_map(Dof), dof2nk(Dof)
{
   const double *cp = poly1d.ClosedPoints(p + 1);
   const double *op = poly1d.OpenPoints(p);
   const int dof3 = Dof/3;

#ifndef MFEM_USE_OPENMP
   shape_cx.SetSize(p + 2);
   shape_ox.SetSize(p + 1);
   shape_cy.SetSize(p + 2);
   shape_oy.SetSize(p + 1);
   shape_cz.SetSize(p + 2);
   shape_oz.SetSize(p + 1);
   dshape_cx.SetSize(p + 2);
   dshape_cy.SetSize(p + 2);
   dshape_cz.SetSize(p + 2);
#endif

   // faces
   int o = 0;
   for (int j = 0; j <= p; j++)  // (3,2,1,0) -- bottom
      for (int i = 0; i <= p; i++)
         dof_map[2*dof3 + i + ((p - j) + 0*(p + 1))*(p + 1)] = o++;
   for (int j = 0; j <= p; j++)  // (0,1,5,4) -- front
      for (int i = 0; i <= p; i++)
         dof_map[1*dof3 + i + (0 + j*(p + 2))*(p + 1)] = o++;
   for (int j = 0; j <= p; j++)  // (1,2,6,5) -- right
      for (int i = 0; i <= p; i++)
         dof_map[0*dof3 + (p + 1) + (i + j*(p + 1))*(p + 2)] = o++;
   for (int j = 0; j <= p; j++)  // (2,3,7,6) -- back
      for (int i = 0; i <= p; i++)
         dof_map[1*dof3 + (p - i) + ((p + 1) + j*(p + 2))*(p + 1)] = o++;
   for (int j = 0; j <= p; j++)  // (3,0,4,7) -- left
      for (int i = 0; i <= p; i++)
         dof_map[0*dof3 + 0 + ((p - i) + j*(p + 1))*(p + 2)] = o++;
   for (int j = 0; j <= p; j++)  // (4,5,6,7) -- top
      for (int i = 0; i <= p; i++)
         dof_map[2*dof3 + i + (j + (p + 1)*(p + 1))*(p + 1)] = o++;

   // interior
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 1; i <= p; i++)
            dof_map[0*dof3 + i + (j + k*(p + 1))*(p + 2)] = o++;
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 1; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dof_map[1*dof3 + i + (j + k*(p + 2))*(p + 1)] = o++;
   // z-components
   for (int k = 1; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
            dof_map[2*dof3 + i + (j + k*(p + 1))*(p + 1)] = o++;

   // dof orientations
   // for odd p, do not change the orientations in the mid-planes
   // {i = p/2 + 1}, {j = p/2 + 1}, {k = p/2 + 1} in the x, y, z-components
   // respectively.
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p/2; i++)
         {
            int idx = 0*dof3 + i + (j + k*(p + 1))*(p + 2);
            dof_map[idx] = -1 - dof_map[idx];
         }
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p/2; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx = 1*dof3 + i + (j + k*(p + 2))*(p + 1);
            dof_map[idx] = -1 - dof_map[idx];
         }
   // z-components
   for (int k = 0; k <= p/2; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx = 2*dof3 + i + (j + k*(p + 1))*(p + 1);
            dof_map[idx] = -1 - dof_map[idx];
         }

   o = 0;
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p + 1; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
            {
               idx = -1 - idx;
               dof2nk[idx] = 4;
            }
            else
            {
               dof2nk[idx] = 2;
            }
            Nodes.IntPoint(idx).Set3(cp[i], op[j], op[k]);
         }
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p + 1; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
            {
               idx = -1 - idx;
               dof2nk[idx] = 1;
            }
            else
            {
               dof2nk[idx] = 3;
            }
            Nodes.IntPoint(idx).Set3(op[i], cp[j], op[k]);
         }
   // z-components
   for (int k = 0; k <= p + 1; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
            {
               idx = -1 - idx;
               dof2nk[idx] = 0;
            }
            else
            {
               dof2nk[idx] = 5;
            }
            Nodes.IntPoint(idx).Set3(op[i], op[j], cp[k]);
         }
}

void RT_HexahedronElement::CalcVShape(const IntegrationPoint &ip,
                                      DenseMatrix &shape) const
{
   const int pp1 = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(pp1 + 1), shape_ox(pp1), shape_cy(pp1 + 1), shape_oy(pp1);
   Vector shape_cz(pp1 + 1), shape_oz(pp1);
#endif

   cbasis1d.Eval(ip.x, shape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy);
   obasis1d.Eval(ip.y, shape_oy);
   cbasis1d.Eval(ip.z, shape_cz);
   obasis1d.Eval(ip.z, shape_oz);

   int o = 0;
   // x-components
   for (int k = 0; k < pp1; k++)
      for (int j = 0; j < pp1; j++)
         for (int i = 0; i <= pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = s*shape_cx(i)*shape_oy(j)*shape_oz(k);
            shape(idx,1) = 0.;
            shape(idx,2) = 0.;
         }
   // y-components
   for (int k = 0; k < pp1; k++)
      for (int j = 0; j <= pp1; j++)
         for (int i = 0; i < pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = 0.;
            shape(idx,1) = s*shape_ox(i)*shape_cy(j)*shape_oz(k);
            shape(idx,2) = 0.;
         }
   // z-components
   for (int k = 0; k <= pp1; k++)
      for (int j = 0; j < pp1; j++)
         for (int i = 0; i < pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = 0.;
            shape(idx,1) = 0.;
            shape(idx,2) = s*shape_ox(i)*shape_oy(j)*shape_cz(k);
         }
}

void RT_HexahedronElement::CalcDivShape(const IntegrationPoint &ip,
                                        Vector &divshape) const
{
   const int pp1 = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(pp1 + 1), shape_ox(pp1), shape_cy(pp1 + 1), shape_oy(pp1);
   Vector shape_cz(pp1 + 1), shape_oz(pp1);
   Vector dshape_cx(pp1 + 1), dshape_cy(pp1 + 1), dshape_cz(pp1 + 1);
#endif

   cbasis1d.Eval(ip.x, shape_cx, dshape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy, dshape_cy);
   obasis1d.Eval(ip.y, shape_oy);
   cbasis1d.Eval(ip.z, shape_cz, dshape_cz);
   obasis1d.Eval(ip.z, shape_oz);

   int o = 0;
   // x-components
   for (int k = 0; k < pp1; k++)
      for (int j = 0; j < pp1; j++)
         for (int i = 0; i <= pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            divshape(idx) = s*dshape_cx(i)*shape_oy(j)*shape_oz(k);
         }
   // y-components
   for (int k = 0; k < pp1; k++)
      for (int j = 0; j <= pp1; j++)
         for (int i = 0; i < pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            divshape(idx) = s*shape_ox(i)*dshape_cy(j)*shape_oz(k);
         }
   // z-components
   for (int k = 0; k <= pp1; k++)
      for (int j = 0; j < pp1; j++)
         for (int i = 0; i < pp1; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            divshape(idx) = s*shape_ox(i)*shape_oy(j)*dshape_cz(k);
         }
}


const double RT_TriangleElement::nk[6] =
{ 0., -1., 1., 1., -1., 0. };

const double RT_TriangleElement::c = 1./3.;

RT_TriangleElement::RT_TriangleElement(const int p)
   : VectorFiniteElement(2, Geometry::TRIANGLE, (p + 1)*(p + 3), p + 1,
                         FunctionSpace::Pk), dof2nk(Dof), T(Dof)
{
   const double *iop = (p > 0) ? poly1d.OpenPoints(p - 1) : NULL;
   const double *bop = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof, Dim);
   divu.SetSize(Dof);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1);
#endif

   // edges
   int o = 0;
   for (int i = 0; i <= p; i++)  // (0,1)
   {
      Nodes.IntPoint(o).Set2(bop[i], 0.);
      dof2nk[o++] = 0;
   }
   for (int i = 0; i <= p; i++)  // (1,2)
   {
      Nodes.IntPoint(o).Set2(bop[p-i], bop[i]);
      dof2nk[o++] = 1;
   }
   for (int i = 0; i <= p; i++)  // (2,0)
   {
      Nodes.IntPoint(o).Set2(0., bop[p-i]);
      dof2nk[o++] = 2;
   }

   // interior
   for (int j = 0; j < p; j++)
      for (int i = 0; i + j < p; i++)
      {
         double w = iop[i] + iop[j] + iop[p-1-i-j];
         Nodes.IntPoint(o).Set2(iop[i]/w, iop[j]/w);
         dof2nk[o++] = 0;
         Nodes.IntPoint(o).Set2(iop[i]/w, iop[j]/w);
         dof2nk[o++] = 2;
      }

   for (int k = 0; k < Dof; k++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(k);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);
      const double *n_k = nk + 2*dof2nk[k];

      o = 0;
      for (int j = 0; j <= p; j++)
         for (int i = 0; i + j <= p; i++)
         {
            double s = shape_x(i)*shape_y(j)*shape_l(p-i-j);
            T(o++, k) = s*n_k[0];
            T(o++, k) = s*n_k[1];
         }
      for (int i = 0; i <= p; i++)
      {
         double s = shape_x(i)*shape_y(p-i);
         T(o++, k) = s*((ip.x - c)*n_k[0] + (ip.y - c)*n_k[1]);
      }
   }

   T.Invert();
   // dbg << "RT_TriangleElement(" << p << ") : "; T.TestInversion();
}

void RT_TriangleElement::CalcVShape(const IntegrationPoint &ip,
                                    DenseMatrix &shape) const
{
   const int p = Order - 1;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_l(p + 1);
   DenseMatrix u(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l);

   int o = 0;
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         double s = shape_x(i)*shape_y(j)*shape_l(p-i-j);
         u(o,0) = s;  u(o,1) = 0;  o++;
         u(o,0) = 0;  u(o,1) = s;  o++;
      }
   for (int i = 0; i <= p; i++)
   {
      double s = shape_x(i)*shape_y(p-i);
      u(o,0) = (ip.x - c)*s;  u(o,1) = (ip.y - c)*s;  o++;
   }

   Mult(T, u, shape);
}

void RT_TriangleElement::CalcDivShape(const IntegrationPoint &ip,
                                      Vector &divshape) const
{
   const int p = Order - 1;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1),  shape_y(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_l(p + 1);
   Vector divu(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y, shape_l, dshape_l);

   int o = 0;
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         int k = p - i - j;
         divu(o++) = (dshape_x(i)*shape_l(k) -
                      shape_x(i)*dshape_l(k))*shape_y(j);
         divu(o++) = (dshape_y(j)*shape_l(k) -
                      shape_y(j)*dshape_l(k))*shape_x(i);
      }
   for (int i = 0; i <= p; i++)
   {
      int j = p - i;
      divu(o++) = ((shape_x(i) + (ip.x - c)*dshape_x(i))*shape_y(j) +
                   (shape_y(j) + (ip.y - c)*dshape_y(j))*shape_x(i));
   }

   T.Mult(divu, divshape);
}


const double RT_TetrahedronElement::nk[12] =
{ 1,1,1,  -1,0,0,  0,-1,0,  0,0,-1 };
// { .5,.5,.5, -.5,0,0, 0,-.5,0, 0,0,-.5}; // n_F |F|

const double RT_TetrahedronElement::c = 1./4.;

RT_TetrahedronElement::RT_TetrahedronElement(const int p)
   : VectorFiniteElement(3, Geometry::TETRAHEDRON, (p + 1)*(p + 2)*(p + 4)/2,
                         p + 1, FunctionSpace::Pk), dof2nk(Dof), T(Dof)
{
   const double *iop = (p > 0) ? poly1d.OpenPoints(p - 1) : NULL;
   const double *bop = poly1d.OpenPoints(p);

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p + 1);
   shape_y.SetSize(p + 1);
   shape_z.SetSize(p + 1);
   shape_l.SetSize(p + 1);
   dshape_x.SetSize(p + 1);
   dshape_y.SetSize(p + 1);
   dshape_z.SetSize(p + 1);
   dshape_l.SetSize(p + 1);
   u.SetSize(Dof, Dim);
   divu.SetSize(Dof);
#else
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
#endif

   int o = 0;
   // faces (see Mesh::GenerateFaces in mesh/mesh.cpp,
   //        the constructor of H1_TetrahedronElement)
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)  // (1,2,3)
      {
         double w = bop[i] + bop[j] + bop[p-i-j];
         Nodes.IntPoint(o).Set3(bop[p-i-j]/w, bop[i]/w, bop[j]/w);
         dof2nk[o++] = 0;
      }
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)  // (0,3,2)
      {
         double w = bop[i] + bop[j] + bop[p-i-j];
         Nodes.IntPoint(o).Set3(0., bop[j]/w, bop[i]/w);
         dof2nk[o++] = 1;
      }
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)  // (0,1,3)
      {
         double w = bop[i] + bop[j] + bop[p-i-j];
         Nodes.IntPoint(o).Set3(bop[i]/w, 0., bop[j]/w);
         dof2nk[o++] = 2;
      }
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)  // (0,2,1)
      {
         double w = bop[i] + bop[j] + bop[p-i-j];
         Nodes.IntPoint(o).Set3(bop[j]/w, bop[i]/w, 0.);
         dof2nk[o++] = 3;
      }

   // interior
   for (int k = 0; k < p; k++)
      for (int j = 0; j + k < p; j++)
         for (int i = 0; i + j + k < p; i++)
         {
            double w = iop[i] + iop[j] + iop[k] + iop[p-1-i-j-k];
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2nk[o++] = 1;
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2nk[o++] = 2;
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2nk[o++] = 3;
         }

   for (int m = 0; m < Dof; m++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(m);
      poly1d.CalcBasis(p, ip.x, shape_x);
      poly1d.CalcBasis(p, ip.y, shape_y);
      poly1d.CalcBasis(p, ip.z, shape_z);
      poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);
      const double *nm = nk + 3*dof2nk[m];

      o = 0;
      for (int k = 0; k <= p; k++)
         for (int j = 0; j + k <= p; j++)
            for (int i = 0; i + j + k <= p; i++)
            {
               double s = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);
               T(o++, m) = s * nm[0];
               T(o++, m) = s * nm[1];
               T(o++, m) = s * nm[2];
            }
      for (int j = 0; j <= p; j++)
         for (int i = 0; i + j <= p; i++)
         {
            double s = shape_x(i)*shape_y(j)*shape_z(p-i-j);
            T(o++, m) = s*((ip.x - c)*nm[0] + (ip.y - c)*nm[1] +
                           (ip.z - c)*nm[2]);
         }
   }

   T.Invert();
   // dbg << "RT_TetrahedronElement(" << p << ") : "; T.TestInversion();
}

void RT_TetrahedronElement::CalcVShape(const IntegrationPoint &ip,
                                       DenseMatrix &shape) const
{
   const int p = Order - 1;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1), shape_y(p + 1), shape_z(p + 1), shape_l(p + 1);
   DenseMatrix u(Dof, Dim);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x);
   poly1d.CalcBasis(p, ip.y, shape_y);
   poly1d.CalcBasis(p, ip.z, shape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l);

   int o = 0;
   for (int k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
         {
            double s = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(p-i-j-k);
            u(o,0) = s;  u(o,1) = 0;  u(o,2) = 0;  o++;
            u(o,0) = 0;  u(o,1) = s;  u(o,2) = 0;  o++;
            u(o,0) = 0;  u(o,1) = 0;  u(o,2) = s;  o++;
         }
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         double s = shape_x(i)*shape_y(j)*shape_z(p-i-j);
         u(o,0) = (ip.x - c)*s;  u(o,1) = (ip.y - c)*s;  u(o,2) = (ip.z - c)*s;
         o++;
      }

   Mult(T, u, shape);
}

void RT_TetrahedronElement::CalcDivShape(const IntegrationPoint &ip,
                                         Vector &divshape) const
{
   const int p = Order - 1;

#ifdef MFEM_USE_OPENMP
   Vector shape_x(p + 1),  shape_y(p + 1),  shape_z(p + 1),  shape_l(p + 1);
   Vector dshape_x(p + 1), dshape_y(p + 1), dshape_z(p + 1), dshape_l(p + 1);
   Vector divu(Dof);
#endif

   poly1d.CalcBasis(p, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(p, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(p, ip.z, shape_z, dshape_z);
   poly1d.CalcBasis(p, 1. - ip.x - ip.y - ip.z, shape_l, dshape_l);

   int o = 0;
   for (int k = 0; k <= p; k++)
      for (int j = 0; j + k <= p; j++)
         for (int i = 0; i + j + k <= p; i++)
         {
            int l = p - i - j - k;
            divu(o++) = (dshape_x(i)*shape_l(l) -
                         shape_x(i)*dshape_l(l))*shape_y(j)*shape_z(k);
            divu(o++) = (dshape_y(j)*shape_l(l) -
                         shape_y(j)*dshape_l(l))*shape_x(i)*shape_z(k);
            divu(o++) = (dshape_z(k)*shape_l(l) -
                         shape_z(k)*dshape_l(l))*shape_x(i)*shape_y(j);
         }
   for (int j = 0; j <= p; j++)
      for (int i = 0; i + j <= p; i++)
      {
         int k = p - i - j;
         divu(o++) =
            (shape_x(i) + (ip.x - c)*dshape_x(i))*shape_y(j)*shape_z(k) +
            (shape_y(j) + (ip.y - c)*dshape_y(j))*shape_x(i)*shape_z(k) +
            (shape_z(k) + (ip.z - c)*dshape_z(k))*shape_x(i)*shape_y(j);
      }

   T.Mult(divu, divshape);
}


const double ND_HexahedronElement::tk[18] =
{ 1.,0.,0.,  0.,1.,0.,  0.,0.,1., -1.,0.,0.,  0.,-1.,0.,  0.,0.,-1. };

ND_HexahedronElement::ND_HexahedronElement(const int p)
   : VectorFiniteElement(3, Geometry::CUBE, 3*p*(p + 1)*(p + 1), p,
                         FunctionSpace::Qk),
     cbasis1d(poly1d.ClosedBasis(p)), obasis1d(poly1d.OpenBasis(p - 1)),
     dof_map(Dof), dof2tk(Dof)
{
   const double *cp = poly1d.ClosedPoints(p);
   const double *op = poly1d.OpenPoints(p - 1);
   const int dof3 = Dof/3;

#ifndef MFEM_USE_OPENMP
   shape_cx.SetSize(p + 1);
   shape_ox.SetSize(p);
   shape_cy.SetSize(p + 1);
   shape_oy.SetSize(p);
   shape_cz.SetSize(p + 1);
   shape_oz.SetSize(p);
   dshape_cx.SetSize(p + 1);
   dshape_cy.SetSize(p + 1);
   dshape_cz.SetSize(p + 1);
#endif

   // edges
   int o = 0;
   for (int i = 0; i < p; i++)  // (0,1)
      dof_map[0*dof3 + i + (0 + 0*(p + 1))*p] = o++;
   for (int i = 0; i < p; i++)  // (1,2)
      dof_map[1*dof3 + p + (i + 0*p)*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (3,2)
      dof_map[0*dof3 + i + (p + 0*(p + 1))*p] = o++;
   for (int i = 0; i < p; i++)  // (0,3)
      dof_map[1*dof3 + 0 + (i + 0*p)*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (4,5)
      dof_map[0*dof3 + i + (0 + p*(p + 1))*p] = o++;
   for (int i = 0; i < p; i++)  // (5,6)
      dof_map[1*dof3 + p + (i + p*p)*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (7,6)
      dof_map[0*dof3 + i + (p + p*(p + 1))*p] = o++;
   for (int i = 0; i < p; i++)  // (4,7)
      dof_map[1*dof3 + 0 + (i + p*p)*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (0,4)
      dof_map[2*dof3 + 0 + (0 + i*(p + 1))*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (1,5)
      dof_map[2*dof3 + p + (0 + i*(p + 1))*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (2,6)
      dof_map[2*dof3 + p + (p + i*(p + 1))*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (3,7)
      dof_map[2*dof3 + 0 + (p + i*(p + 1))*(p + 1)] = o++;

   // faces
   // (3,2,1,0) -- bottom
   for (int j = 1; j < p; j++) // x - components
      for (int i = 0; i < p; i++)
         dof_map[0*dof3 + i + ((p - j) + 0*(p + 1))*p] = o++;
   for (int j = 0; j < p; j++) // y - components
      for (int i = 1; i < p; i++)
         dof_map[1*dof3 + i + ((p - 1 - j) + 0*p)*(p + 1)] = -1 - (o++);
   // (0,1,5,4) -- front
   for (int k = 1; k < p; k++) // x - components
      for (int i = 0; i < p; i++)
         dof_map[0*dof3 + i + (0 + k*(p + 1))*p] = o++;
   for (int k = 0; k < p; k++) // z - components
      for (int i = 1; i < p; i++ )
         dof_map[2*dof3 + i + (0 + k*(p + 1))*(p + 1)] = o++;
   // (1,2,6,5) -- right
   for (int k = 1; k < p; k++) // y - components
      for (int j = 0; j < p; j++)
         dof_map[1*dof3 + p + (j + k*p)*(p + 1)] = o++;
   for (int k = 0; k < p; k++) // z - components
      for (int j = 1; j < p; j++)
         dof_map[2*dof3 + p + (j + k*(p + 1))*(p + 1)] = o++;
   // (2,3,7,6) -- back
   for (int k = 1; k < p; k++) // x - components
      for (int i = 0; i < p; i++)
         dof_map[0*dof3 + (p - 1 - i) + (p + k*(p + 1))*p] = -1 - (o++);
   for (int k = 0; k < p; k++) // z - components
      for (int i = 1; i < p; i++)
         dof_map[2*dof3 + (p - i) + (p + k*(p + 1))*(p + 1)] = o++;
   // (3,0,4,7) -- left
   for (int k = 1; k < p; k++) // y - components
      for (int j = 0; j < p; j++)
         dof_map[1*dof3 + 0 + ((p - 1 - j) + k*p)*(p + 1)] = -1 - (o++);
   for (int k = 0; k < p; k++) // z - components
      for (int j = 1; j < p; j++)
         dof_map[2*dof3 + 0 + ((p - j) + k*(p + 1))*(p + 1)] = o++;
   // (4,5,6,7) -- top
   for (int j = 1; j < p; j++) // x - components
      for (int i = 0; i < p; i++)
         dof_map[0*dof3 + i + (j + p*(p + 1))*p] = o++;
   for (int j = 0; j < p; j++) // y - components
      for (int i = 1; i < p; i++)
         dof_map[1*dof3 + i + (j + p*p)*(p + 1)] = o++;

   // interior
   // x-components
   for (int k = 1; k < p; k++)
      for (int j = 1; j < p; j++)
         for (int i = 0; i < p; i++)
            dof_map[0*dof3 + i + (j + k*(p + 1))*p] = o++;
   // y-components
   for (int k = 1; k < p; k++)
      for (int j = 0; j < p; j++)
         for (int i = 1; i < p; i++)
            dof_map[1*dof3 + i + (j + k*p)*(p + 1)] = o++;
   // z-components
   for (int k = 0; k < p; k++)
      for (int j = 1; j < p; j++)
         for (int i = 1; i < p; i++)
            dof_map[2*dof3 + i + (j + k*(p + 1))*(p + 1)] = o++;

   // set dof2tk and Nodes
   o = 0;
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i < p; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
               dof2tk[idx = -1 - idx] = 3;
            else
               dof2tk[idx] = 0;
            Nodes.IntPoint(idx).Set3(op[i], cp[j], cp[k]);
         }
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j < p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
               dof2tk[idx = -1 - idx] = 4;
            else
               dof2tk[idx] = 1;
            Nodes.IntPoint(idx).Set3(cp[i], op[j], cp[k]);
         }
   // z-components
   for (int k = 0; k < p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx;
            if ((idx = dof_map[o++]) < 0)
               dof2tk[idx = -1 - idx] = 5;
            else
               dof2tk[idx] = 2;
            Nodes.IntPoint(idx).Set3(cp[i], cp[j], op[k]);
         }
}

void ND_HexahedronElement::CalcVShape(const IntegrationPoint &ip,
                                      DenseMatrix &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(p + 1), shape_ox(p), shape_cy(p + 1), shape_oy(p);
   Vector shape_cz(p + 1), shape_oz(p);
#endif

   cbasis1d.Eval(ip.x, shape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy);
   obasis1d.Eval(ip.y, shape_oy);
   cbasis1d.Eval(ip.z, shape_cz);
   obasis1d.Eval(ip.z, shape_oz);

   int o = 0;
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i < p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = s*shape_ox(i)*shape_cy(j)*shape_cz(k);
            shape(idx,1) = 0.;
            shape(idx,2) = 0.;
         }
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j < p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = 0.;
            shape(idx,1) = s*shape_cx(i)*shape_oy(j)*shape_cz(k);
            shape(idx,2) = 0.;
         }
   // z-components
   for (int k = 0; k < p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            shape(idx,0) = 0.;
            shape(idx,1) = 0.;
            shape(idx,2) = s*shape_cx(i)*shape_cy(j)*shape_oz(k);
         }
}

void ND_HexahedronElement::CalcCurlShape(const IntegrationPoint &ip,
                                         DenseMatrix &curl_shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(p + 1), shape_ox(p), shape_cy(p + 1), shape_oy(p);
   Vector shape_cz(p + 1), shape_oz(p);
   Vector dshape_cx(p + 1), dshape_cy(p + 1), dshape_cz(p + 1);
#endif

   cbasis1d.Eval(ip.x, shape_cx, dshape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy, dshape_cy);
   obasis1d.Eval(ip.y, shape_oy);
   cbasis1d.Eval(ip.z, shape_cz, dshape_cz);
   obasis1d.Eval(ip.z, shape_oz);

   int o = 0;
   // x-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i < p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            curl_shape(idx,0) = 0.;
            curl_shape(idx,1) =  s*shape_ox(i)* shape_cy(j)*dshape_cz(k);
            curl_shape(idx,2) = -s*shape_ox(i)*dshape_cy(j)* shape_cz(k);
         }
   // y-components
   for (int k = 0; k <= p; k++)
      for (int j = 0; j < p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            curl_shape(idx,0) = -s* shape_cx(i)*shape_oy(j)*dshape_cz(k);
            curl_shape(idx,1) = 0.;
            curl_shape(idx,2) =  s*dshape_cx(i)*shape_oy(j)* shape_cz(k);
         }
   // z-components
   for (int k = 0; k < p; k++)
      for (int j = 0; j <= p; j++)
         for (int i = 0; i <= p; i++)
         {
            int idx, s;
            if ((idx = dof_map[o++]) < 0)
               idx = -1 - idx, s = -1;
            else
               s = +1;
            curl_shape(idx,0) =   s* shape_cx(i)*dshape_cy(j)*shape_oz(k);
            curl_shape(idx,1) =  -s*dshape_cx(i)* shape_cy(j)*shape_oz(k);
            curl_shape(idx,2) = 0.;
         }
}


const double ND_QuadrilateralElement::tk[8] =
{ 1.,0.,  0.,1., -1.,0., 0.,-1. };

ND_QuadrilateralElement::ND_QuadrilateralElement(const int p)
   : VectorFiniteElement(2, Geometry::SQUARE, 2*p*(p + 1), p,
                         FunctionSpace::Qk),
     cbasis1d(poly1d.ClosedBasis(p)), obasis1d(poly1d.OpenBasis(p - 1)),
     dof_map(Dof), dof2tk(Dof)
{
   const double *cp = poly1d.ClosedPoints(p);
   const double *op = poly1d.OpenPoints(p - 1);
   const int dof2 = Dof/2;

#ifndef MFEM_USE_OPENMP
   shape_cx.SetSize(p + 1);
   shape_ox.SetSize(p);
   shape_cy.SetSize(p + 1);
   shape_oy.SetSize(p);
   dshape_cx.SetSize(p + 1);
   dshape_cy.SetSize(p + 1);
#endif

   // edges
   int o = 0;
   for (int i = 0; i < p; i++)  // (0,1)
      dof_map[0*dof2 + i + 0*p] = o++;
   for (int j = 0; j < p; j++)  // (1,2)
      dof_map[1*dof2 + p + j*(p + 1)] = o++;
   for (int i = 0; i < p; i++)  // (2,3)
      dof_map[0*dof2 + (p - 1 - i) + p*p] = -1 - (o++);
   for (int j = 0; j < p; j++)  // (3,0)
      dof_map[1*dof2 + 0 + (p - 1 - j)*(p + 1)] = -1 - (o++);

   // interior
   // x-components
   for (int j = 1; j < p; j++)
      for (int i = 0; i < p; i++)
         dof_map[0*dof2 + i + j*p] = o++;
   // y-components
   for (int j = 0; j < p; j++)
      for (int i = 1; i < p; i++)
         dof_map[1*dof2 + i + j*(p + 1)] = o++;

   // set dof2tk and Nodes
   o = 0;
   // x-components
   for (int j = 0; j <= p; j++)
      for (int i = 0; i < p; i++)
      {
         int idx;
         if ((idx = dof_map[o++]) < 0)
            dof2tk[idx = -1 - idx] = 2;
         else
            dof2tk[idx] = 0;
         Nodes.IntPoint(idx).Set2(op[i], cp[j]);
      }
   // y-components
   for (int j = 0; j < p; j++)
      for (int i = 0; i <= p; i++)
      {
         int idx;
         if ((idx = dof_map[o++]) < 0)
            dof2tk[idx = -1 - idx] = 3;
         else
            dof2tk[idx] = 1;
         Nodes.IntPoint(idx).Set2(cp[i], op[j]);
      }
}

void ND_QuadrilateralElement::CalcVShape(const IntegrationPoint &ip,
                                         DenseMatrix &shape) const
{
   const int p = Order;

#ifdef MFEM_USE_OPENMP
   Vector shape_cx(p + 1), shape_ox(p), shape_cy(p + 1), shape_oy(p);
#endif

   cbasis1d.Eval(ip.x, shape_cx);
   obasis1d.Eval(ip.x, shape_ox);
   cbasis1d.Eval(ip.y, shape_cy);
   obasis1d.Eval(ip.y, shape_oy);

   int o = 0;
   // x-components
   for (int j = 0; j <= p; j++)
      for (int i = 0; i < p; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         shape(idx,0) = s*shape_ox(i)*shape_cy(j);
         shape(idx,1) = 0.;
      }
   // y-components
   for (int j = 0; j < p; j++)
      for (int i = 0; i <= p; i++)
      {
         int idx, s;
         if ((idx = dof_map[o++]) < 0)
            idx = -1 - idx, s = -1;
         else
            s = +1;
         shape(idx,0) = 0.;
         shape(idx,1) = s*shape_cx(i)*shape_oy(j);
      }
}

void ND_QuadrilateralElement::CalcCurlShape(const IntegrationPoint &ip,
                                            DenseMatrix &curl_shape) const
{
   mfem_error("ND_QuadrilateralElement::CalcCurlShape");
}


const double ND_TetrahedronElement::tk[18] =
{ 1.,0.,0.,  0.,1.,0.,  0.,0.,1.,  -1.,1.,0.,  -1.,0.,1.,  0.,-1.,1. };

const double ND_TetrahedronElement::c = 1./4.;

ND_TetrahedronElement::ND_TetrahedronElement(const int p)
   : VectorFiniteElement(3, Geometry::TETRAHEDRON, p*(p + 2)*(p + 3)/2, p,
                         FunctionSpace::Pk), dof2tk(Dof), T(Dof)
{
   const double *eop = poly1d.OpenPoints(p - 1);
   const double *fop = (p > 1) ? poly1d.OpenPoints(p - 2) : NULL;
   const double *iop = (p > 2) ? poly1d.OpenPoints(p - 3) : NULL;

   const int pm1 = p - 1, pm2 = p - 2, pm3 = p - 3;

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p);
   shape_y.SetSize(p);
   shape_z.SetSize(p);
   shape_l.SetSize(p);
   dshape_x.SetSize(p);
   dshape_y.SetSize(p);
   dshape_z.SetSize(p);
   dshape_l.SetSize(p);
   u.SetSize(Dof, Dim);
#else
   Vector shape_x(p), shape_y(p), shape_z(p), shape_l(p);
#endif

   int o = 0;
   // edges
   for (int i = 0; i < p; i++) // (0,1)
   {
      Nodes.IntPoint(o).Set3(eop[i], 0., 0.);
      dof2tk[o++] = 0;
   }
   for (int i = 0; i < p; i++) // (0,2)
   {
      Nodes.IntPoint(o).Set3(0., eop[i], 0.);
      dof2tk[o++] = 1;
   }
   for (int i = 0; i < p; i++) // (0,3)
   {
      Nodes.IntPoint(o).Set3(0., 0., eop[i]);
      dof2tk[o++] = 2;
   }
   for (int i = 0; i < p; i++) // (1,2)
   {
      Nodes.IntPoint(o).Set3(eop[pm1-i], eop[i], 0.);
      dof2tk[o++] = 3;
   }
   for (int i = 0; i < p; i++) // (1,3)
   {
      Nodes.IntPoint(o).Set3(eop[pm1-i], 0., eop[i]);
      dof2tk[o++] = 4;
   }
   for (int i = 0; i < p; i++) // (2,3)
   {
      Nodes.IntPoint(o).Set3(0., eop[pm1-i], eop[i]);
      dof2tk[o++] = 5;
   }

   // faces
   for (int j = 0; j <= pm2; j++)  // (1,2,3)
      for (int i = 0; i + j <= pm2; i++)
      {
         double w = fop[i] + fop[j] + fop[pm2-i-j];
         Nodes.IntPoint(o).Set3(fop[pm2-i-j]/w, fop[i]/w, fop[j]/w);
         dof2tk[o++] = 3;
         Nodes.IntPoint(o).Set3(fop[pm2-i-j]/w, fop[i]/w, fop[j]/w);
         dof2tk[o++] = 4;
      }
   for (int j = 0; j <= pm2; j++)  // (0,3,2)
      for (int i = 0; i + j <= pm2; i++)
      {
         double w = fop[i] + fop[j] + fop[pm2-i-j];
         Nodes.IntPoint(o).Set3(0., fop[j]/w, fop[i]/w);
         dof2tk[o++] = 2;
         Nodes.IntPoint(o).Set3(0., fop[j]/w, fop[i]/w);
         dof2tk[o++] = 1;
      }
   for (int j = 0; j <= pm2; j++)  // (0,1,3)
      for (int i = 0; i + j <= pm2; i++)
      {
         double w = fop[i] + fop[j] + fop[pm2-i-j];
         Nodes.IntPoint(o).Set3(fop[i]/w, 0., fop[j]/w);
         dof2tk[o++] = 0;
         Nodes.IntPoint(o).Set3(fop[i]/w, 0., fop[j]/w);
         dof2tk[o++] = 2;
      }
   for (int j = 0; j <= pm2; j++)  // (0,2,1)
      for (int i = 0; i + j <= pm2; i++)
      {
         double w = fop[i] + fop[j] + fop[pm2-i-j];
         Nodes.IntPoint(o).Set3(fop[j]/w, fop[i]/w, 0.);
         dof2tk[o++] = 1;
         Nodes.IntPoint(o).Set3(fop[j]/w, fop[i]/w, 0.);
         dof2tk[o++] = 0;
      }

   // interior
   for (int k = 0; k <= pm3; k++)
      for (int j = 0; j + k <= pm3; j++)
         for (int i = 0; i + j + k <= pm3; i++)
         {
            double w = iop[i] + iop[j] + iop[k] + iop[pm3-i-j-k];
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2tk[o++] = 0;
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2tk[o++] = 1;
            Nodes.IntPoint(o).Set3(iop[i]/w, iop[j]/w, iop[k]/w);
            dof2tk[o++] = 2;
         }

   for (int m = 0; m < Dof; m++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(m);
      const double *tm = tk + 3*dof2tk[m];
      o = 0;

      poly1d.CalcBasis(pm1, ip.x, shape_x);
      poly1d.CalcBasis(pm1, ip.y, shape_y);
      poly1d.CalcBasis(pm1, ip.z, shape_z);
      poly1d.CalcBasis(pm1, 1. - ip.x - ip.y - ip.z, shape_l);

      for (int k = 0; k <= pm1; k++)
         for (int j = 0; j + k <= pm1; j++)
            for (int i = 0; i + j + k <= pm1; i++)
            {
               double s = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(pm1-i-j-k);
               T(o++, m) = s * tm[0];
               T(o++, m) = s * tm[1];
               T(o++, m) = s * tm[2];
            }
      for (int k = 0; k <= pm1; k++)
         for (int j = 0; j + k <= pm1; j++)
         {
            double s = shape_x(pm1-j-k)*shape_y(j)*shape_z(k);
            T(o++, m) = s*((ip.y - c)*tm[0] - (ip.x - c)*tm[1]);
            T(o++, m) = s*((ip.z - c)*tm[0] - (ip.x - c)*tm[2]);
         }
      for (int k = 0; k <= pm1; k++)
      {
         T(o++, m) =
            shape_y(pm1-k)*shape_z(k)*((ip.z - c)*tm[1] - (ip.y - c)*tm[2]);
      }
   }

   T.Invert();
   // dbg << "ND_TetrahedronElement(" << p << ") : "; T.TestInversion();
}

void ND_TetrahedronElement::CalcVShape(const IntegrationPoint &ip,
                                       DenseMatrix &shape) const
{
   const int pm1 = Order - 1;

#ifdef MFEM_USE_OPENMP
   const int p = Order;
   Vector shape_x(p), shape_y(p), shape_z(p), shape_l(p);
   DenseMatrix u(Dof, Dim);
#endif

   poly1d.CalcBasis(pm1, ip.x, shape_x);
   poly1d.CalcBasis(pm1, ip.y, shape_y);
   poly1d.CalcBasis(pm1, ip.z, shape_z);
   poly1d.CalcBasis(pm1, 1. - ip.x - ip.y - ip.z, shape_l);

   int n = 0;
   for (int k = 0; k <= pm1; k++)
      for (int j = 0; j + k <= pm1; j++)
         for (int i = 0; i + j + k <= pm1; i++)
         {
            double s = shape_x(i)*shape_y(j)*shape_z(k)*shape_l(pm1-i-j-k);
            u(n,0) =  s;  u(n,1) = 0.;  u(n,2) = 0.;  n++;
            u(n,0) = 0.;  u(n,1) =  s;  u(n,2) = 0.;  n++;
            u(n,0) = 0.;  u(n,1) = 0.;  u(n,2) =  s;  n++;
         }
   for (int k = 0; k <= pm1; k++)
      for (int j = 0; j + k <= pm1; j++)
      {
         double s = shape_x(pm1-j-k)*shape_y(j)*shape_z(k);
         u(n,0) = s*(ip.y - c);  u(n,1) = -s*(ip.x - c);  u(n,2) =  0.;  n++;
         u(n,0) = s*(ip.z - c);  u(n,1) =  0.;  u(n,2) = -s*(ip.x - c);  n++;
      }
   for (int k = 0; k <= pm1; k++)
   {
      double s = shape_y(pm1-k)*shape_z(k);
      u(n,0) = 0.;  u(n,1) = s*(ip.z - c);  u(n,2) = -s*(ip.y - c);  n++;
   }

   Mult(T, u, shape);
}

void ND_TetrahedronElement::CalcCurlShape(const IntegrationPoint &ip,
                                          DenseMatrix &curl_shape) const
{
   const int pm1 = Order - 1;

#ifdef MFEM_USE_OPENMP
   const int p = Order;
   Vector shape_x(p), shape_y(p), shape_z(p), shape_l(p);
   Vector dshape_x(p), dshape_y(p), dshape_z(p), dshape_l(p);
   DenseMatrix u(Dof, Dim);
#endif

   poly1d.CalcBasis(pm1, ip.x, shape_x, dshape_x);
   poly1d.CalcBasis(pm1, ip.y, shape_y, dshape_y);
   poly1d.CalcBasis(pm1, ip.z, shape_z, dshape_z);
   poly1d.CalcBasis(pm1, 1. - ip.x - ip.y - ip.z, shape_l, dshape_l);

   int n = 0;
   for (int k = 0; k <= pm1; k++)
      for (int j = 0; j + k <= pm1; j++)
         for (int i = 0; i + j + k <= pm1; i++)
         {
            int l = pm1-i-j-k;
            const double dx = (dshape_x(i)*shape_l(l) -
                               shape_x(i)*dshape_l(l))*shape_y(j)*shape_z(k);
            const double dy = (dshape_y(j)*shape_l(l) -
                               shape_y(j)*dshape_l(l))*shape_x(i)*shape_z(k);
            const double dz = (dshape_z(k)*shape_l(l) -
                               shape_z(k)*dshape_l(l))*shape_x(i)*shape_y(j);

            u(n,0) =  0.;  u(n,1) =  dz;  u(n,2) = -dy;  n++;
            u(n,0) = -dz;  u(n,1) =  0.;  u(n,2) =  dx;  n++;
            u(n,0) =  dy;  u(n,1) = -dx;  u(n,2) =  0.;  n++;
         }
   for (int k = 0; k <= pm1; k++)
      for (int j = 0; j + k <= pm1; j++)
      {
         int i = pm1 - j - k;
         // s = shape_x(i)*shape_y(j)*shape_z(k);
         // curl of s*(ip.y - c, -(ip.x - c), 0):
         u(n,0) =  shape_x(i)*(ip.x - c)*shape_y(j)*dshape_z(k);
         u(n,1) =  shape_x(i)*shape_y(j)*(ip.y - c)*dshape_z(k);
         u(n,2) =
            -((dshape_x(i)*(ip.x - c) + shape_x(i))*shape_y(j)*shape_z(k) +
              (dshape_y(j)*(ip.y - c) + shape_y(j))*shape_x(i)*shape_z(k));
         n++;
         // curl of s*(ip.z - c, 0, -(ip.x - c)):
         u(n,0) = -shape_x(i)*(ip.x - c)*dshape_y(j)*shape_z(k);
         u(n,1) = (shape_x(i)*shape_y(j)*(dshape_z(k)*(ip.z - c) + shape_z(k)) +
                   (dshape_x(i)*(ip.x - c) + shape_x(i))*shape_y(j)*shape_z(k));
         u(n,2) = -shape_x(i)*dshape_y(j)*shape_z(k)*(ip.z - c);
         n++;
      }
   for (int k = 0; k <= pm1; k++)
   {
      int j = pm1 - k;
      // curl of shape_y(j)*shape_z(k)*(0, ip.z - c, -(ip.y - c)):
      u(n,0) = -((dshape_y(j)*(ip.y - c) + shape_y(j))*shape_z(k) +
                 shape_y(j)*(dshape_z(k)*(ip.z - c) + shape_z(k)));
      u(n,1) = 0.;
      u(n,2) = 0.;  n++;
   }

   Mult(T, u, curl_shape);
}


const double ND_TriangleElement::tk[8] =
{ 1.,0.,  -1.,1.,  0.,-1.,  0.,1. };

const double ND_TriangleElement::c = 1./3.;

ND_TriangleElement::ND_TriangleElement(const int p)
   : VectorFiniteElement(2, Geometry::TRIANGLE, p*(p + 2), p,
                         FunctionSpace::Pk), dof2tk(Dof), T(Dof)
{
   const double *eop = poly1d.OpenPoints(p - 1);
   const double *iop = (p > 1) ? poly1d.OpenPoints(p - 2) : NULL;

   const int pm1 = p - 1, pm2 = p - 2;

#ifndef MFEM_USE_OPENMP
   shape_x.SetSize(p);
   shape_y.SetSize(p);
   shape_l.SetSize(p);
   dshape_x.SetSize(p);
   dshape_y.SetSize(p);
   dshape_l.SetSize(p);
   u.SetSize(Dof, Dim);
#else
   Vector shape_x(p), shape_y(p), shape_l(p);
#endif

   int n = 0;
   // edges
   for (int i = 0; i < p; i++) // (0,1)
   {
      Nodes.IntPoint(n).Set2(eop[i], 0.);
      dof2tk[n++] = 0;
   }
   for (int i = 0; i < p; i++) // (1,2)
   {
      Nodes.IntPoint(n).Set2(eop[pm1-i], eop[i]);
      dof2tk[n++] = 1;
   }
   for (int i = 0; i < p; i++) // (2,0)
   {
      Nodes.IntPoint(n).Set2(0., eop[pm1-i]);
      dof2tk[n++] = 2;
   }

   // interior
   for (int j = 0; j <= pm2; j++)
      for (int i = 0; i + j <= pm2; i++)
      {
         double w = iop[i] + iop[j] + iop[pm2-i-j];
         Nodes.IntPoint(n).Set2(iop[i]/w, iop[j]/w);
         dof2tk[n++] = 0;
         Nodes.IntPoint(n).Set2(iop[i]/w, iop[j]/w);
         dof2tk[n++] = 3;
      }

   for (int m = 0; m < Dof; m++)
   {
      const IntegrationPoint &ip = Nodes.IntPoint(m);
      const double *tm = tk + 2*dof2tk[m];
      n = 0;

      poly1d.CalcBasis(pm1, ip.x, shape_x);
      poly1d.CalcBasis(pm1, ip.y, shape_y);
      poly1d.CalcBasis(pm1, 1. - ip.x - ip.y, shape_l);

      for (int j = 0; j <= pm1; j++)
         for (int i = 0; i + j <= pm1; i++)
         {
            double s = shape_x(i)*shape_y(j)*shape_l(pm1-i-j);
            T(n++, m) = s * tm[0];
            T(n++, m) = s * tm[1];
         }
      for (int j = 0; j <= pm1; j++)
      {
         T(n++ ,m) =
            shape_x(pm1-j)*shape_y(j)*((ip.y - c)*tm[0] - (ip.x - c)*tm[1]);
      }
   }

   T.Invert();
   // dbg << "ND_TriangleElement(" << p << ") : "; T.TestInversion();
}

void ND_TriangleElement::CalcVShape(const IntegrationPoint &ip,
                                    DenseMatrix &shape) const
{
   const int pm1 = Order - 1;

#ifdef MFEM_USE_OPENMP
   const int p = Order;
   Vector shape_x(p), shape_y(p), shape_l(p);
   DenseMatrix u(Dof, Dim);
#endif

   poly1d.CalcBasis(pm1, ip.x, shape_x);
   poly1d.CalcBasis(pm1, ip.y, shape_y);
   poly1d.CalcBasis(pm1, 1. - ip.x - ip.y, shape_l);

   int n = 0;
   for (int j = 0; j <= pm1; j++)
      for (int i = 0; i + j <= pm1; i++)
      {
         double s = shape_x(i)*shape_y(j)*shape_l(pm1-i-j);
         u(n,0) =  s;  u(n,1) = 0.;  n++;
         u(n,0) = 0.;  u(n,1) =  s;  n++;
      }
   for (int j = 0; j <= pm1; j++)
   {
      double s = shape_x(pm1-j)*shape_y(j);
      u(n,0) =  s*(ip.y - c);
      u(n,1) = -s*(ip.x - c);
   }

   Mult(T, u, shape);
}

void ND_TriangleElement::CalcCurlShape(const IntegrationPoint &ip,
                                       DenseMatrix &curl_shape) const
{
   mfem_error("ND_TriangleElement::CalcCurlShape");
}


void NURBS1DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   kv[0]->CalcShape(shape, ijk[0], ip.x);

   double sum = 0.0;
   for (int i = 0; i <= Order; i++)
      sum += (shape(i) *= weights(i));

   shape /= sum;
}

void NURBS1DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   Vector grad(dshape.Data(), Dof);

   kv[0]->CalcShape (shape_x, ijk[0], ip.x);
   kv[0]->CalcDShape(grad,    ijk[0], ip.x);

   double sum = 0.0, dsum = 0.0;
   for (int i = 0; i <= Order; i++)
   {
      sum  += (shape_x(i) *= weights(i));
      dsum += (   grad(i) *= weights(i));
   }

   sum = 1.0/sum;
   add(sum, grad, -dsum*sum*sum, shape_x, grad);
}

void NURBS2DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   kv[0]->CalcShape(shape_x, ijk[0], ip.x);
   kv[1]->CalcShape(shape_y, ijk[1], ip.y);

   double sum = 0.0;
   for (int o = 0, j = 0; j <= Order; j++)
   {
      const double sy = shape_y(j);
      for (int i = 0; i <= Order; i++, o++)
      {
         sum += ( shape(o) = shape_x(i)*sy*weights(o) );
      }
   }

   shape /= sum;
}

void NURBS2DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   double sum, dsum[2];

   kv[0]->CalcShape ( shape_x, ijk[0], ip.x);
   kv[1]->CalcShape ( shape_y, ijk[1], ip.y);

   kv[0]->CalcDShape(dshape_x, ijk[0], ip.x);
   kv[1]->CalcDShape(dshape_y, ijk[1], ip.y);

   sum = dsum[0] = dsum[1] = 0.0;
   for (int o = 0, j = 0; j <= Order; j++)
   {
      const double sy = shape_y(j), dsy = dshape_y(j);
      for (int i = 0; i <= Order; i++, o++)
      {
         sum += ( u(o) = shape_x(i)*sy*weights(o) );

         dsum[0] += ( dshape(o,0) = dshape_x(i)*sy *weights(o) );
         dsum[1] += ( dshape(o,1) =  shape_x(i)*dsy*weights(o) );
      }
   }

   sum = 1.0/sum;
   dsum[0] *= sum*sum;
   dsum[1] *= sum*sum;

   for (int o = 0; o < Dof; o++)
   {
      dshape(o,0) = dshape(o,0)*sum - u(o)*dsum[0];
      dshape(o,1) = dshape(o,1)*sum - u(o)*dsum[1];
   }
}

void NURBS3DFiniteElement::CalcShape(const IntegrationPoint &ip,
                                     Vector &shape) const
{
   kv[0]->CalcShape(shape_x, ijk[0], ip.x);
   kv[1]->CalcShape(shape_y, ijk[1], ip.y);
   kv[2]->CalcShape(shape_z, ijk[2], ip.z);

   double sum = 0.0;
   for (int o = 0, k = 0; k <= Order; k++)
   {
      const double sz = shape_z(k);
      for (int j = 0; j <= Order; j++)
      {
         const double sy_sz = shape_y(j)*sz;
         for (int i = 0; i <= Order; i++, o++)
         {
            sum += ( shape(o) = shape_x(i)*sy_sz*weights(o) );
         }
      }
   }

   shape /= sum;
}

void NURBS3DFiniteElement::CalcDShape(const IntegrationPoint &ip,
                                      DenseMatrix &dshape) const
{
   double sum, dsum[3];

   kv[0]->CalcShape ( shape_x, ijk[0], ip.x);
   kv[1]->CalcShape ( shape_y, ijk[1], ip.y);
   kv[2]->CalcShape ( shape_z, ijk[2], ip.z);

   kv[0]->CalcDShape(dshape_x, ijk[0], ip.x);
   kv[1]->CalcDShape(dshape_y, ijk[1], ip.y);
   kv[2]->CalcDShape(dshape_z, ijk[2], ip.z);

   sum = dsum[0] = dsum[1] = dsum[2] = 0.0;
   for (int o = 0, k = 0; k <= Order; k++)
   {
      const double sz = shape_z(k), dsz = dshape_z(k);
      for (int j = 0; j <= Order; j++)
      {
         const double  sy_sz  =  shape_y(j)* sz;
         const double dsy_sz  = dshape_y(j)* sz;
         const double  sy_dsz =  shape_y(j)*dsz;
         for (int i = 0; i <= Order; i++, o++)
         {
            sum += ( u(o) = shape_x(i)*sy_sz*weights(o) );

            dsum[0] += ( dshape(o,0) = dshape_x(i)* sy_sz *weights(o) );
            dsum[1] += ( dshape(o,1) =  shape_x(i)*dsy_sz *weights(o) );
            dsum[2] += ( dshape(o,2) =  shape_x(i)* sy_dsz*weights(o) );
         }
      }
   }

   sum = 1.0/sum;
   dsum[0] *= sum*sum;
   dsum[1] *= sum*sum;
   dsum[2] *= sum*sum;

   for (int o = 0; o < Dof; o++)
   {
      dshape(o,0) = dshape(o,0)*sum - u(o)*dsum[0];
      dshape(o,1) = dshape(o,1)*sum - u(o)*dsum[1];
      dshape(o,2) = dshape(o,2)*sum - u(o)*dsum[2];
   }
}
