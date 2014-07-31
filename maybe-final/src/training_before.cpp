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

	cout << "---1.Opening train file '" << CONF_train_file << "'" << endl;
	cout << "----No iteration mode----" << endl;
	//-features
	HashMap all_features(CONS_feat_map_size);
	vector<int> all_feat_ashead;
	vector<string* > all_feat_str;
	//-counts
	int sentence_count=0,feat_count=0,tokens_count=0;

	//--if no outside vocab
	if(!CONF_vocab_out){
		cout << "1.5-costruct vocab since no out-vocab" << endl;
		int words_count=0,pos_count=0;
		//-words and pos
		HashMap all_words(CONS_vocab_map_size);
		vector<string* > all_word_str;
		HashMap all_poss(CONS_pos_map_size);
		vector<string* > all_pos_str;
		//1.init the words-HashMap
		cout << "-- First go through all to get vocabs." << endl;
		all_word_str.push_back(&SENTENCE_START);
		all_word_str.push_back(&SENTENCE_END);
		all_word_str.push_back(&SENTENCE_START_2);
		all_word_str.push_back(&SENTENCE_END_2);
		all_word_str.push_back(&WORD_UNKNOWN);
		all_words.insert(pair<string*, int>(&SENTENCE_START,words_count++));
		all_words.insert(pair<string*, int>(&SENTENCE_END,words_count++));
		all_words.insert(pair<string*, int>(&SENTENCE_START_2,words_count++));
		all_words.insert(pair<string*, int>(&SENTENCE_END_2,words_count++));
		all_words.insert(pair<string*, int>(&WORD_UNKNOWN,words_count++));
		if(CONF_if_consider_pos==2){
			//add pos info...
			all_pos_str.push_back(&POS_START);
			all_pos_str.push_back(&POS_END);
			all_pos_str.push_back(&POS_START_2);
			all_pos_str.push_back(&POS_END_2);
			all_pos_str.push_back(&POS_UNKNOWN);
			all_poss.insert(pair<string*, int>(&POS_START,pos_count++));
			all_poss.insert(pair<string*, int>(&POS_END,pos_count++));
			all_poss.insert(pair<string*, int>(&POS_START_2,pos_count++));
			all_poss.insert(pair<string*, int>(&POS_END_2,pos_count++));
			all_poss.insert(pair<string*, int>(&POS_UNKNOWN,pos_count++));
		}
		//1.5first get all the words
		reader->startReading(CONF_train_file.c_str());
		DependencyInstance* x = reader->getNext();
		while(x != NULL){
			int length = x->forms->size();
			//words
			for(int i=0;i<length;i++){
				//here we have to take care of the POS(here is the mode 1 or 2 --- add behind words)
				string *to_find = CONF_if_consider_pos ? (x->combined_feats->at(i)) : x->forms->at(i);
				HashMap::iterator iter = all_words.find(to_find);
				if(iter == all_words.end()){
					string* new_one = new string(*to_find);
					all_words.insert(pair<string*, int>(new_one,words_count++));
					all_word_str.push_back(new_one);
				}
				//here is only in mode 2
				if(CONF_if_consider_pos==2){
					string *to_find = x->postags->at(i);
					HashMap::iterator iter = all_poss.find(to_find);
					if(iter == all_poss.end()){
						string* new_one = new string(*to_find);
						all_poss.insert(pair<string*, int>(new_one,pos_count++));
						all_pos_str.push_back(new_one);
					}
				}
			}
			delete x;
			x = reader->getNext();
		}
		cout << "Done, words: "<< words_count << ",pos: " << pos_count << ",all" << words_count+pos_count << endl;
		//1.8 mix words and pos --- all to words...
		if(CONF_if_consider_pos==2){
			for(int i=0;i<pos_count;i++){
				string* tmp = all_pos_str.at(i);
				all_words.insert(pair<string*, int>(tmp,words_count++));
				all_word_str.push_back(tmp);
			}
		}
		//1.9.write out vocab file
		cout << "---Now Writing vocab to '" << CONF_vocab_file << "'" << endl;
		ofstream fout(CONF_vocab_file.c_str());
		for(int i=0;i<words_count;i++)
			fout << *(all_word_str.at(i)) << '\n';
		fout.close();
		reader->finishReading();
	}

	//2.read-in all the feats
	cout << "--2.load vocab and get features" << endl;
	HashMap * all_words = load_wordlist(CONF_vocab_file.c_str());
	reader->startReading(CONF_train_file.c_str());
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		sentence_count++;
		if(sentence_count % 3000 == 0)
			cout << "--Reading sentence " << sentence_count << endl;
		int length = x->forms->size();
		tokens_count += length;
		int* word_index = get_word_index(length,x,all_words,0);
		//--deal with all pairs
		for(int i=0;i<length;i++){
			for(int j=i+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = get_feature(length,j,i,word_index);
					else
						tmp_str = get_feature(length,i,j,word_index);
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
			string* tmp = get_feature(length,head,i,word_index);
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
			<< "--Tokens: " << tokens_count << '\n'
			<< "--Feats: " << feat_count << '\n';

	//4.assign the scores or classes && write to data-file
	cout << "---3.assign scores and write... " << endl;
	cout << "---Classify mode is "<< CONF_if_y_calss << endl;
	REAL* final_scores = new REAL[feat_count];
	string unknown_token = WORD_UNKNOWN;
	ofstream fout;
	fout.open(CONF_data_file.c_str());
	fout << feat_count << ' ' << IND_CONF_x_dim_final << ' ' << 1 << endl;
	for(int i=0;i<feat_count;i++){
		if(i % 4000000 == 0)
			cout << "--Ok with features " << i << endl;
		//write the x
		fout << *(all_feat_str.at(i));
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
			tmp = (int)((tmp-SCO_MIN_BOUNT)/(SCO_MAX_BOUND-SCO_MIN_BOUNT) * CONF_y_class_size);
		}
		final_scores[i] = tmp;
		fout << tmp << '\n';
	}
	fout.close();

	//5.evaluate for testing
	cout << "---4.testing it... " << endl;
	debug_pretraining_evaluate(final_scores,&all_features,all_words);
	cout << "--Finish pre-training." << endl;
	return;
}
