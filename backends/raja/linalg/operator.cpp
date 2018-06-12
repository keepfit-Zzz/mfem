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

namespace mfem
{

namespace raja
{

RajaConstrainedOperator::RajaConstrainedOperator(
   mfem::Operator *A_,
   const mfem::Array<int> &constraintList_,
   bool own_A_)

   : Operator(A_->InLayout()->As<Layout>()),
     z(OutLayout_()),
     w(OutLayout_()),
     mfem_z((z.DontDelete(), z)),
     mfem_w((w.DontDelete(), w))
{
   push();
   Setup(OutLayout_().RajaEngine().GetDevice(), A_, constraintList_, own_A_);
   pop();
}

void RajaConstrainedOperator::Setup(raja::device device_,
                                    mfem::Operator *A_,
                                    const mfem::Array<int> &constraintList_,
                                    bool own_A_)
{
   push();
   device = device_;
   A = A_;
   own_A = own_A_;
   constraintIndices = constraintList_.Size();
   constraintList = constraintList_.Get_PArray()->As<Array>().RajaMem();
   pop();
}

void RajaConstrainedOperator::EliminateRHS(const Vector &x, Vector &b) const
{
   push();
   //const std::string &okl_defines = InLayout_().RajaEngine().GetOklDefines();
   //raja::kernel mapDofs = mapDofBuilder.build(device, okl_defines);
   w.Fill<double>(0.0);
   if (constraintIndices)
   {
      //assert(false);
      vector_map_dofs(constraintIndices,
                      (double*)w.RajaMem().ptr(),
                      (double*)x.RajaMem().ptr(),
                      (int*)constraintList.ptr());
   }
   A->Mult(mfem_w, mfem_z);
   b.Axpby<double>(1.0, b, -1.0, z);
   if (constraintIndices)
   {
      //mapDofs(constraintIndices, b.RajaMem(), x.RajaMem(), constraintList);
      vector_map_dofs(constraintIndices,
                      (double*)b.RajaMem().ptr(),
                      (double*)x.RajaMem().ptr(),
                      (int*)constraintList.ptr());
   }
   pop();
}

void RajaConstrainedOperator::Mult_(const Vector &x, Vector &y) const
{
   push();
   mfem::Vector mfem_y(y);
   if (constraintIndices == 0)
   {
      A->Mult(x.Wrap(), mfem_y);
      return;
   }

   //const std::string &okl_defines = InLayout_().RajaEngine().GetOklDefines();
   //::raja::kernel mapDofs   = mapDofBuilder.build(device, okl_defines);
   //::raja::kernel clearDofs = clearDofBuilder.build(device, okl_defines);

   z.Assign<double>(x); // z = x

   //assert(false);
   //clearDofs
   vector_clear_dofs(constraintIndices, (double*)z.RajaMem().ptr(),
                     (int*)constraintList.ptr());

   A->Mult(mfem_z, mfem_y);

   //assert(false);
   //mapDofs
   vector_map_dofs(constraintIndices,
                   (double*)y.RajaMem().ptr(),
                   (double*)x.RajaMem().ptr(),
                   (int*)constraintList.ptr());
   pop();
}

RajaConstrainedOperator::~RajaConstrainedOperator()
{
   if (own_A)
   {
      delete A;
   }
}

} // namespace mfem::raja

} // namespace mfem

#endif // defined(MFEM_USE_BACKENDS) && defined(MFEM_USE_RAJA)