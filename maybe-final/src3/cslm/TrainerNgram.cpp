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
 * $Id: TrainerNgram.cpp,v 1.32 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <algorithm>

#include "Mach.h"
#include "TrainerNgram.h"

TrainerNgram::TrainerNgram (Mach *pmach, ErrFct *perrfct,
	char *train_fname, char *dev_fname,
	REAL p_lr_beg, REAL p_lr_mult, REAL p_wd,
	int p_maxep, int p_ep)
 : Trainer(pmach,perrfct,NULL,NULL,p_lr_beg,p_lr_mult,p_wd,p_maxep,p_ep),
   order(0)
{
  char	msg[1024];

  idim=mach->GetIdim(); odim=mach->GetOdim(); bsize=mach->GetBsize();

  if (odim < 16) {
    sprintf(msg,"TrainerNgram: output dimension of the machine is suspiciously small (%d)\n", odim);
    //Error(msg);
    printf(msg); // TODO:
  }

  if (train_fname) {
    data_train = new Data(train_fname);
    if (idim != data_train->GetIdim()) {
      sprintf(msg,"TrainerNgram: input dimension of the training data (%d) does not match the one of the machine (%d)\n", data_train->GetIdim(), idim);
      Error(msg);
    }
    if (data_train->GetOdim() != 1) {
      sprintf(msg,"TrainerNgram: output dimension of the training data should be 1, found %d\n", data_train->GetOdim());
      Error(msg);
    }
  }
  else 
    data_train=NULL;

  if (dev_fname) {
    data_dev = new Data(dev_fname, data_train);
    data_dev_alloc=true;
    if (idim != data_dev->GetIdim()) {
      sprintf(msg,"TrainerNgram: input dimension of the validation data (%d) does not match the one of the machine (%d)\n", data_dev->GetIdim(), idim);
      Error(msg);
    }
    if (data_dev->GetOdim() != 1) {
      sprintf(msg,"TrainerNgram: output dimension of the validation data should be 1, found %d\n", data_dev->GetOdim());
      Error(msg);
    }
  }
  else {
    data_dev=NULL;
    data_dev_alloc=false;
  }

  buf_target_wid = new WordID[odim*bsize];
}

TrainerNgram::TrainerNgram (Mach *pmach, ErrFct *perrfct, Data *data)
 : Trainer(pmach,perrfct,NULL,NULL,0,0,0,0,0),
    order(0)
{
  char	msg[1024];

  idim=mach->GetIdim(); odim=mach->GetOdim(); bsize=mach->GetBsize();

  if (odim < 16) {
    sprintf(msg,"TrainerNgram: output dimension of the machine is suspiciously small (%d)\n", odim);
    Error(msg);
  }

  data_train=NULL;
  data_dev=data;
  data_dev_alloc=false; // do not free it by this class !

  if (data_dev) {
    if (idim != data_dev->GetIdim()) {
      sprintf(msg,"TrainerNgram: input dimension of the validation data (%d) does not match the one of the machine (%d)\n", data_dev->GetIdim(), idim);
      Error(msg);
    }
    if (data_dev->GetOdim() != 1) {
      sprintf(msg,"TrainerNgram: output dimension of the validation data should be 1, found %d\n", data_dev->GetOdim());
      Error(msg);
    }
  }

  buf_target_wid = new WordID[odim*bsize];
}

TrainerNgram::~TrainerNgram()
{
  delete [] buf_target_wid;
}

//**************************************************************************************

