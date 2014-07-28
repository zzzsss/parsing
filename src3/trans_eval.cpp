//std ---> stdout
//-0.9 ~ 0.9 --> classifications

#include "eval.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "tools/CONLLReader.h"
#include "tools/CONLLWriter.h"
#include "tools/Eisner.h"
#include "tools/DependencyEvaluator.h"
#include "tools/HashMap.h"
using namespace std;

#define LOW -0.91
#define HIGH 0.91
string CONF_train_file = "train.right";
HashMap *load_wordlist_dup(const char *fname)
{
	//lists of the vocabularies
	cerr << "Opening vocab file '" << fname << "'" << endl;
	ifstream ifs;
	ifs.open(fname,ios::in);
	//CHECK_FILE(ifs,fname);
	int num=0;
	HashMap *res = new HashMap(1000000);
	while (!ifs.eof()){
		string buf;
		ifs >> buf;
		if (buf=="") continue; // HACK
		string* tmp = new string(buf);
		res->insert(pair<string*, int>(tmp, num++));
	}
	if(res->size() != num){
		//sth wrong
		cerr << "Something wrong with vocab file." << endl;
	}
	cerr << "Done with loading vocab, all is " << num << endl;
	return res;
}

void debug_pretraining_evaluate(int num,int* scores,HashMap* maps)
{
	HashMap* wl = load_wordlist_dup("vocab.list");
	//evaluate the exact train file --- same order...
	CONLLReader* reader = new CONLLReader();
	reader->startReading(CONF_train_file.c_str());
	DependencyInstance* x = reader->getNext();
	int miss_count = 0,sentence_count=0;
	while(x != NULL){
		if(++sentence_count % 1000 == 0)
			cerr << "--Finish sentence " << sentence_count << "miss:" << miss_count << endl;
		int length = x->forms->size();
		double *tmp_scores = new double[length*length*2];

		string unknown_token = "<unk>";
		int* word_index = new int[length+2];	//including <s> and </s>
		for(int i=0;i<length;i++){
			HashMap::iterator iter = wl->find(x->forms->at(i));
			if(iter == wl->end()){
				word_index[i+1] = wl->find(&unknown_token)->second;
			}
			else
				word_index[i+1] = iter->second;
		}
		string sen_s = SENTENCE_START;
		string sen_e = SENTENCE_END;
		word_index[0] = wl->find(&sen_s)->second;
		word_index[length+1] = wl->find(&sen_e)->second;

		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					int index = get_index2(length,ii,j,lr);
					stringstream tmps;
					if(lr==E_LEFT)
						tmps << word_index[j] << ' ' << word_index[j+1] << ' '
							 << word_index[j+2] << ' ' << word_index[ii] << ' '
							 << word_index[ii+1] << ' ' << word_index[ii+2];
					else
						tmps << word_index[ii] << ' ' << word_index[ii+1] << ' '
							 << word_index[ii+2] << ' ' << word_index[j] << ' '
							 << word_index[j+1] << ' ' << word_index[j+2];
					//add it or already there
					string temp_s = tmps.str();
					HashMap::iterator iter = maps->find(&temp_s);
					int which = iter->second;
					tmp_scores[index] = scores[which];
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
	cerr << "Final miss " << miss_count << endl;
}

int main_undef(int argc,char ** argv)
{
	int total,in,out;
	int index;
	HashMap* maps = new HashMap(90000000);
	//head line
	cerr << "Start transform..." << endl;
	cin >> total >> in >> out;
	int* new_score = new int[total];
	cout << total << " " << in << " " << 1 << endl;
	for(int i=0;i<total;i++){
		stringstream tmps;
		for(int j=0;j<CONF_X_dim;j++){
			cin >> index;
			cout << index << " ";
			tmps << index;
			if(j<CONF_X_dim-1)
				tmps << ' ';
		}
		string* adding = new string(tmps.str());
		maps->insert(pair<string*, int>(adding,i));
		//output just one classification
		double tmp=0;
		cin >> tmp;
		int it = (int)((tmp-LOW)/(HIGH-LOW) * CONF_Y_dim);
		new_score[i] = it;
		cout << it << endl;
	}

	//if eval
	if(argc > 1 && argv[1][0]=='y'){
		cerr << "Start eval..." << endl;
		debug_pretraining_evaluate(total,new_score,maps);
	}
	return 0;
}
