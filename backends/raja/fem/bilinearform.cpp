// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443211. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the MFEM library. For more information and source code
// availability see http://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

#include "../../../config/config.hpp"
#if defined(MFEM_USE_BACKENDS) && defined(MFEM_USE_RAJA)

#include "../raja.hpp"

//#if 0

namespace mfem
{

namespace raja
{

RajaBilinearForm::RajaBilinearForm(RajaFiniteElementSpace *ofespace_) :
   Operator(ofespace_->RajaVLayout()),
   localX(ofespace_->RajaEVLayout()),
   localY(ofespace_->RajaEVLayout())
{
   push();
   Init(ofespace_->RajaEngine(), ofespace_, ofespace_);
   pop();
}

RajaBilinearForm::RajaBilinearForm(RajaFiniteElementSpace *otrialFESpace_,
                                   RajaFiniteElementSpace *otestFESpace_) :
   Operator(otrialFESpace_->RajaVLayout(),
            otestFESpace_->RajaVLayout()),
   localX(otrialFESpace_->RajaEVLayout()),
   localY(otestFESpace_->RajaEVLayout())
{
   push();
   Init(otrialFESpace_->RajaEngine(), otrialFESpace_, otestFESpace_);
   pop();
}

void RajaBilinearForm::Init(const Engine &e,
                            RajaFiniteElementSpace *otrialFESpace_,
                            RajaFiniteElementSpace *otestFESpace_)
{
   push();
   engine.Reset(&e);
   otrialFESpace = otrialFESpace_;
   trialFESpace  = otrialFESpace_->GetFESpace();

   otestFESpace = otestFESpace_;
   testFESpace  = otestFESpace_->GetFESpace();

   mesh = trialFESpace->GetMesh();
   dbg(" done!");
   pop();
}

int RajaBilinearForm::BaseGeom() const
{
   push();
   pop();
   return mesh->GetElementBaseGeometry();
}

int RajaBilinearForm::GetDim() const
{
   push(); pop();
   return mesh->Dimension();
}

int64_t RajaBilinearForm::GetNE() const
{
   push(); pop();
   return mesh->GetNE();
}

Mesh& RajaBilinearForm::GetMesh() const
{
   push(); pop();
   return *mesh;
}

RajaFiniteElementSpace& RajaBilinearForm::GetTrialRajaFESpace() const
{
   push(); pop();
   return *otrialFESpace;
}

RajaFiniteElementSpace& RajaBilinearForm::GetTestRajaFESpace() const
{
   push(); pop();
   return *otestFESpace;
}

mfem::FiniteElementSpace& RajaBilinearForm::GetTrialFESpace() const
{
   push(); pop();
   return *trialFESpace;
}

mfem::FiniteElementSpace& RajaBilinearForm::GetTestFESpace() const
{
   push(); pop();
   return *testFESpace;
}

int64_t RajaBilinearForm::GetTrialNDofs() const
{
   push(); pop();
   return trialFESpace->GetNDofs();
}

int64_t RajaBilinearForm::GetTestNDofs() const
{
   return testFESpace->GetNDofs();
}

int64_t RajaBilinearForm::GetTrialVDim() const
{
   return trialFESpace->GetVDim();
}

int64_t RajaBilinearForm::GetTestVDim() const
{
   return testFESpace->GetVDim();
}

const FiniteElement& RajaBilinearForm::GetTrialFE(const int i) const
{
   return *(trialFESpace->GetFE(i));
}

const FiniteElement& RajaBilinearForm::GetTestFE(const int i) const
{
   return *(testFESpace->GetFE(i));
}

// Adds new Domain Integrator.
void RajaBilinearForm::AddDomainIntegrator(RajaIntegrator *integrator)
{
   push();
   AddIntegrator(integrator, DomainIntegrator);
   pop();
}

// Adds new Boundary Integrator.
void RajaBilinearForm::AddBoundaryIntegrator(RajaIntegrator *integrator)
{
   push();
   AddIntegrator(integrator, BoundaryIntegrator);
   pop();
}

// Adds new interior Face Integrator.
void RajaBilinearForm::AddInteriorFaceIntegrator(RajaIntegrator *integrator)
{
   push();
   AddIntegrator(integrator, InteriorFaceIntegrator);
   pop();
}

// Adds new boundary Face Integrator.
void RajaBilinearForm::AddBoundaryFaceIntegrator(RajaIntegrator *integrator)
{
   push();
   AddIntegrator(integrator, BoundaryFaceIntegrator);
   pop();
}

// Adds Integrator based on RajaIntegratorType
void RajaBilinearForm::AddIntegrator(RajaIntegrator *integrator,
                                     const RajaIntegratorType itype)
{
   push();
   if (integrator == NULL)
   {
      std::stringstream error_ss;
      error_ss << "RajaBilinearForm::";
      switch (itype)
      {
         case DomainIntegrator      : error_ss << "AddDomainIntegrator";       break;
         case BoundaryIntegrator    : error_ss << "AddBoundaryIntegrator";     break;
         case InteriorFaceIntegrator: error_ss << "AddInteriorFaceIntegrator"; break;
         case BoundaryFaceIntegrator: error_ss << "AddBoundaryFaceIntegrator"; break;
      }
      error_ss << " (...):\n"
               << "  Integrator is NULL";
      const std::string error = error_ss.str();
      mfem_error(error.c_str());
   }
   integrator->SetupIntegrator(*this, itype);
   integrators.push_back(integrator);
   pop();
}

const mfem::Operator* RajaBilinearForm::GetTrialProlongation() const
{
   return otrialFESpace->GetProlongationOperator();
}

const mfem::Operator* RajaBilinearForm::GetTestProlongation() const
{
   return otestFESpace->GetProlongationOperator();
}

const mfem::Operator* RajaBilinearForm::GetTrialRestriction() const
{
   return otrialFESpace->GetRestrictionOperator();
}

const mfem::Operator* RajaBilinearForm::GetTestRestriction() const
{
   return otestFESpace->GetRestrictionOperator();
}

void RajaBilinearForm::Assemble()
{
   push();
   // [MISSING] Find geometric information that is needed by intergrators
   //             to share between integrators.
   const int integratorCount = (int) integrators.size();
   for (int i = 0; i < integratorCount; ++i)
   {
      integrators[i]->Assemble();
   }
   pop();
}

void RajaBilinearForm::FormLinearSystem(const mfem::Array<int> &constraintList,
                                        mfem::Vector &x, mfem::Vector &b,
                                        mfem::Operator *&Aout,
                                        mfem::Vector &X, mfem::Vector &B,
                                        int copy_interior)
{
   push();
   FormOperator(constraintList, Aout);
   InitRHS(constraintList, x, b, Aout, X, B, copy_interior);
   pop();
}

void RajaBilinearForm::FormOperator(const mfem::Array<int> &constraintList,
                                    mfem::Operator *&Aout)
{
   push();
   const mfem::Operator *trialP = GetTrialProlongation();
   const mfem::Operator *testP  = GetTestProlongation();
   mfem::Operator *rap = this;

   if (trialP)
   {
      rap = new RAPOperator(*testP, *this, *trialP);
   }

   Aout = new RajaConstrainedOperator(rap, constraintList,
                                      rap != this);
   pop();
}

void RajaBilinearForm::InitRHS(const mfem::Array<int> &constraintList,
                               mfem::Vector &x, mfem::Vector &b,
                               mfem::Operator *A,
                               mfem::Vector &X, mfem::Vector &B,
                               int copy_interior)
{
   push();
   const mfem::Operator *P = GetTrialProlongation();
   const mfem::Operator *R = GetTrialRestriction();
   if (P)
   {
      // Variational restriction with P
      B.Resize(P->InLayout());
      P->MultTranspose(b, B);
      X.Resize(R->OutLayout());
      R->Mult(x, X);
   }
   else
   {
      // rap, X and B point to the same data as this, x and b
      assert(false);
      X.MakeRef(x);
      B.MakeRef(b);
   }

   if (!copy_interior && constraintList.Size() > 0)
   {
      const Array &constrList = constraintList.Get_PArray()->As<Array>();
      Vector subvec(constrList.RajaLayout());
      vector_get_subvector(constraintList.Size(),
                           (double*)subvec.RajaMem().ptr(),
                           (double*)X.Get_PVector()->As<Vector>().RajaMem().ptr(),
                           (int*)constrList.RajaMem().ptr());
      X.Fill(0.0);
      vector_set_subvector(constraintList.Size(),
                           (double*)X.Get_PVector()->As<Vector>().RajaMem().ptr(),
                           (double*)subvec.RajaMem().ptr(),
                           (int*)constrList.RajaMem().ptr());
   }

   RajaConstrainedOperator *cA = dynamic_cast<RajaConstrainedOperator*>(A);
   if (cA)
   {
      cA->EliminateRHS(X.Get_PVector()->As<Vector>(),
                       B.Get_PVector()->As<Vector>());
   }
   else
   {
      mfem_error("RajaBilinearForm::InitRHS expects an RajaConstrainedOperator");
   }
   pop();
}

// Matrix vector multiplication.
void RajaBilinearForm::Mult_(const Vector &x, Vector &y) const
{
   push();
   otrialFESpace->GlobalToLocal(x, localX);
   localY.Fill<double>(0.0);

   const int integratorCount = (int) integrators.size();
   for (int i = 0; i < integratorCount; ++i)
   {
      integrators[i]->MultAdd(localX, localY);
   }
   otestFESpace->LocalToGlobal(localY, y);
   pop();
}

// Matrix transpose vector multiplication.
void RajaBilinearForm::MultTranspose_(const Vector &x, Vector &y) const
{
   push();
   assert(false);// otestFESpace->GlobalToLocal(x, localX);
   localY.Fill<double>(0.0);

   const int integratorCount = (int) integrators.size();
   for (int i = 0; i < integratorCount; ++i)
   {
      integrators[i]->MultTransposeAdd(localX, localY);
   }

   assert(false);//otrialFESpace->LocalToGlobal(localY, y);
   pop();
}

void RajaBilinearForm::RajaRecoverFEMSolution(const mfem::Vector &X,
                                              const mfem::Vector &b,
                                              mfem::Vector &x)
{
   const mfem::Operator *P = this->GetTrialProlongation();
   if (P)
   {
      // Apply conforming prolongation
      x.Resize(P->OutLayout());
      P->Mult(X, x);
   }
   // Otherwise X and x point to the same data
}

// Frees memory bilinear form.
RajaBilinearForm::~RajaBilinearForm()
{
   // Make sure all integrators free their data
   IntegratorVector::iterator it = integrators.begin();
   while (it != integrators.end())
   {
      delete *it;
      ++it;
   }
}


void BilinearForm::InitRajaBilinearForm()
{
   push();
   // Init 'obform' using 'bform'
   MFEM_ASSERT(bform != NULL, "");
   MFEM_ASSERT(obform == NULL, "");

   RajaFiniteElementSpace &ofes =
      bform->FESpace()->Get_PFESpace()->As<RajaFiniteElementSpace>();
   obform = new RajaBilinearForm(&ofes);

   dbg(", transfer domain integrators");
   mfem::Array<mfem::BilinearFormIntegrator*> &dbfi = *bform->GetDBFI();
   for (int i = 0; i < dbfi.Size(); i++)
   {
      std::string integ_name(dbfi[i]->Name());
      dbg(", integ_name: %s",integ_name.c_str());
      Coefficient *scal_coeff = dbfi[i]->GetScalarCoefficient();
      ConstantCoefficient *const_coeff =
         dynamic_cast<ConstantCoefficient*>(scal_coeff);
      // TODO: other types of coefficients ...
      double val = const_coeff ? const_coeff->constant : 1.0;
      RajaCoefficient ocoeff(obform->RajaEngine(), val);

      RajaIntegrator *ointeg = NULL;

      if (integ_name == "(undefined)")
      {
         MFEM_ABORT("BilinearFormIntegrator does not define Name()");
         ointeg = new RajaMassIntegrator(ocoeff);
      }
      else if (integ_name == "diffusion")
      {
         assert(false);
         ointeg = new RajaDiffusionIntegrator(ocoeff);
      }
      else
      {
         MFEM_ABORT("BilinearFormIntegrator [Name() = " << integ_name
                    << "] is not supported");
      }

      const mfem::IntegrationRule *ir = dbfi[i]->GetIntRule();
      if (ir) { ointeg->SetIntegrationRule(*ir); }

      obform->AddDomainIntegrator(ointeg);
   }
   pop();
   // TODO: other types of integrators ...
}

bool BilinearForm::Assemble()
{
   push();
   if (obform == NULL) { InitRajaBilinearForm(); }
   obform->Assemble();
   pop();
   return true; // --> host assembly is not needed
}

void BilinearForm::FormSystemMatrix(const mfem::Array<int> &ess_tdof_list,
                                    mfem::OperatorHandle &A)
{
   push();
   if (A.Type() == mfem::Operator::ANY_TYPE)
   {
      mfem::Operator *Aout = NULL;
      obform->FormOperator(ess_tdof_list, Aout);
      A.Reset(Aout);
   }
   else
   {
      MFEM_ABORT("Operator::Type is not supported, type = " << A.Type());
   }
   pop();
}

void BilinearForm::FormLinearSystem(const mfem::Array<int> &ess_tdof_list,
                                    mfem::Vector &x, mfem::Vector &b,
                                    mfem::OperatorHandle &A,
                                    mfem::Vector &X, mfem::Vector &B,
                                    int copy_interior)
{
   push();
   FormSystemMatrix(ess_tdof_list, A);
   obform->InitRHS(ess_tdof_list, x, b, A.Ptr(), X, B, copy_interior);
   pop();
}

void BilinearForm::RecoverFEMSolution(const mfem::Vector &X,
                                      const mfem::Vector &b,
                                      mfem::Vector &x)
{
   push();
   obform->RajaRecoverFEMSolution(X, b, x);
   pop();
}

} // namespace mfem::raja

} // namespace mfem

//#endif // 0

#endif // defined(MFEM_USE_BACKENDS) && defined(MFEM_USE_RAJA)