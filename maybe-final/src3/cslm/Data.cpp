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
 * $Id: Data.cpp,v 1.33 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>
#include <cstdlib>
#include <cstring>

#include "Tools.h"
#include "Data.h"
#include "DataAscii.h"
#include "DataAsciiClass.h"
#include "DataMnist.h"
#include "DataNgramBin.h"
#include "DataPhraseBin.h"
#include <ctime>

const char* DATA_HEADER_TXT="DataDescr";
const int   DATA_HEADER_ID=3;
const char* DATA_PRELOAD="Preload";
const int   DATA_PRELOAD_ACT=8;		// preloading is activated, additional flags:
const int   DATA_PRELOAD_TODO=1;	//    mark that preloading was not yet done, we use this to avoid multiple (costly) preloading
					//    this flag is set by Next() -> new Rewind() triggers resampling
const int   DATA_PRELOAD_ONCE=4;	//    we resample only once, even if rewind is called many times
const char* DATA_RESAMPL_MODE="ResamplMode";
const char* DATA_RESAMPL_SEED="ResamplSeed";
const char* DATA_SHUFFLE_MODE="ShuffleMode";
const char* DATA_NORMALIZE_MODE="Normalize";
const char* DATA_PATH_PREFIX="PathPrefix";
const char* DATA_NB_FACTORS="NumberFactors";
const char* DATA_WORD_LIST="WordList";
const char* DATA_WORD_LIST_TARGET="WordListTarget";
const char* DATA_WORD_LIST_SOURCE="WordListSource";

/**************************
 *
 **************************/

