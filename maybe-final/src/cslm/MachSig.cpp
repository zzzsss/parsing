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
 * $Id: MachSig.cpp,v 1.25 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <math.h>

#include "Tools.h"
#include "MachSig.h"

MachSig::MachSig(const int p_idim, const int p_odim, const int p_bsize, const ulong p_nbfw, const ulong p_nbbw)
 : MachLin(p_idim, p_odim, p_bsize, p_nbfw, p_nbbw)
{
}

MachSig::MachSig(const MachSig &m)
 : MachLin(m)
{
}

MachSig::~MachSig()
{
  printf("** destructor MachSig %lx\n",(luint) this);
}

//-----------------------------------------------
// Tools
//-----------------------------------------------

void MachSig::Info(bool detailed, char *txt)
{
  if (detailed) {
    cout << "Information on sigmoidal machine" << endl;
    MachLin::Info(detailed,txt);
  }
  else {
    if (drop_out>0)
      printf("%sMachSig %d-%d, bs=%d, drop-out=%4.2f, passes=%lu/%lu", txt, idim, odim, bsize, drop_out, nb_forw, nb_backw);
    else
      printf("%sMachSig %d-%d, bs=%d, passes=%lu/%lu", txt, idim, odim, bsize, nb_forw, nb_backw);
#ifdef BLAS_CUDA
    printf(", on GPU %d", cuda_dev);
#endif
    tm.disp(", ");
    printf("\n");
  }
}

//-----------------------------------------------
// Training
//-----------------------------------------------

void MachSig::Forw(int eff_bsize)
{

  tm.start();

  if (eff_bsize<=0) eff_bsize=bsize;
  MachLin::Forw(eff_bsize);

    // apply sigmoid on output
  Error("implement sigmoid\n");

  tm.stop();
}

void MachSig::Backw(const float lrate, const float wdecay, int eff_bsize)
{
    // derivate sigmoidal activation function
    //             = grad_hidden .* ( 1 - a_hidden^2 )

  REAL *aptr = data_out;
  REAL *gptr = grad_out;

  if (eff_bsize<=0) eff_bsize=bsize;
  if (!grad_out)
    Error("MachSig::Backw(): output gradient is not set");

  tm.start();

  for (int i=0; i<odim*eff_bsize; i++) {
    REAL val = *aptr++;
    Error("implement derivative of sigmoid\n");
    *gptr=val;
  }

  tm.stop();
  MachLin::Backw(lrate, wdecay, eff_bsize);
}

