/*
 * new_infer.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: zzs
 */

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include<fstream>
#include<ctime>
#include<cstdlib>
#include"Util.h"
#include"StringPool.h"
#include"variable.h"
#include"ParseForestItem.h"
#include"DependencyEvaluator.h"
#include"DependencyParser.h"
#include"new_ones.h"
using namespace std;

static vector<vector<vector<double> > > probs;
static vector<vector<vector<vector<double> > > > nt_probs;
static string No_type = "<nope>";

/* score an instance(sentence) */
void newone_score(DependencyInstance *instance)
{
	int Length = instance->forms->size();
	if(probs.size() < Length){
		probs = vector<vector<vector<double> > >(Length);
		for(int i = 0; i < Length; i++){
			probs[i] = vector<vector<double> >(Length);
			for(int j = 0; j < Length; j++){
				probs[i][j] = vector<double>(2);
			}
		}
	}
	for (int w1 = 0; w1 < Length; w1++) {
			for (int w2 = w1 + 1; w2 < Length; w2++) {
				for (int ph = 0; ph < 2; ph++) {
					probs[w1][w2][ph] = ((double)rand()/RAND_MAX);
				}
			}
		}
}

/* process one sentence
 * 	--- mainly transformed from OnlineTrainer::outputParses
 */
void newone_outputparse_one(DependencyParser& dp,DependencyInstance *instance)
{
	//get probs
	newone_score(instance);

	int K = dp.options->testK;
	int length = instance->length();

	vector<pair<FeatureVector*, string*> >* d = NULL;

	d = dp.decoder->decodeProjective(instance, probs, nt_probs, K);

	vector<string*>* res = Util::split((*d)[0].second, ' ');
	vector<string*>* forms = instance->forms;
	vector<string*>* pos = instance->postags;
	vector<string*>* lemma = instance->lemmas;
	vector<string*>* cpos = instance->cpostags;
	vector<string*> formsNoRoot(length - 1);
	vector<string*> lemmaNoRoot(length - 1);
	vector<string*> cposNoRoot(length - 1);
	vector<string*> posNoRoot(length - 1);
	vector<string*> labels(length - 1);
	vector<int> heads(length - 1);

	for(int j = 0; j < length - 1; j++){
		formsNoRoot[j] = (*forms)[j + 1];
		lemmaNoRoot[j] = (*lemma)[j + 1];
		cposNoRoot[j] = (*cpos)[j + 1];
		posNoRoot[j] = (*pos)[j + 1];
		int position = (int)((*res)[j]->find('|'));
		string head = (*res)[j]->substr(0, position);
		position = (int)((*res)[j]->find(':'));
		string lab = (*res)[j]->substr(position + 1, (*res)[j]->length() - position -1);
		heads[j] = Util::stringToInt(&head);
		if(dp.pipe->labeled)
			labels[j] = dp.pipe->types[Util::stringToInt(&lab)];
		else
			labels[j] = &No_type;
		//cout<<(*res)[j]->c_str()<<'\t'<<formsNoRoot[j]->c_str()<<'\t'<<posNoRoot[j]->c_str()<<'\t'<<head.c_str()<<'\t'<<labels[j]->c_str()<<endl;
	}
	dp.pipe->outputInstance(formsNoRoot, lemmaNoRoot, cposNoRoot, posNoRoot, labels, heads);
	int l = (int)(res->size());
	for(int j = 0; j < l; j++){
		pool->push_back((*res)[j]);
	}
	delete(res);
	if(d != NULL){
		int len = (int)(d->size());
		for(int i = 0; i < len; i++){
			pair<FeatureVector*, string*> p = (*d)[i];
			if(p.first != NULL){
				delete(p.first);
			}
			if(p.second != NULL){
				pool->push_back(p.second);
			}
		}
		delete(d);
	}
}

/* the output parse function
 * 	--- mainly transformed from DependencyParser::outputParses()
 */
void newone_outputparse(DependencyParser* dp_p){
	/* 0.init sth */
	srand((int)time(0));
	/* 1.init files */
	DependencyParser& dp = *dp_p;
	string* tFile = dp.options->testfile;
	string* file = dp.options->outfile;
	long start = clock() / CLOCKS_PER_SEC;
	dp.pipe->initInputFile(tFile);
	dp.pipe->initOutputFile(file);

	/* 2.start */
	cout<<"Processing Sentence: ";
	DependencyInstance* instance = dp.pipe->nextInstance();
	int cnt = 0;
	while(instance != NULL){
		printf("%d ", cnt++);
		//int length = instance->length();
		newone_outputparse_one(dp,instance);

		delete(instance);
		instance = dp.pipe->nextInstance();
	}
	dp.pipe->close();
	long end = clock() / CLOCKS_PER_SEC;
	cout<<endl;
	cout<<"Took: "<<(end - start)<<endl;
	return;
}