Data::Data(const char *p_fname, Data *other_data)
 : fname(p_fname), path_prefix(NULL), idim(0), odim(0), nb_totl(0),
   preload(0), resampl_mode(0), resampl_seed(1234567890), shuffle_mode(0), norm_mode(0), nb_factors(1),
   sr_wlist(NULL), tg_wlist(NULL), sw_shared(NULL), tw_shared(NULL), sw_mutex(NULL), tw_mutex(NULL),
   idx(-1), mem_inp(NULL), mem_trg(NULL), input(NULL), target(NULL)
{

  cout << "Opening data description '" << fname << "'" << endl;
  ifstream ifs;
  ifs.open(fname,ios::in);
  CHECK_FILE(ifs,fname);

    // parsing data description
  int i=ReadInt(ifs,DATA_HEADER_TXT);
  if (i>DATA_HEADER_ID) Error("unknown data description header\n");

  while (!ifs.eof()) {
    bool ok=false;
    vector<DataFile*> factored_df;

    string buf; char line[DATA_LINE_LEN];
    ifs >> buf;
    if (buf[0]=='#') {ifs.getline(line, DATA_LINE_LEN); continue;} // skip comments
    if (buf=="") break; // HACK
    if (buf==DATA_PRELOAD) { preload=DATA_PRELOAD_ACT | DATA_PRELOAD_TODO; ok=true; }
    if (buf==DATA_RESAMPL_MODE) { ifs >> resampl_mode; ok=true; }
    if (buf==DATA_RESAMPL_SEED) { ifs >> resampl_seed; ok=true; }
    if (buf==DATA_SHUFFLE_MODE) { ifs >> shuffle_mode; ok=true; }
    if (buf==DATA_NORMALIZE_MODE) { ifs >> norm_mode; ok=true; }
    if (buf==DATA_PATH_PREFIX) {
      string tmp;
      ifs >> tmp; ok=true;
      cout << "Prefix for all data files: " << tmp << endl;
      path_prefix=strdup(tmp.c_str()); // ugly
    }
    if (buf==DATA_NB_FACTORS) {
      ifs >> nb_factors;
      if (nb_factors<1) Error("The number of factors must be at least one");
      ok=true;
    }
    if (buf==DATA_WORD_LIST_SOURCE) {
      CreateWordList(sr_wlist, sw_shared, sw_mutex, ifs);
      ok=true;
    }
    if (   (buf==DATA_WORD_LIST       )
        || (buf==DATA_WORD_LIST_TARGET) ) {
      CreateWordList(tg_wlist, tw_shared, tw_mutex, ifs);
      ok=true;
    }

    if (buf==DATA_FILE_ASCII) {
      factored_df.clear();
      factored_df.push_back(new DataAscii(path_prefix,ifs));
      for (int i=1; i<nb_factors; i++)
        factored_df.push_back(new DataAscii(path_prefix,ifs,(DataAscii*)factored_df[0]));
      datafile.push_back(factored_df);
      ok=true;
    }

    if (buf==DATA_FILE_ASCIICLASS) {
      factored_df.clear();
      factored_df.push_back(new DataAsciiClass(path_prefix,ifs));
      for (int i=1; i<nb_factors; i++)
        factored_df.push_back(new DataAsciiClass(path_prefix,ifs,(DataAsciiClass*)factored_df[0]));
      datafile.push_back(factored_df);
      ok=true;
    }

    if (buf==DATA_FILE_MNIST) {
      factored_df.clear();
      factored_df.push_back(new DataMnist(path_prefix,ifs));
      for (int i=1; i<nb_factors; i++)
        factored_df.push_back(new DataMnist(path_prefix,ifs,(DataMnist*)factored_df[0]));
      datafile.push_back(factored_df);
      ok=true;
    }

    if (buf==DATA_FILE_NGRAMBIN) {
      factored_df.clear();
      factored_df.push_back(new DataNgramBin(path_prefix,ifs));
      for (int i=1; i<nb_factors; i++)
        factored_df.push_back(new DataNgramBin(path_prefix,ifs,(DataNgramBin*)factored_df[0]));
      datafile.push_back(factored_df);
      ok=true;
    }

    if (buf==DATA_FILE_PHRASEBIN) {
      factored_df.clear();
      factored_df.push_back(new DataPhraseBin(path_prefix,ifs));
      for (int i=1; i<nb_factors; i++)
        factored_df.push_back(new DataPhraseBin(path_prefix,ifs,(DataPhraseBin*)factored_df[0]));
      datafile.push_back(factored_df);
      ok=true;
    }

    if (datafile.size()==1) {
        // input and output dimension is sum of factors
      idim=odim=0;
      for (vector<DataFile*>::iterator it=datafile[0].begin(); it!=datafile[0].end(); ++it) {
        idim+=(*it)->GetIdim();
        odim+=(*it)->GetOdim();
      }
    }
    if (datafile.size()>=1) {
        // check whether additional datafiles have the same dimensions (this is implicitly OK for factors)
        // TODO: check nb of examples
      int nidim=0, nodim=0;
      for (vector<DataFile*>::iterator it=datafile.back().begin(); it!=datafile.back().end(); ++it) {
        nidim+=(*it)->GetIdim();
        nodim+=(*it)->GetOdim();
      }
      if (idim != nidim) Error("mismatch in input dimension\n");
      if (odim != nodim) Error("mismatch in output dimension\n");
    }

    if (!ok) {
      ifs.getline(line, DATA_LINE_LEN);
      cerr << buf << "" << line << endl;
      Error("parse error in above line of the datafile\n");
    }
  }
  ifs.close();

  // check source word list
  if ((sr_wlist != NULL) && (other_data != NULL))
    CompareWordList(sr_wlist, sw_shared, sw_mutex, other_data->sr_wlist, other_data->sw_shared, other_data->sw_mutex);

  // check target word list
  if (tg_wlist == NULL)
    Error("No target word list given\n");
  else if (other_data != NULL)
    CompareWordList(tg_wlist, tw_shared, tw_mutex, other_data->tg_wlist, other_data->tw_shared, other_data->tw_mutex);

  nb_totl=0;
  cout << "Summary of used data: (" << nb_factors << " factors)" << endl;
  for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf) {
    nb_totl+=(*itf)[0]->Info();
    if (nb_factors>1) {
      for (i=1; i<nb_factors; i++) {
        printf("    f%d: ",i+1);
        (*itf)[i]->Info("");
      }
    } 
  }

  cout << " - total number of examples: " << nb_totl << endl;
  cout << " - dimensions: input=" << idim << ", output=" << odim << endl;
  if (resampl_mode) {
    cout << " - resampling with seed " << resampl_seed << endl;
    srand48(resampl_seed);
  }
  if (preload > 0) {
    printf(" - allocating preload buffer of %.1f GBytes\n", (REAL) ((size_t) nb_totl*idim*sizeof(REAL) / 1024 / 1024 / 1024));
    mem_inp = new REAL[(size_t) nb_totl*idim];	// cast to 64bit !
    if (odim>0) mem_trg = new REAL[(size_t) nb_totl*odim];

      // check whether there is a resampling coeff != 0
      // i.e. we need to resample at each rewind
    float s=0;
    for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf)
      s+=(*itf)[0]->GetResampl();
    if (s>=datafile.size()) {
      preload|=DATA_PRELOAD_ONCE;
      cout << " - all resampling coefficients are set to one, loading data once\n";
    }
    
  }
  else {
    if (norm_mode>0)
      Error("Normalization of the data is only implemented with preloading\n");
  }
  Preload();
  Shuffle();
}

