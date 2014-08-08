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
 * $Id: DataFile.h,v 1.13 2014/03/25 21:52:53 schwenk Exp $
 */

#ifndef _DataFile_h
#define _DataFile_h

#include <iostream>
#include <fstream>
#include <vector>
#include "Tools.h"

class DataFile {
protected:
  int	idim, odim;
  ulong  nbex;
  float resampl_coeff;
    // internal handling of data
  char *path_prefix;	// prefix to be added for each data file which will be opened, points to text allocated in Data.h
  char *fname;
public:
    // current data
  ulong  idx;
  REAL *input;		// current input data (needs to be allocated in subclass, we don't know the dimension yet)
  REAL *target_vect;	//         output data
  int  target_id;	//         index of output [0..odim)
   // functions
  DataFile(char *, ifstream &, DataFile* =NULL);	// optional object to initialize when adding factors
  DataFile(char *, char *, const float =1.0);
  virtual ~DataFile();
   // access function
  int GetIdim() { return idim; }
  int GetOdim() { return odim; }
  ulong GetNbex() { return nbex; }
  ulong GetNbresampl() { return (ulong) (nbex*resampl_coeff); }
  float GetResampl() { return resampl_coeff; }
   // main interface
  virtual int Info(const char* = " - ");	// display line with info after loading the data
  virtual void Rewind();	// rewind to first element
  virtual bool Next();		// advance to next data
  virtual int Resampl();	// resample another data (this may skip some elements in the file)
};

#endif
