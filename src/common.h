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
extern double SCO_EACH_FEAT_INIT;
extern double SCO_EACH_FEAT_ADJUST;
extern std::string SCO_map_file;
extern std::string SCO_score_file;
extern std::string SCO_descript_file;

//3.strings
#define SENTENCE_START "<s>"
#define SENTENCE_END "</s>"

//4.iterations
//----------------------------

//----------------------------
//step2 -- training
#define STEP2_TEST

//------------------------------

//----------------------------
//step3 -- output
extern std::string CONF_output_file;
extern std::string CONF_gold_file;

//------------------------------


//z. again forgive my laziness...
//#define main1 main
#ifdef STEP2_TEST
	#define main2test main
#endif
//#define main2 main

#endif /* COMMMON_H_ */
