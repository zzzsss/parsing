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
//For the scoring, there are three ways
//1:no iteration,2:increase step,3:decrease step
#define SCO_WAY 3
//init for each pos/neg train sample
extern double SCO_EACH_FEAT_INIT;
extern double SCO_EACH_FEAT_INIT_NEG;

//adjusts amounts
extern double SCO_STEP;
extern double SCO_CHANGE_AMOUNT;
//extern int SCO_ENLARGE_TRIGGER_CHANGE;
#define SCO_STEP_HIGH 5.0
#define SCO_STEP_LOW 0.5
#define SCO_STEP_CHANGE 0.5
#define SCO_MAX_TIMES 9

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


//z.which main (again forgive my laziness...)
#define main1 main
//#define main2test main
//#define main2 main

#endif /* COMMMON_H_ */
