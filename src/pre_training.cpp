/*
 * pre_training.cpp
 *
 *  Created on: Jul 11, 2014
 *      Author: zzs
 */

/* the steps for pre-training
 * 	--- functions and data
 * 			sorry that all the functions are gathered here for convenience
 */

#include "pre_training.h"
#include "CONLLReader.h"
#include "DependencyInstance.h"
#include "common.h"
#include "HashMap.h"
#include "Eisner.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <fstream>

using namespace std;

namespace pre_training_space{
//data
//1.the instances
vector<DependencyInstance*> *all_instance;
vector<int*> *all_feat_index;

//2.the features
int feat_size=0;
int feat_size_with_rep=0;
int feat_string_len=0;
HashMap *all_features;	//feat-str -> feat-index
vector<string*>* all_feats;//feat-index -> feat-str
vector<double>* all_score;
vector< vector<int> >* all_feat_instance;

int final_misses = 0;

//3.the words
int word_size=0;
HashMap *all_words;		//word-str -> word-index

//4.other statistics
int tokens_size=0;

//functions
void pre_init()
{
	all_instance = new vector<DependencyInstance*>();
	all_feat_index = new vector<int*>();
	all_features = new HashMap(CONF_train_feat_map_size);
	all_feats = new vector<string*>();
	all_score = new vector<double>();
	all_words = new HashMap(CONF_train_word_map_size);
	all_feat_instance = new vector<vector<int> >();
}
void print_statistics()
{
	cout << "Training data:\n"
			<< "Sentences: " << all_instance->size() << '\n'
			<< "Tokens: " << tokens_size << '\n'
			<< "Words: " << word_size << '\n'
			<< "Feats: " << feat_size << '\n'
			<< "ALL feats with rep: " << feat_size_with_rep <<'\n'
			<< "All feats str len: " << feat_string_len <<endl;
}
void print_feat_stat()
{
	//print the verbose stat of feats
	int max = -1;
	int max_score = -1;
	vector<int> lens;
	vector<int> lens_score;
	for(int i=0;i<all_feat_instance->size();i++){
		//rep
		int s = (*all_feat_instance)[i].size();
		if(s>max){
			for(int j=0;j<s-max-1;j++)
				lens.push_back(0);
			lens.push_back(1);
			max = s;
		}
		else
			lens[s]++;
		//score
		int s2 = (*all_score)[i];
		if(s2>max_score){
			for(int j=0;j<s2-max_score-1;j++)
				lens_score.push_back(0);
			lens_score.push_back(1);
			max_score = s2;
		}
		else
			lens_score[s2]++;

	}
	//output
	cout << "--Verbose info of feats:\n";
	cout << "---feats repeat info:\n";
	for(int i=0;i<=max;i++){
		cout << i << ":" << lens[i] << " ";
	}
	cout << endl;
	cout << "---feats scores info:\n";
	for(int i=0;i<=max_score;i++){
		cout << i << ":" << lens_score[i] << " ";
	}
	cout << endl;
}

string* get_feature(DependencyInstance* x,int i,int j)
{
	vector<string*> list = *(x->forms);
	// 2 words
	/*
	string head = *(list[i]);
	string modify = *(list[j]);
	*/
	// 6 words

	int length = list.size()-1;
	string head = ((i==0)?SENTENCE_START:*(list[i-1]))+" "+*(list[i])
			+" "+((i==length)?SENTENCE_END:*(list[i+1]));
	string modify = ((j==0)?SENTENCE_START:*(list[j-1]))+" "+*(list[j])
					+" "+((j==length)?SENTENCE_END:*(list[j+1]));

	return new string(head+" "+modify);
}

// Step1 --- reading
void pre_training_reading()
{
	CONLLReader* reader = new CONLLReader();
	reader->startReading(CONF_train_file.c_str());

	pre_init();
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
#ifdef VERBOSE_DEBUG
		if(all_instance->size() % 1000 == 0)
			cout << "Reading sentence " << all_instance->size() << endl;
#endif
		int length = x->forms->size();
		tokens_size += length - 1;	//excluding root
		feat_size_with_rep += length*length*2;
		//adding features(pairs)
		int *tmp_feat_index = new int[length*length*2];
		for(int i=0;i<length;i++){
			for(int j=i+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					//seperated by space
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = get_feature(x,j,i);
					else
						tmp_str = get_feature(x,i,j);
					//add it or already there
					HashMap::iterator iter = all_features->find(tmp_str);
					if(iter == all_features->end()){
						all_features->insert(pair<string*, int>(tmp_str, feat_size));
						tmp_feat_index[get_index2(length,i,j,lr)] = feat_size;
						feat_size++;
						all_feats->push_back(tmp_str);
						all_score->push_back(0.0);
						all_feat_instance->push_back(vector<int>());
						all_feat_instance->back().push_back(all_instance->size());//add current
#ifdef MEMORY_DEBUG
						feat_string_len += tmp_str->length();
#endif
					}
					else{
						int ttt = iter->second;
						tmp_feat_index[get_index2(length,i,j,lr)] = ttt;
						(*all_feat_instance)[ttt].push_back(all_instance->size());
#ifdef MEMORY_DEBUG
						feat_string_len += tmp_str->length();
#endif
						delete tmp_str;
					}
				}
			}
		}
		for(int i=1;i<length;i++){
			//add some scores first
			int head = (*x->heads)[i];
			string* tmp = get_feature(x,head,i);
			HashMap::iterator iter = all_features->find(tmp);
			(*all_score)[iter->second] +=
					(SCO_EACH_FEAT_INIT/2)*(1+((double)rand()/RAND_MAX));	//add scores(try random)
			delete tmp;
		}
		//adding instances
		all_instance->push_back(x);
		all_feat_index->push_back(tmp_feat_index);
		//adding words
		for(int i=0;i<length;i++){
			string* tmp_s = ((*x->forms)[i]);
			HashMap::iterator iter = all_words->find(tmp_s);
			if(iter == all_words->end()){
				all_words->insert(pair<string*, int>(tmp_s, word_size));
				word_size++;
			}
		}
		//next
		x = reader->getNext();
	}
	delete reader;
	//test-print
	print_statistics();
	//print_feat_stat();
}


