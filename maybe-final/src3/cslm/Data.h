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
 * $Id: Data.h,v 1.20 2014/03/25 21:52:53 schwenk Exp $
 */

#ifndef _Data_h
#define _Data_h

#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>
#include "Tools.h"
#include "DataFile.h"
#include "WordList.h"

// Names of information in files

#define DATA_LINE_LEN 16384   // extern const int gives internal gcc error for 4.7.2
extern const char* DATA_HEADER_TXT;
extern const int   DATA_HEADER_ID;
extern const char* DATA_PRELOAD;
extern const char* DATA_RESAMPL_MODE;
extern const char* DATA_RESAMPL_SEED;
extern const char* DATA_SHUFFLE_MODE;
extern const char* DATA_PATH_PREFIX;
extern const char* DATA_NB_FACTORS;
extern const char* DATA_WORD_LIST;
extern const char* DATA_WORD_LIST_TARGET;
extern const char* DATA_WORD_LIST_SOURCE;

/*
 * Strategy
 *  - there is one function Rewind() and Next() which should not be overridden
 *  - they perform all the processing with preloading, shuffling, etc
 *  - the class specific processing is done in First() and Advance()
 */

typedef vector< vector<DataFile*> > FactoredDataFiles;

class Data
{
private:
  void CreateWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im, ifstream &ifs);
  static void CompareWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im, vector<WordList> *ew, int *es, pthread_mutex_t *em);
  static void DeleteWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im);
protected:
  const char *fname;
  char *path_prefix;	// prefix added to all file names
  int  idim, odim;	// dimensions
  int  nb_totl;		// number of examples
    // flags
  int preload;		// 
  int resampl_mode;	// 
  int resampl_seed;	// 
  int shuffle_mode;	// 
  int norm_mode;	// evtl. perform normalization; bits: 1=subtract mean, 2=divide by var.
  int nb_factors;	// 

    // word lists
  vector<WordList> *sr_wlist, *tg_wlist;
  int *sw_shared, *tw_shared;			// number of objects sharing word list
  pthread_mutex_t *sw_mutex, *tw_mutex;	// mutex used to share word list
    // data files
  FactoredDataFiles  	datafile;
    // actual data
  int  idx;		// index of current example [0,nb-1]
  REAL *mem_inp;	// all the input data in memory
  REAL *mem_trg;	// all the output data in memory
    // local tools, only used when preload is activated
  void Preload();	// preload all data
  void Shuffle();	// shuffle in memory
public:
  Data(const char *fname, Data *other_data = NULL);
  ~Data();
    // access function to local variables
  const char *GetFname() {return fname;}
  int GetIdim() {return idim;}
  int GetOdim() {return odim;}
  int GetNb() {return nb_totl;}
  int GetIdx() {if (idx<0) Error("DataNext() must be called before GetIdx()"); return idx;};
  vector<WordList> *GetSrcWList() {return sr_wlist;}
  vector<WordList> *GetTgtWList() {return tg_wlist;}
    // the following two pointers are only valid after first DataNext() !
  REAL *input;		// pointer to current inputs
  REAL *target;		// pointer to current target
  //REAL *GetData() {return val;}
    // main functions to access data
  void Rewind();	// rewind to first example, performs resmplaing, shuffling etc if activated
  bool Next();		// advance to next example, return FALSE if at end
};

#endif
