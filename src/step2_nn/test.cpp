/*
 * test.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: zzs
 */


#include "load_pre.h"
#include "../HashMap.h"
#include "../CONLLReader.h"
#include "../CONLLWriter.h"
#include "../Eisner.h"
#include "../step3_eval/DependencyEvaluator.h"
#include "../pre_training.h"
#include "../which_main.h"

int main2test()
{
	using namespace training_space;
	load_for_test();

	//testing the training file
	cout << "Now testing the train file..." << endl;
	CONLLReader* reader = new CONLLReader();
	CONLLWriter* writer = new CONLLWriter();
	reader->startReading(CONF_train_file.c_str());
	writer->startWriting(CONF_output_file.c_str());

	int count_now=0;
	int miss_count=0;
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		if(count_now%1000 == 0)
			cout << "Having processed " << count_now << endl;
		count_now++;
		int length = x->forms->size();
		double *tmp_scores = new double[length*length*2];
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = pre_training_space::get_feature(x,j,ii);
					else
						tmp_str = pre_training_space::get_feature(x,ii,j);
					HashMap::iterator iter = all_features->find(tmp_str);
					double score = (*all_score)[iter->second];
					int index = get_index2(length,ii,j,lr);
					tmp_scores[index] = score;
					delete tmp_str;
				}
			}
		}
		vector<int> *ret = decodeProjective(length,tmp_scores);
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(x->heads))[i2])
				miss_count ++;
		}
		delete x->heads;
		x->heads = ret;
		writer->write(x);
		delete x;
		delete []tmp_scores;
		x = reader->getNext();
	}
	reader->finishReading();
	writer->finishWriting();
	delete reader;
	delete writer;

	//evaluate
	cout << "Self-count miss is " << miss_count << endl;
	string t;
	DependencyEvaluator::evaluate(CONF_gold_file,CONF_output_file,t,false);

#ifdef MEMORY_DEBUG
	for(int i=0;i<=20;i++){
		char *a = new char[1024*1024*100];	//100M
		cout << i;
	}
#endif
	return 0;
}

