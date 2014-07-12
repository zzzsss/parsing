/*
 * load_pre.h
 *
 *  Created on: Jul 12, 2014
 *      Author: zzs
 */

#ifndef LOAD_PRE_H_
#define LOAD_PRE_H_

#include "../common.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <fstream>
#include "../HashMap.h"

using namespace std;

namespace training_space{

extern int num_feats;
extern vector<string*> *all_feats_strings;
extern vector<double> *all_score;
extern HashMap *all_features;
void load_pre();

}



#endif /* LOAD_PRE_H_ */
