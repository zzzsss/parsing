/*
 * param.h
 *
 *  Created on: Jul 18, 2014
 *      Author: zzs
 */

#ifndef PARAM_H_
#define PARAM_H_
#include "tools/HashMap.h"
#include "tools/Eisner.h"
#include "tools/CONLLReader.h"
#include "tools/CONLLWriter.h"
#include "tools/DependencyInstance.h"
#include "tools/DependencyEvaluator.h"
//#include "cslm/Tools.h"
//#include "cslm/Data.h"
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
using namespace std;

#define PARSING_DEBUG
#define Error(str) { fprintf(stderr,"Error:%s\n",str); exit(1); }
#define CHECK_FILE(ifs,fname) if(!ifs) { perror(fname); Error(""); }
typedef float REAL;

//debugs
#ifdef PARSING_DEBUG
#define DEBUG_PRETRAINING
#endif

//1.vocabs
#define SENTENCE_START "<s>"
#define SENTENCE_END "</s>"
#define UNKNOW_WORD "<unk>"
extern int CONF_vocab_map_size;
extern string CONF_vocab_file_name;
extern HashMap *load_wordlist(const char *fname);

//2.pre-training
extern std::string CONF_train_file;//well, too lazy to change those...
extern int CONF_train_feat_map_size;
extern int load_trainfile(const char* fname,HashMap* wl,REAL** xbin,REAL** ybin);

extern int CONF_X_dim;
extern int CONF_Y_dim;
//#define WHICH_TWO
#define WHICH_SIX

extern string ASCII_traindata_index;
//for debug

//2.5 scores
//get the scores --- -0.9~+0.9
const REAL SCO_INIT = -0.9;	//constrain low
const REAL SCO_STEP = 1.8;
const REAL SCO_MAX = 0.9;	//constrain high
//for score iterations
const REAL ADJ_INIT = 0.5;
const REAL ADJ_END = 0.09;
const REAL ADJ_STEP = -0.1;
namespace pre_training_space{
	int pre_training(const char* fname,HashMap* wl,REAL** xbin,REAL** ybin);
}

extern void debug_pretraining_write(REAL* ,REAL* ,int );
#endif /* PARAM_H_ */
