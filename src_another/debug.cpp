/*
 * debug.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: zzs
 */

//the debugging functions
#include "param.h"

//files

string DEUBG_traindata_output = "debug_train_output.data";

//-----3.pretraining
void debug_pretraining_write(REAL* xbin,REAL* ybin,int num)
{
	//write the index file
	cout << "--2.5.1-debug write training data out, please check it yourself." << endl;
	ofstream x;
	x.open(ASCII_traindata_index.c_str(),ios::out|ios::trunc);
	if(x.fail()){
		Error("File opened fail, can't write debug Data");
	}
	x << num << '\t' << CONF_X_dim << '\t' << CONF_Y_dim << '\n';
	for(int i=0;i<num;i++){
		for(int j=0;j<CONF_X_dim;j++)
			x << (xbin)[i*CONF_X_dim+j] << '\t';
		x << "\t";
		//for(int j=0;j<CONF_Y_dim;j++)
		//just CONF_Y_dim == 1
			x << (ybin)[i] << ' ';
		x << '\n';
	}
	x.close();
	cout << "--2.5.1-Done with debug write training data out." << endl;
}

extern string* get_feature(DependencyInstance* x,int i,int j);
void debug_pretraining_evaluate(int num,REAL* scores,HashMap* maps)
{
	//evaluate the exact train file --- same order...
	cout << "--2.5.2-debug training data and evaluate." << endl;
	CONLLReader* reader = new CONLLReader();
	CONLLWriter* writer = new CONLLWriter();
	reader->startReading(CONF_train_file.c_str());
	writer->startWriting(DEUBG_traindata_output.c_str());
	DependencyInstance* x = reader->getNext();

	int miss_count = 0,sentence_count=0;
	while(x != NULL){
		if(++sentence_count % 1000 == 0)
			cout << "--Finish sentence " << sentence_count << "miss:" << miss_count << endl;
		int length = x->forms->size();
		double *tmp_scores = new double[length*length*2];
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					int index = get_index2(length,ii,j,lr);
					string *tmp_str = 0;
					if(lr==E_LEFT)
						tmp_str = get_feature(x,j,ii);
					else
						tmp_str = get_feature(x,ii,j);
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
	DependencyEvaluator::evaluate(CONF_train_file,DEUBG_traindata_output,DEUBG_traindata_output,false);
	cout << "--2.5.2-Done with debug training data and evaluate and miss is " << miss_count << endl;
}

