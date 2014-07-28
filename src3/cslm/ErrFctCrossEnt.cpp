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
 * $Id: ErrFctCrossEnt.cpp,v 1.10 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <unistd.h>
#include <time.h>

#include "Tools.h"
#include "ErrFctCrossEnt.h"

//**********************************************************************************
// E = sum_i d_i ln o_i
REAL ErrFctCrossEnt::CalcValue(int eff_bsize) {
  REAL	*optr=output;
  REAL	*tptr=target;
  double err=0.0;

  if (eff_bsize<=0) eff_bsize=bsize;
  for (int i=0; i<eff_bsize*dim; i++) {
      err += *tptr++ * log(*optr++);
  }
  return (REAL) err/dim;
}
//
//**********************************************************************************
// E = sum_i d_i ln o_i
REAL ErrFctCrossEnt::CalcValueNth(int idx) {
  REAL	*optr=output + idx+dim;
  REAL	*tptr=target + idx+dim;
  double err=0.0;

  for (int i=0; i<dim; i++) {
      err += *tptr++ * log(*optr++);
  }
  return (REAL) err/dim;
}


//**********************************************************************************
// dE / do_i = d_i / t_i
REAL ErrFctCrossEnt::CalcGrad(int eff_bsize) {
  REAL	*optr=output;
  REAL	*tptr=target;
  REAL	*gptr=grad;
  REAL err=0.0;

  if (eff_bsize<=0) eff_bsize=bsize;
  for (int i=0; i<eff_bsize*dim; i++) {
    *gptr++ = (*optr == 0) ? 0 : *tptr / *optr; // TODO
    err += *tptr++ * log(*optr++);
  }
  return (REAL) err/dim;
}
