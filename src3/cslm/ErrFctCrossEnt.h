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
 * $Id: ErrFctCrossEnt.h,v 1.8 2014/03/25 21:52:53 schwenk Exp $
 *
 * Class definition of cross entropy error function
 *   E = sum_i  d_i * ln o_i
 *   dE/do_k = d_k / o_k   for o_k <> 0
 * This is usually used with softmax outputs
 */

#ifndef _ErrFctCrossEnt_h
#define _ErrFctCrossEnt_h

#include <iostream>
#include "Tools.h"
#include "ErrFct.h"


class ErrFctCrossEnt : public ErrFct
{
public:
  ErrFctCrossEnt(Mach &mach) : ErrFct(mach) {};
  virtual REAL CalcValue(int=0);		// Calculate value of error function
  virtual REAL CalcValueNth(int);		// Calculate value of error function for a particular example in bunch
  virtual REAL CalcGrad(int=0);			// calculate NEGATIF gradient of error function
};

#endif
