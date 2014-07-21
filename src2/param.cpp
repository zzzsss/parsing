/*
 * param.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: zzs
 */

#include "param.h"


//1.vocab
int CONF_vocab_map_size=50000;
string CONF_vocab_file_name="vocab.list";


//2.pre-training
std::string CONF_train_file="train.right";
int CONF_train_feat_map_size=50000000;

#ifdef WHICH_TWO
int CONF_X_dim = 2;
#endif
#ifdef WHICH_SIX
int CONF_X_dim = 6;
#endif
int CONF_Y_dim = 1;

string ASCII_traindata_index = "train_index_score.data.update";
