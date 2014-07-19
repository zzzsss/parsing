#ifndef _Data_h
#define _Data_h

#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>
#include "Tools.h"

/*
 * redefine of the DATA, fitting for the parsing project
 * 	(decoupled from the wordlist...)
 *		--- by zs
 */

/*
 * IMPORTANT: DATA FORMAT
 * 	<HEADER>: magic number(8byte): double(3.1415926)
 * 	<NUM>:	(4byte):int(dim) (4byte):int(nb_totl)
 * 	<DATA>: REAL * dim * nb_totl
 */

class Data
{
protected:
  const char *fname;
  const char *fx;	//data of the input
  const char *fy;	//data of the result
  //char *path_prefix;	// prefix added to all file names
  int  idim, odim;	// dimensions
  int  nb_totl;		// number of examples

    // actual data
  int  idx;		// index of current example [0,nb-1]
  REAL *mem_inp;	// all the input data in memory
  REAL *mem_trg;	// all the output data in memory
    // local tools, only used when preload is activated
  void Shuffle();	// shuffle in memory
  void read_data(const char *p_x, const char *p_y,int in,int out,int total);
public:
  static const double MAGIC_HEADER=3.1415926;
  static void write_array(const char *,int total,int size,REAL*);
  Data(const char *fname, Data *other_data = NULL);
  Data(REAL*x,REAL*y,int i,int o,int total):
	  idx(-1),idim(i),odim(o),nb_totl(total),mem_inp(x),mem_trg(y){Shuffle();}
  ~Data();
    // access function to local variables
  const char *GetFname() {return fname;}
  int GetIdim() {return idim;}
  int GetOdim() {return odim;}
  int GetNb() {return nb_totl;}
  int GetIdx() {if (idx<0) Error("DataNext() must be called before GetIdx()"); return idx;};
    // the following two pointers are only valid after first DataNext() !
  REAL *input;		// pointer to current inputs
  REAL *target;		// pointer to current target
  //REAL *GetData() {return val;}
    // main functions to access data
  void Rewind();	// rewind to first example, performs resmplaing, shuffling etc if activated
  bool Next();		// advance to next example, return FALSE if at end

  REAL* get_allinput() {return mem_inp;}
  REAL* get_alloutput() {return mem_trg;}
};

#endif
