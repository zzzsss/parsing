/*
 * This file is part of the continuous space language and translation model toolkit
 * for statistical machine translation and large vocabulary speech recognition.
 *
 * Copyright 2014, Holger Schwenk, LIUM, University of Le Mans, France
 *
 * The CSLM toolkit is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * $Id: ErrFct.cpp,v 1.11 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <unistd.h>
#include <time.h>

#include "Tools.h"
#include "ErrFct.h"
#include "Blas.h"

ErrFct::ErrFct (Mach &mach)
 : dim(mach.GetOdim()), bsize(mach.GetBsize()), 
   output(mach.GetDataOut()), target(NULL)
{
#ifdef BLAS_CUDA
  cuda_dev = mach.GetCudaDevice();
  cudaSetDevice(cuda_dev);
  grad = cuda_alloc(dim*bsize, "gradient in Error Function");
#else
  grad = new REAL[dim*bsize];
#endif
}

ErrFct::ErrFct (const ErrFct &efct)
 : dim(efct.dim), bsize(efct.bsize), 
   output(efct.output), target(efct.target)
{
#ifdef BLAS_CUDA
  cuda_dev = efct.cuda_dev;
  cudaSetDevice(cuda_dev);
  grad = cuda_alloc(dim*bsize, "gradient in Error Function");
#else
  grad = new REAL[dim*bsize];
#endif
}

//**************************************************************************************

REAL ErrFct::CalcValue(int eff_bsize) { return 0; }

REAL ErrFct::CalcValueNth(int idx) { return 0; }

REAL ErrFct::CalcGrad(int eff_bsize) {
  if (eff_bsize<=0) eff_bsize=bsize;
  for (int i=0; i<dim*eff_bsize; i++) grad[i]=0.0;
  return 0;
}
