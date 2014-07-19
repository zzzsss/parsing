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
 * $Id: MachSplit.cpp,v 1.47 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>

#include "Tools.h"
#include "MachSplit.h"
//#include "Gpu.cuh"

void MachSplit::do_alloc()
{
#ifdef BLAS_CUDA
  cudaSetDevice(cuda_dev);
  grad_in = cuda_alloc(idim*bsize, "input gradient in split machine");

  int dev_count = (cuda_devs.back() + 1);
  cudaGetDeviceCount(&dev_count);
  gpu_dev_data_in.reserve(dev_count);
  gpu_dev_grad_in.reserve(dev_count);
  gpu_dev_data_in.resize(dev_count, NULL);
  gpu_dev_grad_in.resize(dev_count, NULL);

  // allocate local grad_in and data_in buffers for each GPU
  printf("#### GPU allocate local data for %lu GPU\n", cuda_devs.size());
  for (uint i=0; i<cuda_devs.size(); i++) {
    int dev = cuda_devs[i];
    if (dev == cuda_dev) {
      // no local buffer copy on master GPU
      printf("#### GPU %d: use local data_in from MachSplit\n", cuda_dev);
      gpu_dev_grad_in[dev] = grad_in;
    }
    else {
      cudaSetDevice(dev);
      gpu_dev_data_in[dev] = cuda_alloc(idim*bsize*sizeof(REAL), "MachSplit: GPU local data_in");
      printf("#### GPU %d: allocate local data_in=%p\n", dev, gpu_dev_data_in[dev]);
      gpu_dev_grad_in[dev] = cuda_alloc(idim*bsize*sizeof(REAL), "MachSplit::Backw tmp for AXPY");
    }
  }
#else
  grad_in = (idim*bsize>0) ? new REAL[idim*bsize] : NULL;
#endif
}

void MachSplit::do_delete()
{
#ifdef BLAS_CUDA
  if (grad_in) cublasFree(grad_in);

    // free local data_in and grad_in buffers on each GPU
  for (vector<REAL*>::iterator it=gpu_dev_data_in.begin(); it!=gpu_dev_data_in.end(); ++it)
    if (*it) cudaFree(*it);
  gpu_dev_data_in.clear();
  for (vector<REAL*>::iterator it=gpu_dev_grad_in.begin(); it!=gpu_dev_grad_in.end(); ++it)
    if (*it) cudaFree(*it);
  gpu_dev_grad_in.clear();
#else
  if (grad_in) delete [] grad_in;
#endif
  grad_in = NULL;
}

MachSplit::MachSplit()
 : MachMulti()
{
  data_out=grad_out=NULL;	// important to prevent freeing!
}

MachSplit::MachSplit(const MachSplit &m)
 : MachMulti(m)
{
  data_out=grad_out=NULL;   // important to prevent freeing!
}

MachSplit::~MachSplit()
{
  do_delete();
}

MachSplit *MachSplit::Clone()
{
  MachSplit *m = new MachSplit(*this);
  if (m != NULL)
    m->CloneSubmachs(*this);
  return m;
}
 
void MachSplit::MachAdd(Mach *new_mach)
{

    // REMARK: there is no common output layer no output gradient !!
    //         input gradient is cumulated
    //         input data point to same memory

  if (machs.empty()) {
    machs.push_back(new_mach);
	// think about freeing memory
    idim=new_mach->GetIdim();
    odim=new_mach->GetOdim();
    bsize=new_mach->GetBsize();
    data_in=NULL; // will be set by MachSplit::SetDataIn()
    new_mach->SetDataIn(data_in);
    new_mach->SetGradOut(NULL); // must be done by Trainer()

    do_alloc();
  }
  else {
    if (bsize!=new_mach->GetBsize())
      Error("bunch size of new split machine does not match");
    if (idim!=new_mach->GetIdim())
      Error("input dimension of new split machine does not match");
    machs.push_back(new_mach);
 
      // resize output, we just change odim, no allocation is done since outputs are individual
      // idim does not change !
    odim += new_mach->GetOdim();
#ifdef BLAS_CUDA
    int dev = new_mach->GetCudaDevice();
    if (dev == cuda_dev)
      new_mach->SetDataIn(data_in);   // master GPU is locally chained
    else
      new_mach->SetDataIn(gpu_dev_data_in[dev]);  // remote GPU has its own copy of data_in (data will be transfered by Forw())
#else
    new_mach->SetDataIn(data_in);
#endif
    new_mach->SetGradOut(NULL); // must be done by Trainer!
  }

  activ_forw.push_back(true);
  activ_backw.push_back(true);
}


Mach *MachSplit::MachDel()
{
  if (machs.empty()) {
    Error("impossible to delete element from split machine: is already empty");
  }
  
  Mach *del_mach=machs.back();
  machs.pop_back();

  if (machs.empty()) {
    idim=odim=bsize=0;
    do_delete();
    data_in = NULL;
  }
  else {
      // resize output
    odim -= del_mach->GetOdim();
  }

  activ_forw.pop_back();
  activ_backw.pop_back();

  del_mach->SetDataIn(NULL);
  return del_mach;
}

