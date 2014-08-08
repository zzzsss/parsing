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
 * $Id: DataPhraseBin.cpp,v 1.16 2014/03/25 21:52:53 schwenk Exp $
 */

using namespace std;
#include <iostream>

// system headers
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "Tools.h"
#include "Data.h"
#include "DataPhraseBin.h"

const char* DATA_FILE_PHRASEBIN="DataPhraseBin";
const int DATA_PHRASE_IGN_SHORT_SRC=1;		// ignore phrase pairs for which the source phrase is to short
const int DATA_PHRASE_IGN_SHORT_TGT=16;		// ignore phrase pairs for which the target phrase is to short
const int DATA_PHRASE_IGN_ALL=17;		// all flags ORed together
const int max_buf_len=32;			// maximum length of phrases that can be read

typedef unsigned char uchar;

//*******************

void DataPhraseBin::do_constructor_work()
{
  int n;

  char full_fname[max_word_len]="";

  if (path_prefix) {
    if (strlen(path_prefix)+strlen(fname)+2>(size_t)max_word_len)
      Error("full filename is too long");

    strcpy(full_fname, path_prefix);
    strcat(full_fname, "/");
  }
  strcat(full_fname, fname);

    // parse header binary Ngram file

  fd=open(full_fname, O_RDONLY);
  if (fd<0) {
    perror(full_fname); Error();
  }
  int s;
  read(fd, &s, sizeof(int));
  if (s != sizeof(WordID)) {
    ErrorN("binary phrase data uses %d bytes per index, but this code is compiled for %d byte indices\n", s, (int) sizeof(WordID));
  }

  read(fd, &max_len, sizeof(int));		// maximal length of a phrase (source or target)
  if (max_len<1 || max_len>255) {
    ErrorN("binary phrase data: max length must be in 1..255\n");
  }

    // source side vocabulary infos
  read(fd, &ivocsize, sizeof(int));
  read(fd, &ibos, sizeof(WordID));	// BOS
  read(fd, &ieos, sizeof(WordID));	// EOS, not used
  read(fd, &iunk, sizeof(WordID));	// UNK
  iempty=ibos; // used to identify empty word in phrase

    // read source counts
  inbphw = new int[max_len+1]; inbphw[0]=0;
  for (s=1; s<=max_len; s++) {read(fd, inbphw+s, sizeof(int)); inbphw[0]+=inbphw[s]; }
  printf(" - %s binary phrase pairs with %d entries of max length of %d, mode=%d\n", fname, inbphw[0], max_len, mode);

    // calc source cumulated counts
  icnbphw = new int[max_len+1];
  icnbphw[1]=inbphw[1];
  for (s=2; s<=max_len; s++) icnbphw[s]=icnbphw[s-1]+inbphw[s];
  printf("   source: vocabulary of %d words (bos=%d, eos=%d, unk=%d)\n", ivocsize, ibos, ieos, iunk);
 
    // target side vocabulary infos
  read(fd, &ovocsize, sizeof(int));
  read(fd, &obos, sizeof(WordID));
  read(fd, &oeos, sizeof(WordID));
  read(fd, &ounk, sizeof(WordID));
  oempty=obos;
  printf("   target: vocabulary of %d words (bos=%d, eos=%d, unk=%d)\n", ovocsize, obos, oeos, ounk);

    // read target counts
  onbphw = new int[max_len+1]; onbphw[0]=0;
  for (s=1; s<=max_len; s++) {read(fd, onbphw+s, sizeof(int)); onbphw[0]+=onbphw[s]; }
  if (onbphw[0] != inbphw[0]) {
    ErrorN("number of source phrase (%d) does not match the number of target phrases (%d)\n", inbphw[0], onbphw[0]);
  }

    // calc target cumulated counts
  ocnbphw = new int[max_len+1];
  ocnbphw[1]=onbphw[1];
  for (s=2; s<=max_len; s++) ocnbphw[s]=ocnbphw[s-1]+onbphw[s];
 
  idim=src_phlen;
  odim=tgt_phlen;
 
  if (idim>0) {
    input = new REAL[idim];
  }

  if (odim>0) {
    target_vect = new REAL[odim];
  }

//#ifdef DEBUG
  cout << "    statistics:" << endl;
  printf("       source:"); for (s=0; s<=max_len; s++) printf("%10d", inbphw[s]); printf("\n");
  printf("       target:"); for (s=0; s<=max_len; s++) printf("%10d", onbphw[s]); printf("\n");
//#endif

  printf("   limiting phrase pairs to length %d/%d words, mode %d\n", src_phlen, tgt_phlen, mode);
  if (src_phlen == tgt_phlen) {
      // we can get an UPPER BOUND of the nbr of phrases directly from the file header
    n = icnbphw[src_phlen] < ocnbphw[tgt_phlen] ? icnbphw[src_phlen] : ocnbphw[tgt_phlen];
    nbi=inbphw[0]-n;
    printf("   header: upper bound of %d phrase pairs (%d=%5.2f%% ignored)\n", n, nbi, 100.0*nbi/inbphw[0]);
  }
  
      // counting nbex to get true number of examples
  cout << "    counting ..."; cout.flush();
  time_t t_beg, t_end;
  time(&t_beg);

  int nbi=0; n=0;
  while (DataPhraseBin::Next()) n++;
  nbi=inbphw[0]-n;

  time(&t_end);
  time_t dur=t_end-t_beg;
  printf(" %d phrase pairs (%lum%lus, %d=%5.2f%% ignored)\n", n, dur/60, dur%60, nbi, 100.0*nbi/inbphw[0]);

  if (n>inbphw[0])
    Error("Number of counted examples is larger than the information in file header !?");

  nbex=n;
}

