/*
 * load_pre.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: zzs
 */

#include "load_pre.h"
#include "../cslm/Data.h"


namespace training_space{

//data for the training of nn
//-- some for the test
int num_feats=0;
vector<string*> *all_feats_strings;
vector<double> *all_score;
HashMap *all_features;

//check memory
#ifdef MEMORY_DEBUG
int mem_string=0;	//strings
int mem_num=0;	//all_score/hash
int mem_pointers=0;	//all_feats_strings/hash
int mem_pair=0;		//pairs in hash
#endif

//first load the files from step1
void load_for_test()
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
#ifdef MEMORY_DEBUG
		mem_string += sizeof(tmp) + tmp.size();
#endif
#ifdef STEP2_TEST
		pair<string*, int> it = pair<string*, int>(add,i);
		all_features->insert(it);
#ifdef MEMORY_DEBUG
		mem_pair += sizeof(it);
		mem_num += sizeof(int);
		mem_pointers += sizeof(string*);
#endif
#endif
		all_feats_strings->push_back(add);
#ifdef MEMORY_DEBUG
		mem_pointers += sizeof(string*);
#endif
	}
	fin.close();

	for(int i=0;i<num_feats;i++){
		double a;
		fin2 >> a;
		all_score->push_back(a);
#ifdef MEMORY_DEBUG
		mem_pointers += sizeof(double);
#endif
	}
	fin2.close();

	//the used memory
#ifdef MEMORY_DEBUG
		cout << "Memory used(str,num,poi,pair):"
				<< mem_string <<";" << mem_num <<";" << mem_pointers <<";" << mem_pair << '\n';
		cout << "ALL " << mem_string + mem_num + mem_pointers + mem_pair << '\n';
#ifdef STEP2_TEST
		cout << "HashMap " << all_features->size() << endl;
#endif
#endif
}

HashMap *load_wordlist(const char *fname)
{
	//lists of the vocabularies
	cout << "Opening vocab file '" << fname << "'" << endl;
	ifstream ifs;
	ifs.open(fname,ios::in);
	CHECK_FILE(ifs,fname);
	int num=0;
	HashMap *res = new HashMap(CONF_train_word_map_size);
	while (!ifs.eof()){
		string buf;
		ifs >> buf;
		if (buf=="") continue; // HACK
		string* tmp = new string(buf);
		res->insert(pair<string*, int>(tmp, num++));
	}
	if(res->size() != num){
		//sth wrong
		Error("Something wrong with vocab file.");
	}
	cout << "Done with loading vocab, all is " << num << endl;
	return res;
}

}

