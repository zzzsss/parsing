/*
 * help.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#include "common.h"
//the functions for both procedures...
//--------------------1.get word-list---------------------------
HashMap *load_wordlist(const char *fname)
{
	//lists of the vocabularies
	cout << "---Opening vocab file '" << fname << "'" << endl;
	ifstream ifs;
	ifs.open(fname,ios::in);
	CHECK_FILE(ifs,fname);
	int num=0;
	HashMap *res = new HashMap(CONS_vocab_map_size);
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
	cout << "---Done with loading vocab, all is " << num << endl;
	return res;
}
//-----------------------2.get word-index------------------------------
//used when train_eval,final_eval...(need to delete later...)
int *get_word_index(int length,DependencyInstance* x,HashMap* all_words,int *oov)
{
	string tmp_unknown = WORD_UNKNOWN;
	int* word_index = new int[length+SENTENCE_EXTRA_NUM*2];	//including +4
	for(int i=0;i<length;i++){
		//here we have to take care of the POS
		string *to_find = CONF_if_consider_pos ? (x->combined_feats->at(i)) : x->forms->at(i);
		HashMap::iterator iter = all_words->find(to_find);
		if(iter == all_words->end()){
			if(oov)
				(*oov) ++;
			word_index[SENTENCE_EXTRA_NUM+i] = all_words->find(&tmp_unknown)->second;
		}
		else
			word_index[SENTENCE_EXTRA_NUM+i] = iter->second;
	}
	string sen_s = SENTENCE_START;
	string sen_e = SENTENCE_END;
	string sen_s2 = SENTENCE_START_2;
	string sen_e2 = SENTENCE_END_2;
	word_index[0] = all_words->find(&sen_s2)->second;
	word_index[1] = all_words->find(&sen_s)->second;
	word_index[length+2] = all_words->find(&sen_e)->second;
	word_index[length+3] = all_words->find(&sen_e2)->second;
	return word_index;
}

//----------------3.get_feature-------------------------------------
string* get_feature(DependencyInstance* x,int head,int modify,int* index)
{
	//depends on configurations
	//head first then modify
	stringstream tmp;
	if(CONF_x_dim == 6){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<3) ? (head+1+i) : (modify+i-2);
				tmp << index[which] << '\t';
			}
		}
	}
	else if(CONF_x_dim == 10){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<5) ? (head+i) : (modify+i-5);
				tmp << index[which] << '\t';
			}
		}
	}
	else
		Error("Illegal x dimension");
	return new string(tmp.str());
}

void fill_feature(int head,int modify,int* index,REAL* to_fill)
{
	if(CONF_x_dim == 6){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<3) ? (head+1+i) : (modify+i-2);
				*to_fill++ = index[which];
			}
		}
	}
	else if(CONF_x_dim == 10){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<5) ? (head+i) : (modify+i-5);
				*to_fill++ = index[which];
			}
		}
	}
	else
		Error("Illegal x dimension");
}

//--------------------4.evaluate---------------------------
void debug_pretraining_evaluate(REAL* scores,HashMap* maps,HashMap* wl)
{
	//evaluate the exact train file --- same order...
	cout << "---debug training data and evaluate." << endl;
	CONLLReader* reader = new CONLLReader();
	reader->startReading(CONF_train_file.c_str());
	DependencyInstance* x = reader->getNext();

	int miss_count = 0,sentence_count=0;
	while(x != NULL){
		if(++sentence_count % 3000 == 0)
			cout << "--Finish sentence " << sentence_count << "miss:" << miss_count << endl;
		int length = x->forms->size();
		double *tmp_scores = new double[length*length*2];
		int *word_index = get_word_index(length,x,wl,0);
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					int index = get_index2(length,ii,j,lr);
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = get_feature(x,j,ii,word_index);
					else
						tmp_str = get_feature(x,ii,j,word_index);
					//add it or already there
					HashMap::iterator iter = maps->find(tmp_str);
					int which = iter->second;
					tmp_scores[index] = scores[which];
					delete tmp_str;
				}
			}
		}
		//- decode and write
		vector<int> *ret = decodeProjective(length,tmp_scores);
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(x->heads))[i2])
				miss_count ++;
		}
		delete ret;
		delete x;
		delete []tmp_scores;
		delete []word_index;
		x = reader->getNext();
	}
	reader->finishReading();
	delete reader;
	cout << "---Done with debug training data and evaluate and miss is " << miss_count << endl;
}
