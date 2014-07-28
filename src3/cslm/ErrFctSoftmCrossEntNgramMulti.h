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
 * $Id: ErrFctSoftmCrossEntNgramMulti.h,v 1.5 2014/03/25 21:52:53 schwenk Exp $
 *
 * Class definition of cross entropy error function
 * Special version for NNs that predict MULTIPLE words
 *  - the NN has a large output dimension (vocsize or limited to shortlist)
 *  - the data has one dimensional targets that are taken as index into
 *  	the word list
 *  - therefore the target vector is binary: 1 at the position of the to predicted
 *  	word, 0 elsewhere
 *
 *   E = sum_i  d_i * ln o_i
 *   dE/do_k = d_k / o_k   for o_k <> 0
 * This is usually used with softmax outputs
 */

#ifndef _ErrFctSoftmCrossEntNgramMulti_h
#define _ErrFctSoftmCrossEntNgramMulti_h

#include <iostream>
#include "Tools.h"
#include "ErrFct.h"
#include "ErrFctSoftmCrossEntNgram.h"
#ifdef BLAS_CUDA
#  include "Gpu.cuh"
#endif


class ErrFctSoftmCrossEntNgramMulti : public ErrFctSoftmCrossEntNgram
{
private:
  int nb;		// number of separate output n-gram each of dimension "dim"
			// -> therefore the total size of the gradient is nb*dim !!
public:
  ErrFctSoftmCrossEntNgramMulti(Mach &mach, int n);
  virtual REAL CalcValue(int=0);	// Calculate value of error function
  virtual REAL CalcValueNth(int);	// Calculate value of error function for a particular example in bunch
  virtual REAL CalcGrad(int=0);		// calculate NEGATIF gradient of error function
#ifdef BLAS_CUDA
  virtual void InitGradCumul() { GpuResSet(0.0); };
  virtual REAL GetGradCumul() { return GpuResGet(); };
#endif
};

#endif