//*******************

DataPhraseBin::DataPhraseBin(char *p_prefix, ifstream &ifs, DataPhraseBin* prev_df) : DataFile(p_prefix, ifs, prev_df),
 max_len(0), mode(0), src_phlen(0), tgt_phlen(0),
 inbphw(NULL),
 icnbphw(NULL),
 onbphw(NULL),
 ocnbphw(NULL),
 nbi(0)
{
    // DataPhraseBin <file_name> <resampl_coeff> <src_phrase_len> <tgt_phrase_len> [flags]
    // parse addtl params
  if (prev_df) {
    src_phlen=prev_df->src_phlen;
    tgt_phlen=prev_df->tgt_phlen;
    mode=prev_df->mode;
  }
  else {
    ifs >> src_phlen >> tgt_phlen >> mode;
    if (src_phlen<1 || src_phlen>9)
      Error("length of source phrases must be in [1,9]\n");
    if (tgt_phlen<1 || tgt_phlen>9)
      Error("length of target phrases must be in [1,9]\n");
    if (mode<0 || mode>DATA_PHRASE_IGN_ALL)
      Error("wrong value of DataPhraseBin mode\n");
  }

  do_constructor_work();
}

//*******************

DataPhraseBin::DataPhraseBin(char *p_fname, float p_rcoeff, int p_src_phlen, int p_tgt_phlen, int p_mode)
  : DataFile::DataFile(NULL, p_fname, p_rcoeff),
  mode(p_mode), src_phlen(p_src_phlen), tgt_phlen(p_tgt_phlen)
{

  do_constructor_work();
    // TODO:  counting ?
}

//*******************

DataPhraseBin::~DataPhraseBin()
{

  close(fd);
  if (idim>0) delete [] input;
  if (odim>0) delete [] target_vect;
  if (inbphw) delete [] inbphw;
  if (icnbphw) delete [] icnbphw;
  if (onbphw) delete [] onbphw;
  if (ocnbphw) delete [] ocnbphw;
}


//*******************

bool DataPhraseBin::Next()
{
  bool ok=false;
  WordID buf[max_buf_len];

   // we may need to skip some phrase pairs in function of their length
  while (!ok) {
    int i;
    uchar src_len, tgt_len;

      // read source phrase
    if (read(fd, &src_len, sizeof(src_len)) != sizeof(src_len)) return false;
    if ((int) src_len>max_buf_len) Error("The source phrase is too long, you need to recompile the program\n");
    read(fd, buf, src_len * sizeof(WordID)); // TODO: check read
#ifdef DEBUG
    for (i=0; i<src_len; i++) printf(" %d", buf[i]);
    printf("\n");
#endif
    if ((int) src_len>src_phlen) {
       nbi++; // ignore: too many source words
       ok=false; // won't be used, but we still need to read the target phrase to keep it in sync
    }
    else {
        // copy source phrase into input vector
      for (i=0; i<src_len; i++) input[i] = (REAL) buf[i]; 	// careful: we cast int to float
      for (   ; i<src_phlen; i++) input[i] = (REAL) NULL_WORD;
      ok=true;
    }

      // read target phrase
    if (read(fd, &tgt_len, sizeof(tgt_len)) != sizeof(tgt_len)) return false;
    if ((int)tgt_len>max_buf_len) Error("The target phrase is too long, you need to recompile the program\n");
    read(fd, buf, tgt_len * sizeof(WordID)); // TODO: check read
#ifdef DEBUG
    for (i=0; i<tgt_len; i++) printf(" %d", buf[i]);
    printf("\n");
#endif
    if ((int)tgt_len > tgt_phlen) {
       nbi++; ok=false; continue; // ignore: too many target words
    }
    else {
        // copy target phrase into output vector
      for (i=0; i<tgt_len; i++) target_vect[i] = (REAL) buf[i]; 	// careful: we cast int to float
      for (   ; i<tgt_phlen; i++) target_vect[i] = (REAL) NULL_WORD;
    }
  
      // decide wether the current phrase pair is valid in function of the flags
    if (!ok) {
      continue;
    }

    if (mode & DATA_PHRASE_IGN_SHORT_SRC) {
        // ignore phrase that don't have a source length identical to the input dimension
      if (src_len != src_phlen) {
        nbi++; ok=false; continue;
      }
    }

    if (mode & DATA_PHRASE_IGN_SHORT_TGT) {
        // ignore phrase that don't have a target length identical to the output dimension
      if (tgt_len != tgt_phlen) {
        nbi++; ok=false; continue;
      }
    }

    ok=true;
  } // of while (!ok)

  
#ifdef DEBUG
  printf("EX:");
  for (int i=0; i<idim; i++) printf(" %d", (int) input[i]); printf(" ->");
  for (int i=0; i<odim; i++) printf(" %d", (int) target_vect[i]); printf("\n");
#endif

  idx++;
  return true;
}

/********************
 *
 ********************/


void DataPhraseBin::Rewind()
{
  lseek(fd, sizeof(int), SEEK_SET);	// position on field max_phrase_len
  int mlen;
  read(fd, &mlen, sizeof(int));		// get max_phrase_len
  uint pos = 2 * (sizeof(uint)+3*sizeof(WordID) + mlen*sizeof(int) );
  lseek(fd, pos , SEEK_CUR);
  idx=-1;
}
  
