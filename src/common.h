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
//1.the configurations --- forgive my laziness
extern std::string CONF_train_file;
extern int CONF_train_feat_map_size;
extern int CONF_train_word_map_size;

//2.scores
extern double SCO_EACH_FEAT;

//3.strings
#define SENTENCE_START "<s>"
#define SENTENCE_END "<\s>"

#endif /* COMMMON_H_ */
