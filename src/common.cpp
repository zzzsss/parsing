/*
 * common.cpp
 *
 *  Created on: Jul 11, 2014
 *      Author: zzs
 */

#include "common.h"
#include <string>
#include <iostream>
using namespace std;


//-----------------------
//step1 -- pre-trainging
//1.configurations
string CONF_train_file = "train.right";
int CONF_train_feat_map_size = 30000000;
int CONF_train_word_map_size = 50000;

//2.scores
double SCO_EACH_FEAT_INIT=1;
double SCO_EACH_FEAT_ADJUST=0.5;

string SCO_map_file = "score_map.list";
string SCO_score_file = "score_score.list";
string SCO_descript_file = "score_des.list";


//----------------------------
//step3 -- output
std::string CONF_output_file = "output.txt";
std::string CONF_gold_file = "train.right";
