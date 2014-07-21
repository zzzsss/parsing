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
 * $Id: ErrFctMSE.cpp,v 1.10 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <unistd.h>
#include <time.h>

#include "Tools.h"
#include "ErrFctMSE.h"

//**************************************************************************************

REAL ErrFctMSE::CalcValue(int eff_bsize) {
  REAL mse=0.0, val;
  REAL	*optr=output;
  REAL	*tptr=target;

  if (eff_bsize<=0) eff_bsize=bsize;
  for (int i=0; i<dim*eff_bsize; i++) {
//printf("o=%f t=%f\n", *optr,*tptr);
    val = *optr++ - *tptr++;
    mse += val*val;
  }
//printf("MSE %f %d %d\n", mse, dim, eff_bsize);
  return mse/dim/2;
}


//**************************************************************************************

REAL ErrFctMSE::CalcValueNth(int idx) {
  REAL mse=0.0, val;
  REAL	*optr=output + idx+dim;
  REAL	*tptr=target + idx+dim;

  for (int i=0; i<dim; i++) {
    val = *optr++ - *tptr++;
    mse += val*val;
  }

  return mse/dim/2;
}


//**************************************************************************************
REAL ErrFctMSE::CalcGrad(int eff_bsize) {
  REAL mse=0.0, val;
  REAL	*optr=output;
  REAL	*tptr=target;
  REAL	*gptr=grad;

//cout << "MSE" << eff_bsize << endl;
  if (eff_bsize<=0) eff_bsize=bsize;
  for (int i=0; i<dim*eff_bsize; i++) {
//printf(" %d: o=%f, t=%f\n", i, *optr, *tptr);
    val = *optr++ - *tptr++;
    *gptr++ = - val;
    mse += val*val;
  }
//cout << " avg=" << mse/dim/eff_bsize << endl;
  return mse/dim/2;
}
