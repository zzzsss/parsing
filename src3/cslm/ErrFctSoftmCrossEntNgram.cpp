/*
 * This file is part of the continuous space language and translation model toolkit
 * for statistical machine translation and large vocabulary speech recognition.
 *
 * Copyright 2013, Holger Schwenk, LIUM, University of Le Mans, France
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
 * $Id: ErrFctSoftmCrossEntNgram.cpp,v 1.23 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <unistd.h>
#include <time.h>

#include "Tools.h"
#include "ErrFctSoftmCrossEntNgram.h"
#include "Blas.h"

ErrFctSoftmCrossEntNgram::ErrFctSoftmCrossEntNgram(Mach &mach)
 : ErrFct(mach)
{
#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  err = cuda_alloc(1, "ErrFctSoftmCrossEntNgram: err variable");
#endif
}

ErrFctSoftmCrossEntNgram::ErrFctSoftmCrossEntNgram(const ErrFctSoftmCrossEntNgram &efct)
 : ErrFct(efct)
{
#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  err = cuda_alloc(1, "ErrFctSoftmCrossEntNgram: err variable");
#endif
}

ErrFctSoftmCrossEntNgram::~ErrFctSoftmCrossEntNgram()
{
#ifdef BLAS_CUDA
  if(err) cudaFree(err);
#endif
}

//*********************************************************************************r
// E = log(sum_i d_i ln o_i)
//   = ln o_t     where t is the target index
//   output: dimension voc_size
//   target: dimension 1 with values [0,voc_size[
// We also take the log since this can't be done later if bsize>1

REAL ErrFctSoftmCrossEntNgram::CalcValue(int eff_bsize)
{

  if (eff_bsize<=0) eff_bsize=bsize;

#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  return GpuErrFctSoftmCrossEntNgramCalcValue(eff_bsize, dim, output, target);
#else
  REAL *tptr=target;
  REAL *optr=output;
  double lerr=0.0;

  for (int b=0; b<eff_bsize; b++) {
    //printf("b=%d, tidx=%f, out=%f\n", b, *tptr, optr[(uint) *tptr]);
    lerr += safelog(optr[(uint) *tptr++]);
    //printf("err=%f\n",lerr);
    optr += dim;
  }
  return (REAL) lerr; // TODO: normalize ?
#endif
}


//**********************************************************************************
// E = log(sum_i d_i ln o_i)
//   = ln o_t     where t is the target index
//   output: dimension voc_size
//   target: dimension 1 with values [0,voc_size[
// We also take the log since this can't be done later if bsize>1

REAL ErrFctSoftmCrossEntNgram::CalcValueNth(int idx)
{
#ifdef BLAS_CUDA
  Error("CUDA:  ErrFctSoftmCrossEntNgram::CalcValueNth() not implemented");
  cudaSetDevice(cuda_dev);
  return 0.0;
#else
  REAL	*optr=output + idx+dim;
  REAL	*tptr=target + idx+dim;

  return safelog(optr[(uint) *tptr]);
#endif
}


// We include here the derivation of the softmax outputs since we have
//   dE/da_k = sum_i dE/do_i do_i/da_k
// Due to the sum, dE/do_i and do_i/da_k can't be calculated separately
// dE/do_i = d_i/o_i
// do_i/da_k = o_i (kronecker_ik - o_k)
//  -> dE/da_k = sum_i d_i/o_i * o_i (kronecker_ik - o_k)
//             = sum_i d_i (kronecker_ik - o_k)
//             = (kronecker_tk - o_k)       since d_i=0 for i!=t

REAL ErrFctSoftmCrossEntNgram::CalcGrad(int eff_bsize)
{
  if (eff_bsize<=0) eff_bsize=bsize;

#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  GpuErrFctSoftmCrossEntNgramCalcGrad(eff_bsize, dim, output, grad, target, err);
  REAL res = 0;
  cudaMemcpy(&res, err, sizeof(REAL), cudaMemcpyDeviceToHost);
  return res;
#else

  REAL *optr=output;
  REAL *gptr=grad;
  REAL *tptr=target;
  uint	tidx;
  err=0.0;
  int n=eff_bsize*dim; REAL f1=-1.0;

  memcpy(grad,output,n*sizeof(REAL));
  SCAL(&n,&f1,grad,&inc1);
  for (int b=0; b<eff_bsize; b++) {
    if (*tptr<0.0) ErrorN("negative index %f at %d",*tptr,b);
    tidx=(uint) *tptr++;
    gptr[tidx] += 1.0;
    err += safelog(optr[tidx]);
    gptr+=dim; optr+=dim;
  }

  return err;
#endif
}


REAL ErrFctSoftmCrossEntNgram::CalcGradNull(int eff_bsize)
{
  if (eff_bsize<=0) eff_bsize=bsize;

#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  GpuErrFctSoftmCrossEntNgramCalcGradNull(eff_bsize, dim, output, grad, target, err);
  REAL res = 0;
  cudaMemcpy(&res, err, sizeof(REAL), cudaMemcpyDeviceToHost);
  return res;
#else

  REAL *optr=output;
  REAL *gptr=grad;
  REAL *tptr=target;
  int	tidx;
  err=0.0;
  int n=eff_bsize*dim; REAL f1=-1.0;

  memcpy(grad,output,n*sizeof(REAL));
  SCAL(&n,&f1,grad,&inc1);
  for (int b=0; b<eff_bsize; b++) {
    tidx=(uint) *tptr++;
    if (tidx==NULL_WORD) {
      memset(gptr, 0, dim*sizeof(REAL));
    }
    else {
      gptr[tidx] += 1.0;
      err += safelog(optr[tidx]);
    }
    gptr+=dim; optr+=dim;
  }

  return err;
#endif
}

