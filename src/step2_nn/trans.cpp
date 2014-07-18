/*
 * trans.cpp
 *
 *  Created on: Jul 15, 2014
 *      Author: zzs
 */

#include "load_pre.h"
#include "../HashMap.h"
#include <fstream>
#include <string>
#include "../cslm/Data.h"
#include "../which_main.h"
using namespace std;
using namespace training_space;

int main2trans()
{
	//transform into the binary form
	HashMap * wl = load_wordlist(CONF_wl_file.c_str());
	//get origin map and scores
	cout << "Transforming into binary forms." << endl;
	ifstream x_feat,y_score;
	x_feat.open(SCO_map_file.c_str());
	y_score.open(SCO_score_file.c_str());
	//-loading
	cout << "Loading the result from step1..." << endl;
	int num,num2;
	x_feat >> num;
	y_score >> num2;
	if(num != num2){//error
		Error("Files does not match in size.");
	}
	//-allocating mem
	string str_tmp;
	REAL num_tmp;
	REAL *feat_bin = new REAL[num*CONF_X_dim];
	REAL *score_bin = new REAL[num*CONF_Y_dim];
	for(int i=0;i<num;i++){
		if(i%100000 == 0)
			cout << "Having processed " << i << endl;
		for(int j=0;j<CONF_X_dim;j++){
			x_feat >> str_tmp;
			HashMap::iterator iter = wl->find(&str_tmp);
			if(iter == wl->end()){
				//out of wordlist --- strange
				cout << "Strange word not in the list " << str_tmp << endl;
				feat_bin[i*CONF_X_dim+j] = (REAL)(wl->find(&unknown_token)->second);
			}
			else
				feat_bin[i*CONF_X_dim+j] = iter->second;
		}
		for(int j=0;j<CONF_Y_dim;j++){
			y_score >> num_tmp;
			score_bin[i*CONF_Y_dim+j] = num_tmp;
		}
	}
	x_feat.close();
	y_score.close();
	//-write them
	cout << "Writing binary files." << endl;
	Data::write_array(CONF_feat_bin.c_str(),num,CONF_X_dim,feat_bin);
	Data::write_array(CONF_score_bin.c_str(),num,CONF_Y_dim,score_bin);
	cout << "Ok, done." << endl;
	delete []feat_bin;
	delete []score_bin;
	return 0;
}