// set pointer of input data
void MachSplit::SetDataIn(REAL *data)
{
  data_in=data;
    // all machines point on the same input
#ifdef BLAS_CUDA
  if (cuda_devs.size()==1) { // only one GPU device
    printf("#### CUDA set data_in for one GPU\n");
    for (uint m=0; m<machs.size(); m++) {
      machs[m]->SetDataIn(data_in);
    }
  }
  else {	// multiple GPU devices (no need to use cudaSetDevice() here since we manipulate CPU classes)
    printf("#### CUDA set data_in for %lu GPU\n", cuda_devs.size());
    for (uint m=0; m<machs.size(); m++) {
      int d = machs[m]->GetCudaDevice();
      if (d == cuda_dev)
        machs[m]->SetDataIn(data_in);	// master GPU is locally chained
      else
        machs[m]->SetDataIn(gpu_dev_data_in[d]);  // remote GPU has its own copy of data_in (data will be transfered by Forw())
      printf("### - mach %d is on GPU %d with input %p\n", m, d, machs[m]->GetDataIn());
    }
  }
#else
  for (uint m=0; m<machs.size(); m++){
    machs[m]->SetDataIn(data_in);
  }
#endif
}

// set pointer of output gradient for a particular machine
void MachSplit::SetGradOut(REAL *g, int mid)
{
  if (mid<0 || mid>=(int) machs.size())
    Error("MachSplit::SetGradOut() the specified machine does not exist");
  machs[mid]->SetGradOut(g); 
}

// get pointer to output data of a particular machine
REAL* MachSplit::GetDataOut(int mid)
{
  if (mid<0 || mid>=(int) machs.size())
    ErrorN("MachSplit::GetDataOut() the specified machine %i does not exist."
           " There is %zd machine(s)", mid, machs.size());
  return machs[mid]->GetDataOut(); 
}

//-----------------------------------------------
// File input
//-----------------------------------------------


void MachSplit::ReadData(ifstream &inpf, size_t s, int bs)
{
#ifdef BLAS_CUDA
  if (s!=machs.size()) {
    cerr << "ERROR: data block of split machine has " << s << " machines (" << machs.size() << " were expected)" << endl;    Error();
  }

  odim=0;
  size_t stDevIndex = 0;
  for (vector<Mach*>::iterator it = machs.begin(); it!=machs.end(); ++it, stDevIndex++) {
    cudaSetDevice(cuda_devs[stDevIndex % cuda_devs.size()]);
    (*it) = Mach::Read(inpf, bs);
    odim += (*it)->GetOdim();
  }
#else
  MachMulti::ReadData(inpf, s, bs);

     // get dimensions
  odim=0;
  for (uint m=0; m<machs.size(); m++) odim += machs[m]->GetOdim();
#endif
  idim = machs[0]->GetIdim();
  bsize = machs[0]->GetBsize();

    // allocate memory
  do_delete();
  do_alloc();
#ifdef BLAS_CUDA
  if (cuda_devs.size() > 1) cudaSetDevice(cuda_dev);
#endif

  for (uint m=0; m<machs.size(); m++) {
    machs[m]->SetDataIn(NULL);	// will be set before first Forw()
    machs[m]->SetGradOut(NULL);	// must be done by Trainer()!
  }
}

//
// Tools
//

void MachSplit::Info(bool detailed, char *txt)
{
  if (detailed) {
    cout << "Information on split machine" << endl;
    MachMulti::Info(detailed);
  }
  else {
    printf("%sSplit machine %d-%d, bs=%d, passes=%lu/%lu", txt, idim, odim, bsize, nb_forw, nb_backw);
    tm.disp(", ");
    tbackw.disp(" + backw: ");
    tbackw.newline();
    char ntxt[512];
    sprintf(ntxt,"%s  ", txt);
    for (unsigned int i=0; i<machs.size(); i++) machs[i]->Info(detailed, ntxt);
  }
  printf("%stotal number of parameters: %d (%d MBytes)\n", txt, GetNbParams(), GetNbParams()*(int) sizeof(REAL)/1048576);
}