/**************************
 *
 **************************/

Data::~Data()
{
  if (preload) {
    delete [] mem_inp;
    if (odim>0) delete [] mem_trg;
  }
  for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf)
    for (vector<DataFile*>::iterator it = (*itf).begin(); it!=(*itf).end(); ++it)
      delete (*it);
  datafile.clear();
  DeleteWordList(tg_wlist, tw_shared, tw_mutex);
  DeleteWordList(sr_wlist, sw_shared, sw_mutex);
  if (path_prefix) free(path_prefix);
}


/**************************
 *
 **************************/

void Data::Shuffle()
{
  if (shuffle_mode < 1 || !preload) return;

  time_t t_beg, t_end;
  time(&t_beg);

  REAL	*inp = new REAL[idim];
  REAL	*trg = new REAL[odim];

  cout << " - shuffling data " << shuffle_mode << " times ...";
  cout.flush();
  for (ulong i=0; i<shuffle_mode*(ulong)nb_totl; i++) {
    ulong i1 = (ulong) (nb_totl * drand48());
    ulong i2 = (ulong) (nb_totl * drand48());
     
    memcpy(inp, mem_inp + i1*idim, idim*sizeof(REAL));
    memcpy(mem_inp + i1*idim, mem_inp + i2*idim, idim*sizeof(REAL));
    memcpy(mem_inp + i2*idim, inp, idim*sizeof(REAL));

    if (odim>0) {
      memcpy(trg, mem_trg + i1*odim, odim*sizeof(REAL));
      memcpy(mem_trg + i1*odim, mem_trg + i2*odim, odim*sizeof(REAL));
      memcpy(mem_trg + i2*odim, trg, odim*sizeof(REAL));
    }
    
  }

  delete [] inp; delete [] trg;

  time(&t_end);
  time_t dur=t_end-t_beg;

  cout << " done (" << dur / 60 << "m" << dur % 60 << "s)" << endl;
}

//**************************
//
//

void Data::Preload()
{
  if (!preload) return;
  if (! (preload&DATA_PRELOAD_TODO)) {
    cout << " - all data is already loaded into memory" << endl;
    return;
  }
  preload &= ~DATA_PRELOAD_TODO; // clear flag

  cout << " - loading all data into memory ...";
  cout.flush();
  time_t t_beg, t_end;
  time(&t_beg);

    // rewind everything
  for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf) {
    for (vector<DataFile*>::iterator it = (*itf).begin(); it!=(*itf).end(); ++it) (*it)->Rewind();
  }

  int idx=0;
  for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf) {

      // get the required number of examples from all factors
    int n = -1, maxn = (*itf)[0]->GetNbresampl();
    int idim1=(*itf)[0]->GetIdim();  // dimension of one factor (identical for all, e.g. a 7-gram)
    int odim1=(*itf)[0]->GetOdim();

    while (++n < maxn) {
      bool ok=false;
      while (!ok) {
          // advance synchronously all factors until ok
        for (vector<DataFile*>::iterator it = (*itf).begin(); it!=(*itf).end(); ++it) {
          if (! (*it)->Next()) (*it)->Rewind(); // TODO: deadlock if file empty
        }
        ok = (drand48() < (*itf)[0]->GetResampl());
      }

        // copy all factors sequentially in memory
      REAL *adr_inp=mem_inp+idx*idim;
      REAL *adr_trg=mem_trg+idx*odim;
      for (vector<DataFile*>::iterator it = (*itf).begin(); it!=(*itf).end(); ++it) {
        memcpy(adr_inp, (*it)->input, idim1*sizeof(REAL));
	adr_inp+=idim1;
        if (odim1 > 0) {
          memcpy(adr_trg, (*it)->target_vect, odim1*sizeof(REAL));
	  adr_trg+=odim1;
        }
      }
      idx++; // next example
    }

  }

  if (norm_mode & 1) {
    cout << " subtract mean,"; cout.flush();
    for (int i=0; i<idim; i++) {
      int e;
      REAL m=0, *mptr;
      for (e=0, mptr=mem_inp+i; e<idx; e++, mptr+=idim) m+=*mptr;
      m = m/idx; // mean
      for (e=0, mptr=mem_inp+i; e<idx; e++, mptr+=idim) *mptr -= m;
    }
  }

  if (norm_mode & 2) {
    cout << " divide by variance,"; cout.flush();
    for (int i=0; i<idim; i++) {
      int e;
      REAL m=0, m2=0, *mptr;
      for (e=0, mptr=mem_inp+i; e<idx; e++, mptr+=idim) { m+=*mptr; m2+=*mptr * *mptr; }
      m = m/idx;  // mean
      m2 = m2/idx - m; // var = 1/n sum_i x_i^2  -  mu^2
      if (m2>0)
        for (e=0, mptr=mem_inp+i; e<idx; e++, mptr+=idim)
          *mptr = (*mptr - m) / m2;
    }
  }
