/*
 * load_pre.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: zzs
 */

#include "load_pre.h"


namespace training_space{

//data for the training of nn
int num_feats=0;
vector<string*> *all_feats_strings;
vector<double> *all_score;
HashMap *all_features;

//first load the files from step1
void load_pre()
{
	int temp;
	all_feats_strings = new vector<string*>();
	all_score = new vector<double>();
#ifdef STEP2_TEST
	all_features = new HashMap(CONF_train_feat_map_size);
#endif
	//loading
	cout << "Loading the result from step1..." << endl;
	ifstream fin;
	ifstream fin2;
	fin.open(SCO_map_file.c_str(),ios::in);
	fin2.open(SCO_score_file.c_str(),ios::in);
	fin >> num_feats;
	fin2 >> temp;
	if(temp != num_feats){//error
		cout << "Files not match..." << endl;
		exit(1);
	}

	string tmp;
	std::getline(fin,tmp); //get over lf
	for(int i=0;i<num_feats;i++){
		std::getline(fin,tmp);
		string *add = new string(tmp);
#ifdef STEP2_TEST
		all_features->insert(pair<string*, int>(add,i));
#endif
		all_feats_strings->push_back(add);
	}
	fin.close();

	for(int i=0;i<num_feats;i++){
		double a;
		fin2 >> a;
		all_score->push_back(a);
	}
	fin2.close();
}

}