// forward pass for all machines
//
// strategy for multiple GPUs
//  - there is one master GPU which cumulates all the information
//  - the machines in the split machine are sequentially distributed over multiple GPUs
//    gpu = mach_id modulo nb_gpu ; the fist GPU is the master
//  - IT IS ASSUMED THAT THE ERROR FUNCTIONS ARE LOCAL TO EACH GPU.
//    this must be ensured by the Trainer !
//  - the input-data of the master GPU is chained as usual to preceding machines
//  - the other GPUs have allocated their own input-data
//    the master GPU copys the data to this local buffers
//    there is only one buffer even when a GPU processes multiple parts of the split machine !
//
void MachSplit::Forw(int eff_bsize)
{
  if (machs.empty())
    Error("called Forw() for an empty split machine");

  debugMachInp("MachSplit",data_in,idim,odim,eff_bsize);

  tm.start();

  if (eff_bsize<=0) eff_bsize=bsize;

#ifdef BLAS_CUDA
    // copy the current input data to the other GPU devices
  if (cuda_devs.size() > 1)
    for (uint i=0; i<cuda_devs.size(); i++) {
      int d = cuda_devs[i];
      if (NULL != gpu_dev_data_in[d]) {
        cudaMemcpy(gpu_dev_data_in[d], data_in, idim*eff_bsize*sizeof(REAL), cudaMemcpyDeviceToDevice);	// CUDA knows the device by the unified address space
      }
    }
#endif

  for (unsigned int m=0; m<machs.size(); m++) {
    if (activ_forw[m]) {
      machs[m]->Forw(eff_bsize);
        // its the responsibility of the Trainer to collect the individual outputs
    }
    else {
      Error("  MachSplit: forw deactivated\n");
    }
  }
#ifdef BLAS_CUDA
  if (cuda_devs.size()>1) cudaSetDevice(cuda_dev);	// reset to master GPU
#endif
  nb_forw += eff_bsize;

  tm.stop();

  debugMachOutp("MachSplit",data_out,idim,odim,eff_bsize);
}

// backward pass for all machines and cumulate gradient at input
void MachSplit::Backw(const float lrate, const float wdecay, int eff_bsize)
{
  if (machs.empty())
    Error("called Backw() for an empty split machine");

  if (eff_bsize<=0) eff_bsize=bsize;

  debugMachOutp("MachSplit Grad",grad_out,idim,odim,eff_bsize);

  tbackw.start();

     // verify that the output gradients of the individual machines were set by the Trainer
  for (unsigned int m=0; m<machs.size(); m++) {
    if (! machs[m]->GetGradOut())
      Error("MachSplit::Backw() the gradient of machine %d is not set", (int) m);
  }

#ifdef BLAS_CUDA
  REAL * last_grad_in = NULL; // last grad_in buffer used
#endif

   // backward 1st machine
  if (activ_backw[0]) {
    machs[0]->Backw(lrate,wdecay,eff_bsize);
#ifdef BLAS_CUDA
    last_grad_in = machs[0]->GetGradIn();
    cuda_check_error("MachSplit::Backw after 1st mach");
#else
    memcpy(grad_in, machs[0]->GetGradIn(), idim*eff_bsize*sizeof(REAL));
#endif
  }
  else {
      // clear the gradient so we can cumulate the following ones
#ifdef BLAS_CUDA
    cudaMemset(grad_in, 0.0, idim*eff_bsize*sizeof(REAL));
    last_grad_in = grad_in;
#else
    memset(grad_in, 0.0, idim*eff_bsize*sizeof(REAL));
#endif
  }

     // backward following machines, add gradient on existing ones
#ifdef BLAS_CUDA
  int size = idim*eff_bsize;
  for (unsigned int m=1; m<machs.size(); m++) {
    if (activ_backw[m]) {
      machs[m]->Backw(lrate,wdecay,eff_bsize);
      cuda_check_error("MachSplit::Backw after following mach");
      REAL * grad_ptr = machs[m]->GetGradIn();
      REAL onef = 1.f;
      int one = 1;
      int dev = machs[m]->GetCudaDevice();
      if (gpu_dev_grad_in[dev] != last_grad_in) {
        cudaMemcpy(gpu_dev_grad_in[dev], last_grad_in, size*sizeof(REAL), cudaMemcpyDeviceToDevice);
        last_grad_in = gpu_dev_grad_in[dev];
      }
      AXPY(size, onef, grad_ptr, one, gpu_dev_grad_in[dev], one);
      cuda_check_error("MachSplit::Backw after following mach AXPY");
    }
    else {
    }
  }
  if (grad_in != last_grad_in)
    cudaMemcpy(grad_in, last_grad_in, size*sizeof(REAL), cudaMemcpyDeviceToDevice);

  if (cuda_devs.size()>1) cudaSetDevice(cuda_dev);
  cuda_check_error("MachSplit::Backw end");

#else
  for (unsigned int m=1; m<machs.size(); m++) {
    if (activ_backw[m]) {
      machs[m]->Backw(lrate,wdecay,eff_bsize);
      REAL * grad_ptr = machs[m]->GetGradIn();
      int size = idim*eff_bsize;
      REAL onef = 1.f;
      int one = 1;
      AXPY(&size, &onef, grad_ptr, &one, grad_in, &one);
    }
    else {
    }
  }
#endif

  nb_backw += eff_bsize; 
  tbackw.stop();

  debugMachInp("MachSplit Grad",grad_in,idim,odim,eff_bsize);
}

