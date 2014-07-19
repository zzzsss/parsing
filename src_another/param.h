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
#include "cslm/Tools.h"
#include "cslm/Data.h"
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
using namespace std;

#define PARSING_DEBUG
#define CHECK_FILE(ifs,fname) if(!ifs) { perror(fname); Error(); }
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
extern std::string CONF_train_file;
extern int CONF_train_feat_map_size;
extern int load_trainfile(const char* fname,HashMap* wl,REAL** xbin,REAL** ybin);

extern int CONF_X_dim;
extern int CONF_Y_dim;
//#define WHICH_TWO
#define WHICH_SIX

extern string ASCII_traindata_index;
//for debug

//3.training

#endif /* PARAM_H_ */