REAL TrainerNgram::Train()
{
  if (!data_train) return -1;
#ifdef DEBUG
  printf("*****************\n");
  printf("TrainerNgram::Train():\n");
  printf(" -  data_in: %p \n", (void*) buf_input);
  printf(" -   target: %p \n", (void*) buf_target);
  printf(" - grad_out: %p \n", (void*) errfct->GetGrad());
#endif

  data_train->Rewind();
  Timer ttrain;		// total training time
  ttrain.start();

  REAL log_sum=0;
  nb_ex=0;

#ifdef BLAS_CUDA
  mach->SetDataIn(gpu_input);		// we copy from buf_input to gpu_input
  errfct->SetTarget(gpu_target);	// we copy from buf_target to gpu_target
#else
  mach->SetDataIn(buf_input);
  errfct->SetTarget(buf_target);
#endif
  errfct->SetOutput(mach->GetDataOut());
  mach->SetGradOut(errfct->GetGrad());

  bool data_available;
  do {
      // get a bunch of data
      // TODO: exclude out of slist
    int n=0;
    data_available = true;
    while (n < mach->GetBsize() && data_available) {
      data_available = data_train->Next();
      if (!data_available) break;
      memcpy(buf_input  + n*idim, data_train->input, idim*sizeof(REAL));
      memcpy(buf_target + n*1, data_train->target, 1*sizeof(REAL));
      n++;
    }

    if (n>0) {
#ifdef BLAS_CUDA
      cudaMemcpy(gpu_input, buf_input , n*idim*sizeof(REAL), cudaMemcpyHostToDevice);
      cudaMemcpy(gpu_target, buf_target , n*1*sizeof(REAL), cudaMemcpyHostToDevice);
#endif
      mach->Forw(n); 
      log_sum += errfct->CalcGrad(n);
      SetLrate();
      mach->Backw(lrate, wdecay, n); 
    }

    nb_ex += n;
  } while (data_available);

  ttrain.stop();
  ttrain.disp(" - training time: ");
  printf("\n");
  
  if (nb_ex>0) return exp(-log_sum / (REAL) nb_ex);  // return perplexity
  
  if (nb_ex>0) return exp(-log_sum / (REAL) nb_ex);  // return perplexity

  return -1;
}

//**************************************************************************************
// This should be overriden to do a task-specific validation

REAL TrainerNgram::TestDev(char *fname)
{
  if (!data_dev) return -1;

  ofstream fs;
#ifdef BLAS_CUDA
    Error("Dumping probability stream into file is not yet implemented for GPU cards\n");
#else
  if (fname) {
    cout << " - dumping log probability stream to file '" << fname << "'" << endl;
    fs.open(fname,ios::out);
    CHECK_FILE(fs,fname);
  }
#endif

  int nb_ex_dev=0;
  REAL log_sum=0;
  data_dev->Rewind();

#ifdef BLAS_CUDA
  mach->SetDataIn(gpu_input);		// we copy from buf_input to gpu_input
  errfct->SetTarget(gpu_target);	// we copy from buf_target to gpu_target
#else
  mach->SetDataIn(buf_input);
  errfct->SetTarget(buf_target);
#endif
  errfct->SetOutput(mach->GetDataOut());

      // TODO: we could copy all the examples on the GPU and then split into bunches locally
  bool data_available;
  do {
      // get a bunch of data
      // TODO: exlude out of slist
    int n=0;
    data_available = true;
    while (n < mach->GetBsize() && data_available) {
      data_available = data_dev->Next();
      if (!data_available) break;
      memcpy(buf_input  + n*idim, data_dev->input, idim*sizeof(REAL));
      memcpy(buf_target + n*1, data_dev->target, 1*sizeof(REAL));
      n++;
    }

      // process the bunch
    if (n>0) {
#ifdef BLAS_CUDA
      cudaMemcpy(gpu_input, buf_input , n*idim*sizeof(REAL), cudaMemcpyHostToDevice);
      cudaMemcpy(gpu_target, buf_target , n*1*sizeof(REAL), cudaMemcpyHostToDevice);
#endif
      mach->Forw(n); 
      log_sum += errfct->CalcValue(n);
      if (fname) {
        for (int ni=0; ni<n; ni++)
          fs << errfct->CalcValueNth(n) << endl;	// this is slightly inefficient since we calculate twice the same thing
      }
    }

    nb_ex_dev += n;
  } while (data_available);

  if (fname) fs.close();

  REAL px = (nb_ex_dev>0) ? exp(-log_sum / (REAL) nb_ex_dev) : -1;
  printf(" - %d %d-gram requests, ln_sum=%.2f, overall px=%.2f\n", nb_ex, idim+1, log_sum, px);

  return px;
}


//**************************************************************************************
// information after finishing an epoch

void TrainerNgram::InfoPost ()
{
  cout << " - epoch finished, " << nb_ex << " examples seen, average perplexity: " << err_train << endl;
}

