/*
 * training_before.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: zzs
 */

#include "common.h"

//no iteration version...
void pre_training()
{
	CONLLReader* reader = new CONLLReader();
	reader->startReading(CONF_train_file.c_str());
	cout << "---1.Opening train file '" << CONF_train_file << "'" << endl;
	cout << "----No iteration mode----" << endl;
	HashMap all_features(CONS_feat_map_size);
	HashMap all_words(CONS_vocab_map_size);
	//vector<int> all_feat_freq;
	vector<int> all_feat_ashead;
	vector<string* > all_feat_str;
	vector<string* > all_word_str;
	int sentence_count=0,feat_count=0,tokens_count=0,words_count=0;

	//1.init the words-HashMap
	string* sen_s = new string(SENTENCE_START);
	string* sen_e = new string(SENTENCE_END);
	string* sen_s2 = new string(SENTENCE_START_2);
	string* sen_e2 = new string(SENTENCE_END_2);
	string* unk_w = new string(WORD_UNKNOWN);
	all_word_str.push_back(sen_s);
	all_word_str.push_back(sen_e);
	all_word_str.push_back(sen_s2);
	all_word_str.push_back(sen_e2);
	all_word_str.push_back(unk_w);
	all_words.insert(pair<string*, int>(sen_s,words_count++));
	all_words.insert(pair<string*, int>(sen_e,words_count++));
	all_words.insert(pair<string*, int>(sen_s2,words_count++));
	all_words.insert(pair<string*, int>(sen_e2,words_count++));
	all_words.insert(pair<string*, int>(unk_w,words_count++));

	//2.read-in all the feats
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		sentence_count++;
		if(sentence_count % 3000 == 0)
			cout << "--Reading sentence " << sentence_count << endl;
		int length = x->forms->size();
		tokens_count += length;
		//--2.1.add words --- get index while adding
		int* word_index = new int[length+SENTENCE_EXTRA_NUM*2];	//including +4
		for(int i=0;i<length;i++){
			//here we have to take care of the POS
			string *to_find = CONF_if_consider_pos ? (x->combined_feats->at(i)) : x->forms->at(i);
			HashMap::iterator iter = all_words.find(to_find);
			if(iter == all_words.end()){
				string* new_one = new string(*to_find);
				all_words.insert(pair<string*, int>(new_one,words_count++));
				all_word_str.push_back(new_one);
				word_index[SENTENCE_EXTRA_NUM+i] = words_count-1;
			}
			else
				word_index[SENTENCE_EXTRA_NUM+i] = iter->second;
		}
		word_index[0] = all_words.find(sen_s2)->second;
		word_index[1] = all_words.find(sen_s)->second;
		word_index[length+2] = all_words.find(sen_e)->second;
		word_index[length+3] = all_words.find(sen_e2)->second;
		//--deal with all pairs
		for(int i=0;i<length;i++){
			for(int j=i+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = get_feature(x,j,i,word_index);
					else
						tmp_str = get_feature(x,i,j,word_index);
					//add it or already there
					HashMap::iterator iter = all_features.find(tmp_str);
					if(iter == all_features.end()){
						all_features.insert(pair<string*, int>(tmp_str, feat_count));
						all_feat_str.push_back(tmp_str);
						//all_feat_freq.push_back(1);
						all_feat_ashead.push_back(0);
						feat_count++;
					}
					else{
						int where = iter->second;
						//all_feat_freq[where] += 1;
						delete tmp_str;
					}
				}
			}
		}
		//--add positive samples
		for(int i=1;i<length;i++){
			//add some scores first
			int head = (*x->heads)[i];
			string* tmp = get_feature(x,head,i,word_index);
			HashMap::iterator iter = all_features.find(tmp);
			all_feat_ashead[iter->second] += 1;
			delete tmp;
		}
		//next
		delete x;
		delete []word_index;
		x = reader->getNext();
	}
	reader->finishReading();
	delete reader;
	cout << "-Training data:\n"
			<< "--Sentences: " << sentence_count << '\n'
			<< "--Words: " << words_count << '\n'
			<< "--Tokens: " << tokens_count << '\n'
			<< "--Feats: " << feat_count << '\n';

	//3.write out vocab file
	cout << "---2.Writing vocab to '" << CONF_vocab_file << "'" << endl;
	ofstream fout(CONF_vocab_file.c_str());
	for(int i=0;i<words_count;i++)
		fout << *(all_word_str.at(i)) << '\n';
	fout.close();

	//4.assign the scores or classes && write to data-file
	cout << "---3.assign scores and write... " << endl;
	cout << "---Classify mode is "<< CONF_if_y_calss << endl;
	REAL* final_scores = new REAL[feat_count];
	string unknown_token = WORD_UNKNOWN;
	fout.open(CONF_data_file.c_str());
	fout << feat_count << '\t' << IND_CONF_x_dim_final << '\t' << 1 << endl;
	for(int i=0;i<feat_count;i++){
		if(i % 4000000 == 0)
			cout << "--Ok with features " << i << endl;
		//write the x
		fout << *(all_feat_str.at(i)) << "\t";
		//get y and write it
		int how_many = all_feat_ashead[i];
		REAL tmp = SCO_INIT;
		for(int k=0;k<how_many;k++){
			tmp += (SCO_STEP/2)*(1+((double)rand()/RAND_MAX));
			if(tmp>SCO_MAX){
				tmp = SCO_MAX;
				break;
			}
		}
		//--if classify
		if(CONF_if_y_calss){
			tmp = (int)((tmp-SCO_INIT)/(SCO_MAX-SCO_INIT) * CONF_y_class_size);
		}
		final_scores[i] = tmp;
		fout << tmp << '\n';
	}
	fout.close();

	//5.evaluate for testing
	cout << "---4.testing it... " << endl;
	debug_pretraining_evaluate(final_scores,&all_features,&all_words);
	cout << "--Finish pre-training." << endl;
	return;
}
