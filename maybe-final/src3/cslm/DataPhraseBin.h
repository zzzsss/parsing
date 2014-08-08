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
 * $Id: DataPhraseBin.h,v 1.9 2014/03/25 21:52:53 schwenk Exp $
 */

#ifndef _DataPhraseBin_h
#define _DataPhraseBin_h

#include <iostream>
#include <fstream>

#include "Tools.h"
#include "DataFile.h"

extern const char* DATA_FILE_PHRASEBIN;

// Syntax of a line in data description:
// DataPhraseBin <file_name> <resampl_coeff> <src_phrase_len> <tgt_phrase_len> [flags]
//   1: skip too short source phrases
//  16: skip too short target phrases
// Phrase pairs for which the source or target part is too long are always skipped
// (there is not reasonable way to "back-off" to a shorter phrase pair
//
// format of binary file
// header: (17 int = 68 bytes)
// int 		sizeof(WordID)
// int 		max_phrase_len
// uint		voc_size		\  source
// WordID	unk, bos, eos WordIDs	/
// int* 	array of number of source phrases for each length 1..max_phrase_len
// uint		voc_size		\  target
// WordID	unk, bos, eos WordIDs	/
// int* 	array of number of targ phrases for each length 1..max_phrase_len

class DataPhraseBin : public DataFile
{
private:
  void do_constructor_work();
protected:
  int fd;			// UNIX style binary file
  int max_len;			// max length wof words in phrase, read from file
  int mode;			// TODO
  int src_phlen, tgt_phlen;	// filter: max length of source and target phrases
    // input
  int ivocsize;			// vocab size (including <s>, </s> and <unk>)
  WordID ibos,ieos,iunk;	// word id of BOS, EOS and UNK in source vocabulary
  WordID iempty;		// word id of empty phrase (used to simulate shorter ones)
  int *inbphw;			// array[max_len+1] of nb of phrases per length
				// indices start at 1, indice 0 gives the total count
  int *icnbphw;			// same, but cumulated number
    // ouput
  int ovocsize;			// vocab size (including <s>, </s> and <unk>)
  WordID obos,oeos,ounk;	// word id of BOS, EOS and UNK in target vocabulary
  WordID oempty;		// word id of empty phrase (used to simulate shorter ones)
  int *onbphw, *ocnbphw;
    // stats (in addition to nbex in mother class)
  int nbi;			// ignored phrases (too long source or target part)
public:
  DataPhraseBin(char*, ifstream&, DataPhraseBin* =NULL);	// optional object to initialize when adding factors
  DataPhraseBin(char*, float =1.0, int =5, int =5, int =17);
  virtual ~DataPhraseBin();
  virtual bool Next();
  virtual void Rewind();
};

#endif
