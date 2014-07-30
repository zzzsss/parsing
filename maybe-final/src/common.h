/*
 * common.h
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#ifndef COMMON_H_
#define COMMON_H_
#include "tools/HashMap.h"
#include "tools/Eisner.h"
#include "tools/CONLLReader.h"
#include "tools/CONLLWriter.h"
#include "tools/DependencyInstance.h"
#include "tools/DependencyEvaluator.h"
#include "cslm/Mach.h"
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <sstream>
using namespace std;

//#define Error(str) { fprintf(stderr,"Error:%s\n",str); exit(1); }
//#define CHECK_FILE(ifs,fname) if(!ifs) { perror(fname); Error(""); }
//typedef float REAL;

//--------------constants----------------------
#define SENTENCE_EXTRA_NUM 2
const string SENTENCE_START = "<s>";
const string SENTENCE_END = "</s>";
const string SENTENCE_START_2 = "<ss>";
const string SENTENCE_END_2 = "</ss>";
const string WORD_UNKNOWN = "<unk>";

#define CONS_vocab_map_size 500000
#define CONS_feat_map_size 100000000
//get the scores --- -0.9~+0.9
const REAL SCO_INIT = -0.9;	//constrain low
const REAL SCO_STEP = 1.8;
const REAL SCO_MAX = 0.9;	//constrain high
//for score iterations
const REAL ADJ_INIT = 0.5;
const REAL ADJ_END = 0.09;
const REAL ADJ_STEP = -0.1;

//-------------------Configurations---------------
namespace parsing_conf{
//1.pre-training
extern string CONF_train_file;	//the training file
extern string CONF_data_file;	//the result of pre-training for NN
//2.eval
extern string CONF_test_file;	//testing files
extern string CONF_output_file;
extern string CONF_gold_file;	//golden files
extern string CONF_mach_file;	//mach name
//3.both
extern string CONF_vocab_file;	//output of pre and input of eval
extern int CONF_if_consider_pos;
extern int CONF_x_dim;			//the input dim to the nn 6 or 10--- default 6
extern int IND_CONF_x_dim_final;
extern int CONF_x_dim_missing;	//the missing one for the dimension(only missing one)
extern int CONF_if_y_calss;	//if use the softmax to classify the output
extern int CONF_y_class_size;	//how many classes of y(only if before one is true)

}

//functions
HashMap *load_wordlist(const char *fname);
int *get_word_index(int length,DependencyInstance* x,HashMap* all_words,int *oov);
string* get_feature(DependencyInstance* x,int head,int modify,int* index);
void fill_feature(int head,int modify,int* index,REAL* to_fill);
void debug_pretraining_evaluate(REAL* scores,HashMap* maps,HashMap* wl);
void eval();
void pre_training();

using namespace parsing_conf;

#endif /* COMMON_H_ */
