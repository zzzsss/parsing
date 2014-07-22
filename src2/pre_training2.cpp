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

#include "param.h"
#include <sstream>
extern void debug_pretraining_evaluate(int num,REAL* scores,HashMap* maps);

namespace pre_training_space{
//data
//1.the instances
vector<DependencyInstance*> *all_instance;
vector<int*> *all_feat_index;

//2.the features
int feat_size=0;
HashMap *all_features;	//feat-str -> feat-index
vector<string*>* all_feats;//feat-index -> feat-str
vector<double>* all_score;
vector< vector<int> >* all_feat_instance;

int final_misses = 0;

//3.the words
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
	all_feat_instance = new vector<vector<int> >();
}

void print_statistics()
{
	cout << "Training data:\n"
			<< "Sentences: " << all_instance->size() << '\n'
			<< "Tokens: " << tokens_size << '\n'
			<< "Feats: " << feat_size << endl;
}

string* get_feature(DependencyInstance* x,int i,int j)
{
	vector<string*> list = *(x->forms);
	// 2 words
#ifdef WHICH_TWO
	string head = *(list[i]);
	string modify = *(list[j]);
#endif
	// 6 words
#ifdef WHICH_SIX
	int length = list.size()-1;
	string head = ((i==0)?SENTENCE_START:*(list[i-1]))+" "+*(list[i])
			+" "+((i==length)?SENTENCE_END:*(list[i+1]));
	string modify = ((j==0)?SENTENCE_START:*(list[j-1]))+" "+*(list[j])
					+" "+((j==length)?SENTENCE_END:*(list[j+1]));
#endif

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
		if(all_instance->size() % 3000 == 0)
			cout << "Reading sentence " << all_instance->size() << endl;
		int length = x->forms->size();
		tokens_size += length - 1;	//excluding root
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
						all_score->push_back(SCO_INIT);
						all_feat_instance->push_back(vector<int>());
						all_feat_instance->back().push_back(all_instance->size());//add current
					}
					else{
						int ttt = iter->second;
						tmp_feat_index[get_index2(length,i,j,lr)] = ttt;
						if((*all_feat_instance)[ttt].back() != all_instance->size()){
							//no repeat
							(*all_feat_instance)[ttt].push_back(all_instance->size());
						}
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
					(SCO_STEP/2)*(1+((double)rand()/RAND_MAX));	//add scores(try random)
			//constrain
			if((*all_score)[iter->second] > SCO_MAX)
				(*all_score)[iter->second] = SCO_MAX;
			delete tmp;
		}
		//adding instances
		all_instance->push_back(x);
		all_feat_index->push_back(tmp_feat_index);
		//next
		x = reader->getNext();
	}
	delete reader;
	//test-print
	print_statistics();
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
	return tmp_scores;
}

void check_all_sentences_miss()
{
	int all_size = all_instance->size();
	int all_miss=0;
	for(int i=0;i<all_size;i++){
		DependencyInstance* inst = (*all_instance)[i];
		int length = inst->forms->size();
		double *tmp_scores = get_score_once(i,inst);
		vector<int> *ret = decodeProjective(length,tmp_scores);
		int count=0;
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(inst->heads))[i2])
				count ++;
		}
		all_miss += count;
		delete []tmp_scores;
		delete ret;
	}
	cout << "Checking for miss rate is " << all_miss << "/" << tokens_size << '\n';
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
	cout << "Init miss rate is " << all_miss << "/" << tokens_size << '\n';
	cout << "-Through all sentence's time is " << clock()/1000 - time_start << " ms" << endl;

	//iterate to get better scores
	int iter=0;
	int multiply_enlarge_time=0;
	//--init iter numbers
	REAL current_adjust_amount = ADJ_INIT;
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
						<< clock()/1000-iter_time_fine << "ms"
						<< "Till now Reduce " << reduce_miss_num << endl;
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
			if(*sco_iter + current_adjust_amount <= SCO_MAX){
				*sco_iter += current_adjust_amount;
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
				*sco_iter -= current_adjust_amount;
			}
			//minus
			if(*sco_iter - current_adjust_amount >= SCO_INIT){
				*sco_iter -= current_adjust_amount;
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
				*sco_iter += current_adjust_amount;
			}
			//conclude and update
			if(change_plus<0 && change_plus<=change_minus){//prefer plus
				*sco_iter += current_adjust_amount;
				changed_feat_score++;
				reduce_miss_num -= change_plus;
				for(int i=0;i<num;i++)
					miss_for_each[(*rel_iter)[i]] = miss_plus[i];
				all_miss += change_plus;
			}
			else if(change_minus<0 && change_plus>=change_minus){
				*sco_iter -= current_adjust_amount;
				changed_feat_score++;
				reduce_miss_num -= change_minus;
				for(int i=0;i<num;i++)
					miss_for_each[(*rel_iter)[i]] = miss_minus[i];
				all_miss += change_minus;
			}
			else{
				//no change
			}
			//clean
			delete []miss_origin;
			delete []miss_plus;
			delete []miss_minus;
			f++;
		}
		//finish one iter
		check_all_sentences_miss();
		cout << "Finish iter " << iter << ":" << (clock()/1000-iter_starttime)/1000 << "s\n"<<
			" -- changed feat scores " << changed_feat_score
				<< ";miss rate is " << all_miss << "/" << tokens_size
				<< ";reduce misses of "<< reduce_miss_num << endl;
		if(1){	//break
			//cout << "ok, small changes yet" << endl;
			if(reduce_miss_num > 500){
			}
			else if(current_adjust_amount > ADJ_END){
				current_adjust_amount += ADJ_STEP;
			}
			else if(changed_feat_score==0){
				cout << "No changes, stop." << endl;
				final_misses = all_miss;
				break;
			}
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
	delete all_score;
}

int pre_training(const char* fname,HashMap* wl,REAL** xbin,REAL** ybin)
{
	//0.all the words
	all_words = wl;
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

	//3.change to binary
	//maybe calculating
	cout << "-Transfering as binarys." << endl;
	(*xbin) = new REAL[CONF_X_dim*feat_size];
	(*ybin) = new REAL[CONF_Y_dim*feat_size];
	string unknown_token = UNKNOW_WORD;
	for(int i=0;i<feat_size;i++){
		//get x
		istringstream tmp_stream(*(all_feats->at(i)));
		for(int j=0;j<CONF_X_dim;j++){
			string tmp_find;
			tmp_stream >> tmp_find;
			REAL the_index=0;
			HashMap::iterator iter = wl->find(&tmp_find);
			if(iter == wl->end()){
				cout << "--Strange word not find " << tmp_find << endl;
				the_index = wl->find(&unknown_token)->second;
			}
			else
				the_index = iter->second;
			(*xbin)[j+i*CONF_X_dim] = the_index;
		}
		//get y
		for(int j=0;j<CONF_Y_dim;j++){
			(*ybin)[i*CONF_Y_dim+j] = all_score->at(i*CONF_Y_dim+j);
		}
	}

#ifdef DEBUG_PRETRAINING
	debug_pretraining_evaluate(feat_size,*ybin,all_features);
#endif

	//4.clean up
	pre_training_clean1();
	return feat_size;
}
}