//get the score
double * get_score_once(int inst_index,DependencyInstance* inst)
{
	int* inst_ind = ((*all_feat_index)[inst_index]);
	int length = inst->forms->size();
	double *tmp_scores = new double[length*length*2];

	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				int index = get_index2(length,ii,j,lr);
				tmp_scores[index] = (*all_score)[(*all_feat_index)[inst_index][index]];
			}
		}
	}

	/*
	for(int ii=0;ii<length*length*2;ii++){
		int index = (inst_ind)[ii];
		if(index>=0 && index<feat_size)
			tmp_scores[ii] = (*all_score)[index];
	}
	*/
	return tmp_scores;
}

//Step2 --- scoring
void pre_training_scoring()
{
	//first the original ones
	int all_size = all_instance->size();
	int* miss_for_each = new int[all_size];
	int all_miss = 0;
	int time_start = clock()/1000;
	for(int i=0;i<all_size;i++){
		int time1 = clock();
		DependencyInstance* inst = (*all_instance)[i];
		int length = inst->forms->size();
		double *tmp_scores = get_score_once(i,inst);
		int time2 = clock();
		vector<int> *ret = decodeProjective(length,tmp_scores);
		int time3 = clock();
		int count=0;
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(inst->heads))[i2])
				count ++;
		}
		miss_for_each[i] = count;
		all_miss += count;
		delete []tmp_scores;
		delete ret;
		int time4 = clock();
		//cout << time1 <<";" << time2 << ";" << time3 << ";" << time4 << '\n';
	}
	final_misses = all_miss;
	cout << "Init miss rate is " << all_miss << "/" << tokens_size-all_size << '\n';
	cout << "-Through all sentence's time is " << clock()/1000 - time_start << " ms" << endl;

	//iterate to get better scores
	int iter=0;
	while(1){
		int iter_starttime = clock()/1000;	//ms
		cout << "Iteration " << iter << endl;

		//for each feature(pair)
		//--iteration varaiables
		int f=0;
		vector<vector<int> >::iterator rel_iter=all_feat_instance->begin();
		vector<double >::iterator sco_iter=all_score->begin();
		int iter_time_fine = clock()/1000;
		//--iteration statistics
		int changed_feat_score = 0;
		int reduce_miss_num = 0;
		for(;rel_iter!=all_feat_instance->end();rel_iter++,sco_iter++){
			if(f%10000 == 0){
				cout << "Now " << (double)f/feat_size*100 << "% in "
						<< clock()/1000-iter_time_fine << "ms" << endl;
				iter_time_fine = clock()/1000;
			}
			//the relative instances
			int num = rel_iter->size();
			int* miss_origin = new int[num];
			for(int i=0;i<num;i++)
				miss_origin[i] = miss_for_each[(*rel_iter)[i]];
			int* miss_plus = new int[num];	//misses for plus
			int* miss_minus = new int[num];	//misses for minus
			int change_plus = 0;
			int change_minus = 0;
			//plus
			*sco_iter += SCO_EACH_FEAT_ADJUST;
			for(int i=0;i<num;i++){
				int inst_index_tmp = (*rel_iter)[i];
				DependencyInstance* inst = (*all_instance)[inst_index_tmp];
				int length = inst->forms->size();
				double *tmp_scores = get_score_once(inst_index_tmp,inst);
				vector<int> *ret = decodeProjective(length,tmp_scores);
				int count=0;
				for(int i2=1;i2<length;i2++){	//ignore root
					if((*ret)[i2] != (*(inst->heads))[i2])
						count ++;
				}
				miss_plus[i] = count;
				change_plus += (count - miss_origin[i]);
				delete []tmp_scores;
				delete ret;
			}
			//minus
			*sco_iter -= 2*SCO_EACH_FEAT_ADJUST;
			for(int i=0;i<num;i++){
				int inst_index_tmp = (*rel_iter)[i];
				DependencyInstance* inst = (*all_instance)[inst_index_tmp];
				int length = inst->forms->size();
				double *tmp_scores = get_score_once(inst_index_tmp,inst);
				vector<int> *ret = decodeProjective(length,tmp_scores);
				int count=0;
				for(int i2=1;i2<length;i2++){	//ignore root
					if((*ret)[i2] != (*(inst->heads))[i2])
						count ++;
				}
				miss_minus[i] = count;
				change_minus += (count - miss_origin[i]);
				delete []tmp_scores;
				delete ret;
			}
			//conclude and update
			if(change_plus<0 && change_plus<=change_minus){//prefer plus
				*sco_iter += 2*SCO_EACH_FEAT_ADJUST;
				changed_feat_score++;
				reduce_miss_num -= change_plus;
				for(int i=0;i<num;i++)
					miss_for_each[(*rel_iter)[i]] = miss_plus[i];
				all_miss += change_plus;
			}
			else if(change_minus<0 && change_plus>=change_minus){
				changed_feat_score++;
				reduce_miss_num -= change_minus;
				for(int i=0;i<num;i++)
					miss_for_each[(*rel_iter)[i]] = miss_minus[i];
				all_miss += change_minus;
			}
			else{
				*sco_iter += SCO_EACH_FEAT_ADJUST;
			}
			//clean
			delete []miss_origin;
			delete []miss_plus;
			delete []miss_minus;
			f++;
		}
		//finish one iter
		cout << "Finish iter " << iter << ":" << (clock()/1000-iter_starttime)/1000 << "s\n"<<
			" -- changed feat scores " << changed_feat_score
				<< ";miss rate is " << all_miss << "/" << tokens_size-all_size
				<< ";reduce misses of "<< reduce_miss_num << endl;
		if(changed_feat_score==0){	//break
			cout << "ok, no changes yet" << endl;
			final_misses = all_miss;
			break;
		}
		else{

		}
		iter++;

	}
	//clean
	delete []miss_for_each;
}

