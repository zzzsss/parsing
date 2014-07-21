/*
 * commmon.h
 *
 *  Created on: Jul 11, 2014
 *      Author: zzs
 */

#ifndef COMMMON_H_
#define COMMMON_H_
#include <string>

//the common header
#define VERBOSE_DEBUG
//#define MEMORY_DEBUG

//-----------------------
//step1 -- pre-trainging
//1.the configurations --- forgive my laziness
extern std::string CONF_train_file;
extern int CONF_train_feat_map_size;
extern int CONF_train_word_map_size;

//2.scores
extern std::string SCO_map_file;
extern std::string SCO_score_file;
extern std::string SCO_descript_file;

//3.strings
#define SENTENCE_START "<s>"
#define SENTENCE_END "</s>"
#define SENTENCE_FILE "sentences"

//4.iterations
//----------------------------

//----------------------------
//step2 -- training
//(1)test
#define STEP2_TEST
//(2)transform
extern std::string CONF_wl_file;
extern std::string CONF_feat_bin;
extern std::string CONF_score_bin;
extern int CONF_X_dim;
extern int CONF_Y_dim;
extern std::string unknown_token;
//(3)nn--cslm
#define BLAS_ATLAS
//#define BLAS_INTEL_MKL


//------------------------------

//----------------------------
//step3 -- output
extern std::string CONF_output_file;
extern std::string CONF_gold_file;
extern std::string CONF_test_file;

//------------------------------




#endif /* COMMMON_H_ */
