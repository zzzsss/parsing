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
 * $Id: DataFile.cpp,v 1.16 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <stdlib.h>

#include "Tools.h"
#include "Data.h"
#include "DataFile.h"

DataFile::DataFile(char *p_path_prefix, ifstream &ifs, DataFile *prev_df)
 : idim(0), odim(0), nbex(0), resampl_coeff(1.0), path_prefix(p_path_prefix), fname(NULL),
   idx(-1), input(NULL), target_vect(NULL), target_id(0)
{
  char p_fname[DATA_LINE_LEN];
  
  ifs >> p_fname;
  if (prev_df) resampl_coeff=prev_df->resampl_coeff;
          else ifs >> resampl_coeff;	// read resampl coeff
  if (resampl_coeff<=0 || resampl_coeff>1)
    Error("resampl coefficient must be in (0,1]\n");
  fname=strdup(p_fname);

  // memory allocation of input and target_vect should be done in subclass
  // in function of the dimension and number of examples
}

DataFile::DataFile(char *p_path_prefix, char *p_fname, const float p_rcoeff)
 : idim(0), odim(0), nbex(0), resampl_coeff(p_rcoeff), path_prefix(p_path_prefix), fname(strdup(p_fname)),
   idx(-1), input(NULL), target_vect(NULL), target_id(0)
{

  // memory allocation of input and target_vect should be done in subclass
  // in function of the dimension and number of examples
}

DataFile::~DataFile()
{
  if (fname) free(fname);
  // memory deallocation of input and target_vect should be done in subclass
 
}

/**************************
 *  
 **************************/

int DataFile::Info(const char *txt)
{
  ulong nbr=resampl_coeff*nbex;
  printf("%s%s  %6.4f * %13lu = %13lu\n", txt, fname, resampl_coeff, nbex, nbr);
  return nbr;
}

/**************************
 *  
 **************************/

void DataFile::Rewind()
{
  Error("DataFile::Rewind() should be overriden");
}

//*************************
// read next data in File
// Return false if EOF

bool DataFile::Next()
{
  Error("DataFile::Next() should be overriden");
  return false;
}

//**************************
// generic resampling function using sequential file reads
// cycles sequentially through data until soemthing was found
// based on DataNext() which may be overriden by subclasses
// returns idx of current example

int DataFile::Resampl()
{
  bool ok=false;

  while (!ok) {
    if (!Next()) Rewind(); // TODO: deadlock if file empty
//cout << "Resampled: ";
//for (int i=0; i<idim; i++) cout << input[i] << " ";
    ok = (drand48() < resampl_coeff);
//cout << " ok=" << ok << endl;
  }
 
  return idx;
}