#ifdef DEBUG
    printf("DUMP PRELOADED DATA at adr %x (%d examples of dim %d->%d):\n",mem_inp,idx,idim,odim);
    for (int e=0; e<idx; e++) {
      for (int i=0; i<idim; i++) printf(" %5.2f",mem_inp[e*idim+i]); printf("\n");
    }
#endif

  time(&t_end);
  time_t dur=t_end-t_beg;
  cout << " done (" << dur / 60 << "m" << dur % 60 << "s)" << endl;
}


/**************************
 *
 **************************/

void Data::Rewind()
{
  if (preload) {
       // clear all data, resample and shuffle again
    Preload();
    Shuffle();
  }
  else {
    for (FactoredDataFiles::iterator itf = datafile.begin(); itf!=datafile.end(); ++itf)
      for (vector<DataFile*>::iterator it = (*itf).begin(); it!=(*itf).end(); ++it)
        (*it)->Rewind();
  }
  idx = -1;
}

/**************************
 * Advance to next data
 **************************/

bool Data::Next()
{
  if (idx >= nb_totl-1) return false;
  idx++;

  //verbose version
  /*
  static int v_time = 0;
  if(idx % 10000 == 0){
	  cout << "Having extract data " << (double)idx/nb_totl*100 << "% in"
			  << (clock()-v_time)/1000 << "ms" << endl;
	  v_time = clock();
  }
  */

  if (preload) {
      // just advance to next data in memory
    input = &mem_inp[idx*idim];
    if (odim>0) target = &mem_trg[idx*odim];
//printf("DATA:"); for (int i =0; i<idim; i++) printf(" %5.2f", input[i]); printf("\n");
    if (!(preload&DATA_PRELOAD_ONCE)) preload |= DATA_PRELOAD_TODO; // remember that next Rewind() should preload again
    return true;
  }

  if (nb_factors>1)
    Error("multiple factors are only implemented with preloading");

  if (shuffle_mode > 0) {
      // resample in RANDOMLY SELECTED datafile until data was found
      // we are sure to find something since idx was checked before
    int df = (int) (drand48() * datafile.size());
//cout << " df=" << df << endl;
    datafile[df][0]->Resampl();
    input = datafile[df][0]->input;
    if (odim>0) target = datafile[df][0]->target_vect;
  }
  else {
      // resample SEQUENTIALLY all the data files
    static int df=0, i=-1, nbdf=datafile[df][0]->GetNbex();
    if (idx==0) {df = 0, i=-1, nbdf=datafile[df][0]->GetNbex(); }	// (luint) this) is a hack to know when there was a global rewind
    if (++i >= nbdf) { df++; nbdf=datafile[df][0]->GetNbex(); i=-1; }
    if (df >= (int) datafile.size()) Error("internal error: no examples left\n");
//printf("seq file: df=%d, i=%d\n", df,i);
    datafile[df][0]->Resampl();	//TODO: idx= ??
//cout << " got df=" << df << " idx="<<idx<<endl;
    input = datafile[df][0]->input;
    if (odim>0) target = datafile[df][0]->target_vect;
  }

  return true;
}

