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
	int* word_index;
	int size1 = (length+4);
	if(CONF_if_consider_pos==2)
		word_index = new int[size1*2];	//including +4; pos x 2
	else
		word_index = new int[size1];	//including +4
	for(int i=0;i<length;i++){
		//here we have to take care of the POS
		string *to_find = CONF_if_consider_pos ? (x->combined_feats->at(i)) : x->forms->at(i);
		HashMap::iterator iter = all_words->find(to_find);
		if(iter == all_words->end()){
			if(oov)
				(*oov) ++;
			word_index[2+i] = all_words->find(&WORD_UNKNOWN)->second;
		}
		else
			word_index[2+i] = iter->second;
	}
	word_index[0] = all_words->find(&SENTENCE_START_2)->second;
	word_index[1] = all_words->find(&SENTENCE_START)->second;
	word_index[length+2] = all_words->find(&SENTENCE_END)->second;
	word_index[length+3] = all_words->find(&SENTENCE_END_2)->second;

	if(CONF_if_consider_pos==2){
		for(int i=0;i<length;i++){
			//here we have to take care of the POS
			string *to_find = x->postags->at(i);
			HashMap::iterator iter = all_words->find(to_find);
			if(iter == all_words->end()){
				cout << "Unusual unk pos-tag " << *to_find << endl;
				word_index[2+i+size1] = all_words->find(&POS_UNKNOWN)->second;
			}
			else
				word_index[2+i+size1] = iter->second;
		}
		word_index[size1] = all_words->find(&POS_START_2)->second;
		word_index[size1+1] = all_words->find(&POS_START)->second;
		word_index[size1+length+2] = all_words->find(&POS_END)->second;
		word_index[size1+length+3] = all_words->find(&POS_END_2)->second;
	}
	return word_index;
}

//----------------3.get_feature-------------------------------------
string* get_feature(int len,int head,int modify,int* index)
{
	//depends on configurations
	//head first then modify
	stringstream tmp;
	int *index2 = index + len+4;
	if(CONF_x_dim == 6){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<3) ? (head+1+i) : (modify+i-2);
				tmp << index[which] << ' ';
			}
		}
		if(CONF_if_consider_pos==2){
			for(int i=0;i<CONF_x_dim;i++){
				if(i!=CONF_x_dim_missing){//not ignore it
					int which = (i<3) ? (head+1+i) : (modify+i-2);
					tmp << index2[which] << ' ';
				}
			}
		}
	}
	else if(CONF_x_dim == 10){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<5) ? (head+i) : (modify+i-5);
				tmp << index[which] << ' ';
			}
		}
		if(CONF_if_consider_pos==2){
			for(int i=0;i<CONF_x_dim;i++){
				if(i!=CONF_x_dim_missing){//not ignore it
					int which = (i<5) ? (head+i) : (modify+i-5);
					tmp << index2[which] << ' ';
				}
			}
		}
	}
	else
		Error("Illegal x dimension");
	return new string(tmp.str());
}

void fill_feature(int len,int head,int modify,int* index,REAL* to_fill)
{
	int *index2 = index + len+4;
	if(CONF_x_dim == 6){
		for(int i=0;i<CONF_x_dim;i++){
			if(i!=CONF_x_dim_missing){//not ignore it
				int which = (i<3) ? (head+1+i) : (modify+i-2);
				*to_fill++ = index[which];
			}
		}
		if(CONF_if_consider_pos==2){
			for(int i=0;i<CONF_x_dim;i++){
				if(i!=CONF_x_dim_missing){//not ignore it
					int which = (i<3) ? (head+1+i) : (modify+i-2);
					*to_fill++ = index2[which];
				}
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
		if(CONF_if_consider_pos==2){
			for(int i=0;i<CONF_x_dim;i++){
				if(i!=CONF_x_dim_missing){//not ignore it
					int which = (i<5) ? (head+i) : (modify+i-5);
					*to_fill++ = index2[which];
				}
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
						tmp_str = get_feature(length,j,ii,word_index);
					else
						tmp_str = get_feature(length,ii,j,word_index);
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
