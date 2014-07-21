/*
 * pre_training.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: zzs
 */

#include "param.h"
#include <sstream>

//--------------------1.get word-list---------------------------
HashMap *load_wordlist(const char *fname)
{
	//lists of the vocabularies
	cout << "------1.Opening vocab file '" << fname << "'" << endl;
	ifstream ifs;
	ifs.open(fname,ios::in);
	CHECK_FILE(ifs,fname);
	int num=0;
	HashMap *res = new HashMap(CONF_vocab_map_size);
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
	cout << "------1.Done with loading vocab, all is " << num << endl;
	return res;
}

//----------------------2.load training data----------------------------
string* get_feature(DependencyInstance* x,int i,int j)
{
	// i:head --> j:modify
	vector<string*> list = *(x->forms);
	string head,modify;
	// 2 words
#ifdef WHICH_TWO
	head = *(list[i]);
	modify = *(list[j]);
#endif
	// 6 words
#ifdef WHICH_SIX
	int length = list.size()-1;
	head = ((i==0)?SENTENCE_START:*(list[i-1]))+" "+*(list[i])
			+" "+((i==length)?SENTENCE_END:*(list[i+1]));
	modify = ((j==0)?SENTENCE_START:*(list[j-1]))+" "+*(list[j])
					+" "+((j==length)?SENTENCE_END:*(list[j+1]));
#endif

	return new string(head+" "+modify);
}

vector<int>* get_feature_index(int* x,int i,int j)
{
	//remember the index includes <s> and </s>
	vector<int>* index = new vector<int>();
	//2 words
#ifdef WHICH_TWO
	index->push_back(x[i+1]);
	index->push_back(x[j+1]);
#endif
	// 6 words
#ifdef WHICH_SIX
	index->push_back(x[i]);
	index->push_back(x[i+1]);
	index->push_back(x[i+2]);
	index->push_back(x[j]);
	index->push_back(x[j+1]);
	index->push_back(x[j+2]);
#endif
	return index;
}

int load_trainfile(const char* fname,HashMap* wl,REAL** xbin,REAL** ybin)
{
	CONLLReader* reader = new CONLLReader();
	reader->startReading(fname);

	cout << "------2.Opening train file '" << fname << "'" << endl;
	HashMap all_features(CONF_train_feat_map_size);
	vector<int> all_feat_freq;
	vector<int> all_feat_ashead;
	vector<string* > all_feat_str;
	//vector<vector<int>* > all_feat_windex;

	//read all
	DependencyInstance* x = reader->getNext();
	int sentence_count=0,feat_count=0,tokens_count=0;
	while(x != NULL){
		sentence_count++;
		if(sentence_count % 1000 == 0)
			cout << "--Reading sentence " << sentence_count << endl;
		int length = x->forms->size();
		tokens_count += length;
		//--deal with all pairs
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
					HashMap::iterator iter = all_features.find(tmp_str);
					if(iter == all_features.end()){
						all_features.insert(pair<string*, int>(tmp_str, feat_count));
						all_feat_str.push_back(tmp_str);
						all_feat_freq.push_back(1);
						all_feat_ashead.push_back(0);
						feat_count++;
					}
					else{
						int where = iter->second;
						all_feat_freq[where] += 1;
						delete tmp_str;
					}
				}
			}
		}
		//--add positive samples
		for(int i=1;i<length;i++){
			//add some scores first
			int head = (*x->heads)[i];
			string* tmp = get_feature(x,head,i);
			HashMap::iterator iter = all_features.find(tmp);
			all_feat_ashead[iter->second] += 1;
			delete tmp;
		}
		//next
		delete x;
		x = reader->getNext();
	}
	reader->finishReading();
	delete reader;
	cout << "-Training data:\n"
			<< "--Sentences: " << sentence_count << '\n'
			<< "--Tokens: " << tokens_count << '\n'
			<< "--Feats: " << feat_count << '\n';

	//maybe calculating
	cout << "-Transfering as binarys." << endl;
	(*xbin) = new REAL[CONF_X_dim*feat_count];
	(*ybin) = new REAL[CONF_Y_dim*feat_count];
	string unknown_token = UNKNOW_WORD;
	for(int i=0;i<feat_count;i++){
		//get x
		istringstream tmp_stream(*(all_feat_str.at(i)));
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
			//get the scores --- 0~1
			const int THE_UP_LIMIT = 5;
			const REAL SCO_INIT = -0.9;
			const REAL SCO_STEP = 1.8;
			const REAL SCO_MAX = 0.9;

			//scoring method
			int it = i*CONF_Y_dim+j;
			int how_many = all_feat_ashead[it];
			if(how_many>=THE_UP_LIMIT)
				how_many=THE_UP_LIMIT;
			REAL tmp = SCO_INIT;
			for(int k=0;k<how_many;k++){
				tmp += (SCO_STEP/2)*(1+((double)rand()/RAND_MAX));
				if(tmp>SCO_MAX){
					tmp = SCO_MAX;
					break;
				}
			}
			(*ybin)[it] = tmp;
		}
	}

	//additional debug
#ifdef DEBUG_PRETRAINING
	extern void debug_pretraining_write(REAL* ,REAL* ,int );
	extern void debug_pretraining_evaluate(int ,REAL*,HashMap*);
	debug_pretraining_write(*xbin,*ybin,feat_count);
	debug_pretraining_evaluate(feat_count,*ybin,&all_features);
#endif

	//cleanup
	for(int i=0;i<feat_count;i++){
		delete all_feat_str[i];
		//delete all_feat_windex[i];
	}
	cout << "------2.Done with train file '" << fname << "'" << endl;
	return feat_count;
}