//**************************
//
//

void Data::CreateWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im, ifstream &ifs)
{
  if (im != NULL)
    pthread_mutex_lock(im);

  // new word list
  if (iw == NULL)
    iw = new vector<WordList>;
  if (iw == NULL)
    Error("Can't allocate word list");
  iw->reserve(nb_factors);
  iw->resize(nb_factors);
  for (int i=0; i<nb_factors; i++) {
    string fpath, fname;
    ifs >> fname;
    if (path_prefix != NULL) {
      fpath = path_prefix;
      fpath += '/';
    }
    fpath += fname;
    cout << " - reading word list from file " << fpath << flush;
    WordList::WordIndex voc_size = iw->at(i).Read(fpath.c_str());
    cout << ", got " << voc_size << " words" << endl;
  }

  if (im != NULL)
    pthread_mutex_unlock(im);
  else {
    // word list sharing
    im = new pthread_mutex_t;
    if (im != NULL) {
      pthread_mutex_init(im, NULL);
      int *new_is = new int;
      if (new_is != NULL) {
        (*new_is) = 0;
        is = new_is;
      }
    }
  }
}

void Data::CompareWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im, vector<WordList> *ew, int *es, pthread_mutex_t *em)
{
  if ((iw == NULL) || (ew == NULL))
    return;

  // compare with other word list
  size_t stCurWl = 0;
  size_t stWlCountI = iw->size();
  size_t stWlCountE = ew->size();
  if (stWlCountI == stWlCountE)
    for (; stCurWl < stWlCountI ; stCurWl++) {
      WordList::WordIndex wiCur = 0;
      WordList::WordIndex wiSize = (*iw)[stCurWl].GetSize();
      if (wiSize != (*ew)[stCurWl].GetSize())
        break;
      for (; wiCur < wiSize ; wiCur++) {
        WordList::WordInfo &wiInt = (*iw)[stCurWl].GetWordInfo(wiCur);
        WordList::WordInfo &wiExt = (*ew)[stCurWl].GetWordInfo(wiCur);
        if ((wiInt.id != wiExt.id) || (wiInt.n != wiExt.n)  || (wiInt.cl != wiExt.cl)
            || (strcmp(wiInt.word, wiExt.word) != 0)    )
          break;
      }
      if (wiCur < wiSize)
        break;
    }
  if ((stCurWl < stWlCountI) || (stCurWl < stWlCountE))
    Error("Word lists are not identical\n");
  else {
    vector<WordList> *old_iw = iw;
    pthread_mutex_t *old_im = im;
    int *old_is = is;

    // share other word list
    int inc_is = 0;
    if (em != NULL) {
      pthread_mutex_lock(em);
      inc_is = ((es != NULL) ? (*es) + 1 : 0);
      if (inc_is > 0) {
        (*es) = inc_is;
        iw = ew;
        is = es;
        im = em;
      }
      pthread_mutex_unlock(em);
    }
    if (inc_is <= 0)
      Error ("Can't share word list\n");
    else
      // remove previous word list
      DeleteWordList(old_iw, old_is, old_im);
  }
}

void Data::DeleteWordList(vector<WordList> *&iw, int *&is, pthread_mutex_t *&im)
{
  vector<WordList> *old_iw = iw;
  pthread_mutex_t *old_im = im;
  int *old_is = is;
  is = NULL;
  iw = NULL;
  im = NULL;

  // verify if word list is shared
  if (old_im != NULL) {
    pthread_mutex_lock(old_im);
    if (old_is != NULL) {
      if ((*old_is) > 0) {
        (*old_is)--;
        pthread_mutex_unlock(old_im);
        return;
      }
      else
        delete old_is;
    }
  }

  if (old_iw != NULL)
    delete old_iw;

  // destroy mutex
  if (old_im != NULL) {
    pthread_mutex_unlock(old_im);
    pthread_mutex_destroy(old_im);
    delete old_im;
  }
}