void pre_training_clean1()
{
	for(vector<DependencyInstance*>::iterator iter = all_instance->begin(); iter != all_instance->end(); ++iter){
		delete (*iter);
	}
	delete all_instance;
	for(vector<int*>::iterator iter = all_feat_index->begin(); iter != all_feat_index->end(); ++iter){
		delete [](*iter);
	}
	delete all_feat_index;
	for(HashMap::iterator iter = all_features->begin();iter != all_features->end();iter++){
		delete iter->first;
	}
	delete all_feats;
	delete all_features;
	delete all_words;
	delete all_score;
}

void pre_training()
{
	//1. read-in the data
	int time1 = clock() / (CLOCKS_PER_SEC);
	cout << "---Reading train-file start:" << endl;
	pre_training_reading();
	cout << "---Reading train-file finish: in "
			<< clock() / (CLOCKS_PER_SEC) - time1 << "s" << endl;

	//2. scoring the data
	int time2 = clock() / (1000);
	cout << "---Scoring train-file start:" << endl;
	pre_training_scoring();
	cout << "---Scoring train-file finish: in "
			<< (clock() / (1000) - time2)/1000 << " s" << endl;

	//3. storing the result
	cout << "Storing the result..." << endl;
	ofstream fout;
	//-description file
	fout.open(SCO_descript_file.c_str(),ios::out|ios::trunc);
	fout << "Training data description:\n"
			<< "Sentences: " << all_instance->size() << '\n'
			<< "Tokens: " << tokens_size << '\n'
			<< "Words: " << word_size << '\n'
			<< "Feats: " << feat_size << '\n'
			<< "ALL feats with rep: " << feat_size_with_rep <<'\n'
			<< "Misses after iterations: " << final_misses <<endl;
	fout.close();
	//-maps
	fout.open(SCO_map_file.c_str(),ios::out|ios::trunc);
	fout << feat_size <<'\n';
	for(int i=0;i<feat_size;i++)
		fout << *((*all_feats)[i]) << '\n';
	fout.close();
	//scores
	fout.open(SCO_score_file.c_str(),ios::out|ios::trunc);
	fout << feat_size <<'\n';
	for(int i=0;i<feat_size;i++)
		fout << (*all_score)[i] << '\n';
	fout.close();

	//4.clean up
	pre_training_clean1();
	return;
}
}




